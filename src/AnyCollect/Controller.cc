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

		this->updateData();
		while (true) {
			std::this_thread::sleep_for(this->samplingInterval_ - (std::chrono::steady_clock::now() - startTime));
			startTime = std::chrono::steady_clock::now();

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


	void Controller::updateData() {
		this->updatedMetrics_.clear();
		for (auto& file : this->files_)
			file->read();

		for (const auto& file : this->files_) {
			auto begin = file->begin();
			auto end = file->getLine(begin);
			while(end != file->end()) {
				for (const auto& expression : file->expressions()) {
					std::cmatch match;
					if (begin != end && *(end - 1) == '\n')
						match = expression->apply(begin, end - 1);
					else
						match = expression->apply(begin, end);
					if (match.empty())
						continue;
					for (const auto& matcher : expression->matchers()) {
						auto value = matcher->getValue(match, file->pathParts());
						if (!value.has_value())
							continue;
						auto metric = matcher->getMetric(match, file->pathParts());
						if (!metric.has_value())
							continue;

						auto itr = this->metrics_.find(metric.value().key());
						if (itr == this->metrics_.end()) {
							itr = this->metrics_.insert(this->metrics_.begin(), {metric.value().key(), std::move(metric.value())});
							if (!matcher->computeRate())
								this->updatedMetrics_.push_back(&itr->second);
						} else if (itr->second.roundKey() == this->roundKey_ - 1) {
							this->updatedMetrics_.push_back(&itr->second);
						}
						itr->second.setNewValue(value.value(), matcher->computeRate(), matcher->convertToUnitsPerSecond() ? this->unitsPerSecondFactor_ : 1.0);
						itr->second.setTimestamp(file->timestamp());
						itr->second.setRoundKey(this->roundKey_);
					}
				}
				begin = end;
				end = file->getLine(begin);
			}
		}
		this->roundKey_++;
	}
}
