//
// SnapInterface.cc
//
// Created on October 25th 2018
//
// Copyright 2018 CFM (www.cfm.fr)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <algorithm>

#include "SnapInterface.h"


namespace AnyCollect {
	std::hash<std::string> SnapInterface::hasher{};

	SnapInterface::SnapInterface() :
		controller_(*this),
		sendAllMetrics_(false)
	{ }


	void SnapInterface::insertAppPrefixToNamespace(std::vector<std::string>& ns) {
		std::transform(SnapInterface::appPrefix.begin(), SnapInterface::appPrefix.end(), std::inserter(ns, ns.begin()), [&](auto s) {
			return std::string(s);
		});
	}

	void SnapInterface::setConfig(const Plugin::Config& cfg) {
		std::chrono::seconds sampling = Controller::defaultSamplingInterval;
		std::string configPath;
		bool sendAll = SnapInterface::defaultSendAllMetrics;

		if (cfg.has_int_key(std::string(SnapInterface::configKeySamplingInterval)))
			sampling = std::chrono::seconds(cfg.get_int(std::string(SnapInterface::configKeySamplingInterval)));
		if (cfg.has_string_key(std::string(SnapInterface::configKeyConfigFile)))
			configPath = cfg.get_string(std::string(SnapInterface::configKeyConfigFile));
		if (cfg.has_bool_key(std::string(SnapInterface::configKeySendAllMetrics)))
			sendAll = cfg.get_bool(std::string(SnapInterface::configKeySendAllMetrics));

		this->controller_.setSamplingInterval(sampling);
		this->controller_.loadConfigFromFile(configPath);
		this->sendAllMetrics_ = sendAll;
	}

	void SnapInterface::formatName(std::vector<std::string>& name) {
		for (auto& part : name) {
			bool wasCapitalized = false;
			for (size_t i = 0; i < part.size(); i++) {
				if (!std::isalnum(part[i]) && part[i] != '_') {
					if (i > 0 && part[i - 1] != '_') {
						part[i] = '_';
					} else {
						part.erase(i, 1);
						i--;
					}
				}
				else if (std::isupper(part[i])) {
					if (i > 0 && part[i - 1] != '_' && std::islower(part[i - 1]) && !wasCapitalized) {
						part.insert(i, "_");
						i++;
					}
					part[i] = std::tolower(part[i]);
					wasCapitalized = true;
				} else
					wasCapitalized = false;
			}
			while (part.back() == '_')
				part.erase(part.size() - 1, 1);
			while (part.front() == '_')
				part.erase(0, 1);
		}
	}

	size_t SnapInterface::computeNameKey(const Metric& m) {
		std::string nameHash;
		std::vector<std::string> name = m.name();
		this->formatName(name);
		for (const auto& n : name)
			nameHash.append(n);
		return SnapInterface::hasher(nameHash);
	}

	size_t SnapInterface::computeNameKey(const Plugin::Metric& m) {
		std::string nameHash;
		for (size_t i = 2; i < m.ns().size(); i++)
			nameHash.append(m.ns()[i].get_value());
		return SnapInterface::hasher(nameHash);
	}

	Plugin::Metric SnapInterface::convertToSnapMetric(const Metric& metric) {
		std::vector<std::string> name = metric.name();
		this->formatName(name);
		this->insertAppPrefixToNamespace(name);
		Plugin::Namespace ns{name};
		Plugin::Metric snapMetric{ns, metric.unit(), ""};
		for (const auto& tag : metric.tags())
			snapMetric.add_tag(tag);
		snapMetric.set_data(metric.value());
		snapMetric.set_timestamp(metric.timestamp());
		return snapMetric;
	}


	const Plugin::ConfigPolicy SnapInterface::get_config_policy() {
		Plugin::ConfigPolicy policy;
		std::vector<std::string> ns;
		std::vector<std::string> baseNamespace;
		this->insertAppPrefixToNamespace(baseNamespace);

		ns = baseNamespace;
		ns.emplace_back(SnapInterface::configKeyConfigFile);
		policy.add_rule(ns, Plugin::StringRule{std::string(SnapInterface::configKeyConfigFile), {true}});

		for (size_t i = 0; i < SnapInterface::configKeysInt.size(); i++) {
			ns = baseNamespace;
			ns.emplace_back(SnapInterface::configKeysInt[i]);
			policy.add_rule(ns, Plugin::IntRule{std::string(SnapInterface::configKeysInt[i]), {SnapInterface::configValuesInt[i], false}});
		}
		for (size_t i = 0; i < SnapInterface::configKeysBool.size(); i++) {
			ns = baseNamespace;
			ns.emplace_back(SnapInterface::configKeysBool[i]);
			policy.add_rule(ns, Plugin::BoolRule{std::string(SnapInterface::configKeysBool[i]), {SnapInterface::configValuesBool[i], false}});
		}
		return policy;
	}

	std::vector<Plugin::Metric> SnapInterface::get_metric_types(Plugin::Config cfg) {
		std::vector<Plugin::Metric> metrics;
		this->setConfig(cfg);

		auto availableMetrics = this->controller_.availableMetrics();
		for (auto& m : availableMetrics)
			metrics.push_back(this->convertToSnapMetric(*m));

		std::vector<std::string> name = {std::string(SnapInterface::configKeySendAllMetrics)};
		this->insertAppPrefixToNamespace(name);
		Plugin::Namespace ns{name};
		metrics.emplace_back(ns, "", "");

		return metrics;
	}

	void SnapInterface::get_metrics_in(std::vector<Plugin::Metric> &metsIn) {
		if (metsIn.size() == 0)
			return;

		this->setConfig(metsIn.front().get_config());

		this->requestedMetrics_.clear();
		for (const auto& m : metsIn) {
			if (m.ns().size() == (SnapInterface::appPrefix.size() + 1) && m.ns()[2].get_value() == SnapInterface::configKeySendAllMetrics)
				this->sendAllMetrics_ = true;
			else
				this->requestedMetrics_.insert(this->computeNameKey(m));
		}

		auto availableMetrics = this->controller_.availableMetrics();
		this->metrics_.clear();
		this->unwantedMetrics_.clear();
		for (const auto& m : availableMetrics) {
			if (this->sendAllMetrics_ || this->requestedMetrics_.count(this->computeNameKey(*m)) > 0)
				this->metrics_.insert_or_assign(m->key(), this->convertToSnapMetric(*m));
			else
				this->unwantedMetrics_.insert(m->key());
		}
	}


	void SnapInterface::stream_metrics() {
		if (!this->sendAllMetrics_ && this->requestedMetrics_.empty())
			return;

		this->controller_.collectMetrics();
	}


	void SnapInterface::contollerCollectedMetrics(const AnyCollect::Controller& , const std::vector<const AnyCollect::Metric*>& metrics) {
		this->metricsToSend_.clear();
		for (const auto& metric : metrics) {
			auto itr = this->metrics_.find(metric->key());
			if (itr != this->metrics_.end()) {
				itr->second.set_data(metric->value());
				itr->second.set_timestamp(metric->timestamp());
				this->metricsToSend_.push_back(&itr->second);
			} else if (this->sendAllMetrics_) {
				auto itr3 = this->metrics_.insert_or_assign(metric->key(), this->convertToSnapMetric(*metric));
				this->metricsToSend_.push_back(&itr3.first->second);
			} else {
				auto itr2 = this->unwantedMetrics_.find(metric->key());
				if (itr2 == this->unwantedMetrics_.end()) {
					if (this->requestedMetrics_.count(this->computeNameKey(*metric)) > 0) {
						auto itr3 = this->metrics_.insert_or_assign(metric->key(), this->convertToSnapMetric(*metric));
						this->metricsToSend_.push_back(&itr3.first->second);
					} else {
						this->unwantedMetrics_.insert(metric->key());
					}
				}
			}
		}
		this->send_metrics(this->metricsToSend_);
	}

	bool SnapInterface::contollerShouldStopCollectingMetrics(const AnyCollect::Controller& ) {
		return this->context_cancelled();
	}
}


int main(int argc, char* argv[]) {
    Plugin::Meta meta(Plugin::Type::StreamCollector, std::string(AnyCollect::SnapInterface::appName), AnyCollect::SnapInterface::appVersion, Plugin::RpcType::GRPCStream);
	meta.concurrency_count = 1;
	meta.exclusive = false;
	meta.strategy = Plugin::Strategy::Sticky;
	meta.cache_ttl = 1ms;
    AnyCollect::SnapInterface plugin;
	Plugin::start_stream_collector(argc, argv, &plugin, meta);
	return 0;
}
