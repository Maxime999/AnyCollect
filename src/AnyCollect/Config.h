//
// Config.h
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

#include <iostream>
#include <string_view>

#include <json.hpp>

using namespace std::literals;


namespace AnyCollect {
	struct Config {
		struct file {
			struct expression {
				struct metric {
					static constexpr std::string_view nameKey = "Name"sv;
					using nameType = std::vector<std::string>;
					static constexpr std::string_view valueKey = "Value"sv;
					using valueType = std::string;
					static constexpr std::string_view unitKey = "Unit"sv;
					using unitType = std::string;
					static constexpr std::string_view tagsKey = "Tags"sv;
					using tagsType = std::map<std::string, std::string>;
					static constexpr std::string_view computeRateKey = "ComputeRate"sv;
					using computeRateType = bool;
					static constexpr std::string_view convertToUnitsPerSecondKey = "ConvertToUnitsPerSecond"sv;
					using convertToUnitsPerSecondType = bool;

					nameType name;
					valueType value;
					unitType unit;
					tagsType tags;
					computeRateType computeRate;
					convertToUnitsPerSecondType convertToUnitsPerSecond;
				};
				static constexpr std::string_view regexKey = "Regex"sv;
				using regexType = std::string;
				static constexpr std::string_view metricsKey = "Metrics"sv;
				using metricsType = std::vector<nlohmann::json>;

				regexType regex;
				std::vector<metric> metrics;
			};
			static constexpr std::string_view pathsKey = "Paths"sv;
			using pathsType = std::vector<std::string>;
			static constexpr std::string_view expressionsKey = "Expressions"sv;
			using expressionsType = std::vector<nlohmann::json>;

			pathsType paths;
			std::vector<expression> expressions;
		};
		static constexpr std::string_view filesKey = "Files"sv;
		using filesType = std::vector<nlohmann::json>;

		std::vector<file> files;

		Config(const std::string& path) noexcept;
    };

	void from_json(const nlohmann::json& j, Config& c) noexcept;

	template<typename T, typename K>
	T getValue(const nlohmann::json& j, const K& key) noexcept {
		try {
			return j[std::string(key)].get<T>();
		}
		catch(const std::exception& e) {
			std::cerr << "Error while parsing configuration file: field named \"" << key << "\" of required type not found." << std::endl;
			std::cerr << "Internal error: " << e.what() << std::endl;
			abort();
		}
	}
}
