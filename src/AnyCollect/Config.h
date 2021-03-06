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
	/**
	 * @brief Struct used to represent the JSON configuration file
	 */
	struct Config {
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
			std::vector<Config::expression::metric> metrics;
		};

		struct file {
			static constexpr std::string_view pathsKey = "Paths"sv;
			using pathsType = std::vector<std::string>;
			static constexpr std::string_view expressionsKey = "Expressions"sv;
			using expressionsType = std::vector<nlohmann::json>;

			pathsType paths;
			std::vector<Config::expression> expressions;
		};

		struct command {
			static constexpr std::string_view programKey = "Program"sv;
			using programType = std::string;
			static constexpr std::string_view argumentsKey = "Arguments"sv;
			using argumentsType = std::vector<std::string>;
			static constexpr std::string_view expressionsKey = "Expressions"sv;
			using expressionsType = std::vector<nlohmann::json>;

			programType program;
			argumentsType arguments;
			std::vector<Config::expression> expressions;
		};

		static constexpr std::string_view filesKey = "Files"sv;
		using filesType = std::vector<nlohmann::json>;
		static constexpr std::string_view commandsKey = "Commands"sv;
		using commandsType = std::vector<nlohmann::json>;

		std::vector<Config::file> files;
		std::vector<Config::command> commands;

		/**
		 * @brief Parses the specified config file into a Config object
		 *
		 * @param path path of the config file to parse
		 */
		Config(const std::string& path) noexcept;
    };


	/**
	 * @brief Converts a json object into a Config object
	 *
	 * @param j json object to parse
	 * @param c Config object to put json values into
	 */
	void from_json(const nlohmann::json& j, Config& c) noexcept;

	/**
	 * @brief Parse a JSON value of specified type from a JSON dictionary
	 *
	 * If the key is not in the JSON dictionary, the program's execution is aborted with an error.
	 *
	 * @tparam T Expected type of JSON object
	 * @tparam K Type of key
	 * @param j JSON dictionary
	 * @param key key of the JSON object to get
	 * @return the extracted value with specified key and type
	 */
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
