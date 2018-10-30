//
// SnapInterface.h
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

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include <AnyCollect/Controller.h>
#include <snap/plugin.h>

using namespace std::literals;


namespace AnyCollect {
	class SnapInterface final : public Plugin::StreamCollectorInterface, public AnyCollect::ControllerDelegate {
		public:
			static constexpr int appVersion = 1;
			static constexpr std::string_view appName = "anycollect"sv;

		protected:
			static constexpr std::array appPrefix = {"cfm"sv, "anycollect"sv};
			static constexpr std::string_view configKeyConfigFile = "ConfigFile"sv;
			static constexpr std::string_view configKeySamplingInterval = "SamplingInterval"sv;
			static constexpr std::string_view configKeyMaxCollectDuration = "MaxCollectDuration"sv;
			static constexpr std::string_view configKeyMaxMetricsBuffer = "MaxMetricsBuffer"sv;
			static constexpr std::array configKeysString = {configKeyConfigFile};
			static constexpr std::array configKeysInt = {configKeySamplingInterval, configKeyMaxCollectDuration, configKeyMaxMetricsBuffer};
			static constexpr std::chrono::seconds defaultMaxCollectDuration = 0s;
			static constexpr size_t defaultMaxMetricsBuffer = 0;
			static constexpr std::array<int, configKeysInt.size()> configValuesInt = {AnyCollect::Controller::defaultSamplingInterval.count(), SnapInterface::defaultMaxCollectDuration.count(), SnapInterface::defaultMaxMetricsBuffer};
			static std::hash<std::string> hasher;

			AnyCollect::Controller controller_;
			std::map<size_t, Plugin::Metric> metrics_;
			std::set<size_t> requestedMetrics_;
			std::set<size_t> unwantedMetrics_;
			std::vector<Plugin::Metric*> metricsToSend_;

			void insertAppPrefixToNamespace(std::vector<std::string>& ns);
			void setConfig(const Plugin::Config& cfg);
			size_t computeNameKey(const Metric& m);
			size_t computeNameKey(const Plugin::Metric& m);
			Plugin::Metric convertToSnapMetric(const Metric& metric);

		public:
			SnapInterface();

			const Plugin::ConfigPolicy get_config_policy() override final;
     	 	std::vector<Plugin::Metric> get_metric_types(Plugin::Config cfg) override final;
			void get_metrics_in(std::vector<Plugin::Metric> &metsIn) override final;

			void stream_metrics() override final;

			void contollerCollectedMetrics(const Controller& controller, const std::vector<Metric*>& metrics) override final;
			bool contollerShouldStopCollectingMetrics(const Controller& controller) override final;

	};
}


int main(int argc, char* argv[]);
