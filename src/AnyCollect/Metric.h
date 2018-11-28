//
// Metric.h
//
// Created on October 26th 2018
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
#include <map>
#include <string>
#include <vector>


namespace AnyCollect {
	/**
	 * @brief Class used to represent a metric object (with a name, unit, tags, a value and a timestamp)
	 */
	class Metric {
		protected:
			static std::hash<std::string> hasher;					//!< Object used to compute keys (string hashes)
			size_t key_;											//!< Key of the metric (hash of its name, tag keys and tags values)
			size_t roundKey_;										//!< Key of the collection iteration which created this metric
			std::vector<std::string> name_;							//!< Array of strings representing the name of the metric
			double previousValue_;									//!< Previous value of the metric
			double value_;											//!< Current value of the metric
			std::chrono::system_clock::time_point timestamp_;		//!< Timestamp of the metric
			std::string unit_;										//!< Unit of the metric
			std::map<std::string, std::string> tags_;				//!< Tags of the metric

		public:
			/**
			 * @brief Deleted default constructor
			 */
			Metric() = delete;

			/**
			 * @brief Construct a new Metric object
			 *
			 * @param name name of the metric
			 * @param tags tags of the metric
			 * @param unit unit of the metric
			 */
			Metric(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags, const std::string& unit = "") noexcept;

			/**
			 * @brief Copy constructor
			 *
			 * @param other metric to copy
			 */
			Metric(const Metric& other) noexcept;

			/**
			 * @brief Move constructor
			 *
			 * @param other metric to move
			 */
			Metric(Metric&& other) noexcept;

			/**
			 * @brief Assignment operator
			 *
			 * @param other metric to copy
			 */
			Metric& operator=(Metric other) noexcept;


			/**
			 * @brief Compute the key of potential metric
			 *
			 * @param name name of the metric
			 * @param tags tags of the metric
			 * @return the potential metric's key
			 */
			static size_t generateKey(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags);

			/**
			 * @brief Compute the key of a metric
			 *
			 * @param metric the metric to compute the key of
			 * @return the potential metric's key
			 */
			static size_t generateKey(const Metric& metric);


			/**
			 * @brief Returns the name of the metric
			 */
			const std::vector<std::string>& name() const noexcept;

			/**
			 * @brief Returns the key of the metric
			 */
			size_t key() const noexcept;

			/**
			 * @brief Returns the key of the collection iteration which created the metric
			 */
			size_t roundKey() const noexcept;

			/**
			 * @brief Returns the value of the metric
			 */
			double value() const noexcept;

			/**
			 * @brief Returns the timestamp of the metric
			 */
			std::chrono::system_clock::time_point timestamp() const noexcept;

			/**
			 * @brief Returns the unit of the metric
			 */
			const std::string& unit() const noexcept;

			/**
			 * @brief Returns the tags of the metric
			 */
			const std::map<std::string, std::string>& tags() const noexcept;


			/**
			 * @brief Sets a new value to the metric
			 *
			 * @param value the new value
			 * @param computeRate whether the rate should be computed (as `previousValue_ - value`)
			 * @param unitsPerSecondFactor factor to convert the value into units per seconds
			 */
			void setNewValue(double value, bool computeRate, double unitsPerSecondFactor = 1.0) noexcept;

			/**
			 * @brief Adds a new value to the current value
			 *
			 * When a matcher computes the same metric during a single collection iteration (same round key), the different computed values are summed
			 *
			 * @param value The value to add
			 * @param unitsPerSecondFactor factor to convert the value into units per seconds
			 */
			void updateValue(double value, double unitsPerSecondFactor = 1.0) noexcept;

			/**
			 * @brief Sets the timestamp of the metric
			 */
			void setTimestamp(std::chrono::system_clock::time_point timestamp) noexcept;

			/**
			 * @brief Sets the round key of the metric
			 */
			void setRoundKey(size_t roundKey) noexcept;
	};
}
