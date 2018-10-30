//
// Controller.cc
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

#include <thread>

#include "Config.h"
#include "Controller.h"


namespace AnyCollect {
	Controller::Controller(ControllerDelegate& delegate) noexcept :
		delegate_(delegate),
		isCollecting_(false)
	{
		this->setSamplingInterval(Controller::defaultSamplingInterval);
	}


	ControllerDelegate& Controller::delegate() const noexcept {
		return this->delegate_;
	}


	bool Controller::isCollecting() const noexcept {
		return this->isCollecting_;
	}

	std::chrono::seconds Controller::samplingInterval() const noexcept {
		return this->samplingInterval_;
	}


	void Controller::loadConfigFromFile(const std::string& configPath) {
		Config config = Config{configPath};

		this->files_.clear();
		this->expressions_.clear();
		this->matchers_.clear();
		size_t fileIndex = 0;

		for (const auto& file : config.files) {
			for (const auto& path : file.paths) {
				auto paths = File::filePathsMatchingGlobbingPattern(path);
				for (const auto& p : paths)
					this->files_.push_back(std::make_shared<File>(p));
			}
			for (const auto& expression : file.expressions) {
				this->expressions_.push_back(std::make_shared<Expression>(expression.regex));
				for (const auto& metric : expression.metrics) {
					this->matchers_.push_back(std::make_shared<Matcher>());
					this->matchers_.back()->setName(metric.name);
					this->matchers_.back()->setValue(metric.value);
					this->matchers_.back()->setUnit(metric.unit);
					this->matchers_.back()->setTags(metric.tags);
					this->matchers_.back()->setComputeRate(metric.computeRate);
					this->matchers_.back()->setConvertToUnitsPerSecond(metric.convertToUnitsPerSecond);
					this->expressions_.back()->matchers().push_back(this->matchers_.back());
				}
				for (auto itr = this->files_.begin() + fileIndex; itr != this->files_.end(); itr++)
					(*itr)->expressions().push_back(this->expressions_.back());
			}
			fileIndex = this->files_.size();
		}
	}


	void Controller::setSamplingInterval(std::chrono::seconds interval) noexcept {
		if (this->isCollecting_)
			return;

		this->samplingInterval_ = interval;
		if (interval != 0s)
			this->unitsPerSecondFactor_ = 1.0 / interval.count();
		else
			this->unitsPerSecondFactor_ = 1.0;
	}


	void Controller::collectMetrics() {
#if GPERFTOOLS_CPU_PROFILE
		ProfilerStart("/tmp/aa.prof");
#endif
		this->isCollecting_ = true;
		this->roundKey_ = 0;
		auto startTime = std::chrono::steady_clock::now();

		this->updatedMetrics_.clear();
		this->readFiles();
		this->updateData();

		while (true) {
			std::this_thread::sleep_for(this->samplingInterval_ - (std::chrono::steady_clock::now() - startTime));
			startTime = std::chrono::steady_clock::now();

			this->updatedMetrics_.clear();
			this->readFiles();
			this->updateData();

			this->delegate_.contollerCollectedMetrics(*this, this->updatedMetrics_);
			if (this->delegate_.contollerShouldStopCollectingMetrics(*this)) {
				this->isCollecting_ = false;
#if GPERFTOOLS_CPU_PROFILE
				ProfilerStop();
#endif
				return;
			}
#if GPERFTOOLS_CPU_PROFILE
			ProfilerFlush();
#endif
		}
	}


	void Controller::readFiles() {
		for (auto& file : this->files_)
			file->read();
	}

	void Controller::updateData() {
		for (const auto& file : this->files_) {
			auto begin = file->begin();
			while(begin != file->end()) {
				auto end = file->getLine(begin);
				if (begin != end) {
					for (const auto& expression : file->expressions()) {
						std::cmatch match = expression->apply(begin, end);
						if (!match.empty()) {
							for (const auto& matcher : expression->matchers())
								this->parseData(*file, match, *matcher);
						}
					}
				}
				if (end != file->end())
					begin = end + 1;
				else
					break;
			}
		}
		this->roundKey_++;
	}

	void Controller::parseData(const File& file, const std::cmatch& match, const Matcher& matcher) {
		auto value = matcher.getValue(match, file.pathParts());
		if (!value.has_value())
			return;
		auto newMetric = matcher.getMetric(match, file.pathParts());
		if (!newMetric.has_value())
			return;

		auto itr = this->metrics_.find(newMetric.value().key());
		bool isNew = (itr == this->metrics_.end());
		if (isNew)
			itr = this->metrics_.insert(this->metrics_.begin(), {newMetric.value().key(), std::move(newMetric.value())});

		Metric& metric = itr->second;
		isNew = (isNew || (metric.roundKey() != this->roundKey_ - 1));
		if (!isNew || !matcher.computeRate())
			this->updatedMetrics_.push_back(&metric);

		if (metric.roundKey() != this->roundKey_)
			metric.setNewValue(value.value(), matcher.computeRate(), matcher.convertToUnitsPerSecond() ? this->unitsPerSecondFactor_ : 1.0);
		else
			metric.updateValue(value.value(), matcher.convertToUnitsPerSecond() ? this->unitsPerSecondFactor_ : 1.0);
		metric.setTimestamp(file.timestamp());
		metric.setRoundKey(this->roundKey_);
	}

}
