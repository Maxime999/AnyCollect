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

#include "Metric.h"

using namespace std::literals;


namespace AnyCollect {
	class Matcher {
		public:
			static constexpr char matchSubstitutionPrefix = '$';
			static constexpr std::string_view matchSubstitutionPathPrefix = "path_"sv;

		protected:
			std::vector<std::string> name_;
			std::string value_;
			std::string unit_;
			std::map<std::string, std::string> tags_;
			bool computeRate_;
			bool convertToUnitsPerSecond_;

		public:
			Matcher() noexcept;

			const std::vector<std::string>& name() const noexcept;
			const std::string& value() const noexcept;
			const std::string& unit() const noexcept;
			const std::map<std::string, std::string>& tags() const noexcept;
			bool computeRate() const noexcept;
			bool convertToUnitsPerSecond() const noexcept;

			void setName(const std::vector<std::string>& name) noexcept;
			void setValue(const std::string& value) noexcept;
			void setUnit(const std::string& unit) noexcept;
			void setTags(const std::map<std::string, std::string>& tags) noexcept;
			void setComputeRate(bool computeRate) noexcept;
			void setConvertToUnitsPerSecond(bool convertToUnitsPerSecond) noexcept;

			std::optional<std::vector<std::string>> getName(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
			std::optional<double> getValue(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
			std::optional<std::string> getUnit(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
			std::optional<std::map<std::string, std::string>> getTags(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
			std::optional<Metric> getMetric(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept;
	};
}
