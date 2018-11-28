//
// Matcher.h
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

#include <map>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "Config.h"
#include "Metric.h"

using namespace std::literals;


namespace AnyCollect {
	/**
	 * @brief Class used to represent a matcher, that is a way to convert expressions matches into a metric
	 */
	class Matcher {
		public:
			static constexpr char matchEscapeChar = '\\';									//!< Escape character
			static constexpr char matchSubstitutionPrefix = '$';							//!< Variables prefix character
			static constexpr std::string_view matchSubstitutionPathPrefix = "path_"sv;		//!< Path part prefix variable string

		protected:
			std::vector<std::string> name_;													//!< Pattern for the name of the metric
			std::string value_;																//!< Pattern for the value of the metric
			std::string unit_;																//!< Pattern for the unit of the metric
			std::map<std::string, std::string> tags_;										//!< Pattern for the tags of the metric
			bool computeRate_;																//!< Whether the metric is a rate
			bool convertToUnitsPerSecond_;													//!< Whether the metric should be converted to units per second

		public:
			/**
			 * @brief Construct a new Matcher object
			 */
			Matcher() noexcept;

			/**
			 * @brief Construct a new Matcher object
			 *
			 * @param config configuration to use
			 */
			Matcher(const Config::expression::metric& config) noexcept;


			/**
			 * @brief Returns the pattern for the name of the metric
			 */
			const std::vector<std::string>& name() const noexcept;

			/**
			 * @brief Returns the pattern for the value of the metric
			 */
			const std::string& value() const noexcept;

			/**
			 * @brief Returns the pattern for the unit of the metric
			 */
			const std::string& unit() const noexcept;

			/**
			 * @brief Returns the pattern for the tags of the metric
			 */
			const std::map<std::string, std::string>& tags() const noexcept;

			/**
			 * @brief Returns whether the metric is a rate
			 */
			bool computeRate() const noexcept;

			/**
			 * @brief Returns whether the metric should be converted to units per second
			 */
			bool convertToUnitsPerSecond() const noexcept;


			/**
			 * @brief Sets the pattern for the name of the metric
			 */
			void setName(const std::vector<std::string>& name) noexcept;

			/**
			 * @brief Sets the pattern for the value of the metric
			 */
			void setValue(const std::string& value) noexcept;

			/**
			 * @brief Sets the pattern for the unit of the metric
			 */
			void setUnit(const std::string& unit) noexcept;

			/**
			 * @brief Sets the pattern for the tags of the metric
			 */
			void setTags(const std::map<std::string, std::string>& tags) noexcept;

			/**
			 * @brief Sets whether the metric is a rate
			 */
			void setComputeRate(bool computeRate) noexcept;

			/**
			 * @brief Sets whether the metric should be converted to units per second
			 */
			void setConvertToUnitsPerSecond(bool convertToUnitsPerSecond) noexcept;


			/**
			 * @brief Use an expression match to compute the metric's name
			 *
			 * @param match expression match to use
			 * @param pathParts parts of the source file's path, if any
			 * @return the matched name, or an empty `std::optional` if it couldn't be matched
			 */
			std::optional<std::vector<std::string>> getName(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;

			/**
			 * @brief Use an expression match to compute the metric's value
			 *
			 * @param match expression match to use
			 * @param pathParts parts of the source file's path, if any
			 * @return the matched value, or an empty `std::optional` if it couldn't be matched
			 */
			std::optional<double> getValue(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;

			/**
			 * @brief Use an expression match to compute the metric's unit
			 *
			 * @param match expression match to use
			 * @param pathParts parts of the source file's path, if any
			 * @return the matched unit, or an empty `std::optional` if it couldn't be matched
			 */
			std::optional<std::string> getUnit(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;

			/**
			 * @brief Use an expression match to compute the metric's tags
			 *
			 * @param match expression match to use
			 * @param pathParts parts of the source file's path, if any
			 * @return the matched tags, or an empty `std::optional` if it couldn't be matched
			 */
			std::optional<std::map<std::string, std::string>> getTags(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;

			/**
			 * @brief Use an expression match to compute the metric
			 *
			 * @param match expression match to use
			 * @param pathParts parts of the source file's path, if any
			 * @return the computed metric, or an empty `std::optional` if any of its fields couldn't be matched
			 */
			std::optional<Metric> getMetric(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
	};
}
