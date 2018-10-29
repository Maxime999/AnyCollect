//
// Controller.h
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

#include <chrono>
#include <memory>
#include <regex>
#include <vector>

#include "File.h"
#include "Expression.h"
#include "Matcher.h"
#include "Metric.h"

using namespace std::literals;


namespace AnyCollect {
	class ControllerDelegate;

	class Controller {
		public:
#if PROFILING
			static constexpr std::chrono::seconds defaultSamplingInterval = 0s;
#else
			static constexpr std::chrono::seconds defaultSamplingInterval = 10s;
#endif

		protected:
			ControllerDelegate& delegate_;

			bool isCollecting_;
			std::chrono::seconds samplingInterval_;
			double unitsPerSecondFactor_;
			size_t roundKey_;

			std::vector<std::shared_ptr<File>> files_;
			std::vector<std::shared_ptr<Expression>> expressions_;
			std::vector<std::shared_ptr<Matcher>> matchers_;
			std::map<size_t, Metric> metrics_;
			std::vector<Metric*> updatedMetrics_;

			void updateData();

		public:
			Controller(ControllerDelegate& delegate) noexcept;

			ControllerDelegate& delegate() const noexcept;

			bool isCollecting() const noexcept;
			std::chrono::seconds samplingInterval() const noexcept;

			void loadConfigFromFile(const std::string& configPath);
			void setSamplingInterval(std::chrono::seconds interval) noexcept;

			void collectMetrics();
	};

	class ControllerDelegate {
		public:
			virtual void contollerCollectedMetrics(const Controller& controller, const std::vector<Metric*>& metrics) = 0;
			virtual bool contollerShouldStopCollectingMetrics(const Controller& controller) = 0;
	};
}
