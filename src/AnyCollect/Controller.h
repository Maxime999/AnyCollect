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

#include "Source.h"
#include "Expression.h"
#include "Matcher.h"
#include "Metric.h"

using namespace std::literals;


namespace AnyCollect {
	class ControllerDelegate;

	/**
	 * @brief
	 *
	 */
	class Controller {
		public:
#if PROFILING
			static constexpr std::chrono::seconds defaultSamplingInterval = 0s;			//!< Default parameter option
#else
			static constexpr std::chrono::seconds defaultSamplingInterval = 10s;		//!< Default parameter option
#endif

		protected:
			ControllerDelegate& delegate_;												//!< Delegate to alert when something happens

			bool isCollecting_;															//!< Whether the receiver is collecting metrics
			std::chrono::seconds samplingInterval_;										//!< Metrics sampling interval
			double unitsPerSecondFactor_;												//!< Factor to convert metric differences to units per second
			size_t roundKey_;															//!< Metric collection iteration unique identifier

			std::vector<std::shared_ptr<Source>> sources_;								//!< Array of sources
			std::vector<std::shared_ptr<Expression>> expressions_;						//!< Array of expressions
			std::vector<std::shared_ptr<Matcher>> matchers_;							//!< Array of matchers
			std::map<size_t, Metric> metrics_;											//!< Map associating keys to their metric
			std::vector<const Metric*> updatedMetrics_;									//!< Array of pointers to the iteration's metrics

			/**
			 * @brief Updates sources (fetches file contents and executes commands)
			 */
			void updateSources() noexcept;

			/**
			 * @brief For each line of each source, executes the source's expressions to find matches
			 */
			void computeMatches() noexcept;

			/**
			 * @brief Updates metrics from a match
			 *
			 * @param source the Matcher's source
			 * @param match the results of the expression's matching
			 * @param matcher the Matcher object to create a metric from
			 */
			void parseData(const Source& source, const std::cmatch& match, const Matcher& matcher) noexcept;

		public:
			/**
			 * @brief Construct a new Controller object
			 *
			 * @param delegate the controller's delegate
			 */
			Controller(ControllerDelegate& delegate) noexcept;


			/**
			 * @brief Returns the controller's delegate
			 */
			ControllerDelegate& delegate() const noexcept;


			/**
			 * @brief Returns whether the controller is collecting metrics
			 */
			bool isCollecting() const noexcept;

			/**
			 * @brief Returns the metrics sampling interval
			 */
			std::chrono::seconds samplingInterval() const noexcept;


			/**
			 * @brief Configures sources, expressions and matchers according to config file
			 *
			 * @param configPath the config file path
			 */
			void loadConfigFromFile(const std::string& configPath);

			/**
			 * @brief Sets the metrics sampling interval
			 */
			void setSamplingInterval(std::chrono::seconds interval) noexcept;


			/**
			 * @brief Returns the array of all currently matching metrics on the system, without their values
			 *
			 * @return metrics currently available on the system
			 */
			std::vector<const Metric*> availableMetrics() noexcept;

			/**
			 * @brief Launch metric collection loop
			 */
			void collectMetrics() noexcept;
	};


	/**
	 * @brief Abstract delegate of Controller
	 *
	 * Each function is called when the Controller has done something of importance
	 */
	class ControllerDelegate {
		public:
			/**
			 * @brief Function called every time the Controller has collected metrics
			 *
			 * @param controller the calling Controller
			 * @param metrics latest collected metrics
			 */
			virtual void contollerCollectedMetrics(const Controller& controller, const std::vector<const Metric*>& metrics) = 0;

			/**
			 * @brief Function called after each iteration to ask the delegate whether the controller should stop collecting metrics
			 *
			 * @param controller the calling Controller
			 * @return *true* if the controller should stop
			 * @return *false* otherwise
			 */
			virtual bool contollerShouldStopCollectingMetrics(const Controller& controller) = 0;
	};
}
