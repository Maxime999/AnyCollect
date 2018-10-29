//
// Config.cc
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

#include <iostream>

#include "Config.h"
#include "File.h"


namespace AnyCollect {
	Config::Config(const std::string& path) {
		try {
			File configFile = File{path};
			configFile.read();
			nlohmann::json configJson = nlohmann::json::parse(configFile.contents());
			from_json(configJson, *this);
		}
		catch(const std::exception& e) {
			std::cerr << "Error while parsing configuration file:\n" << e.what() << '\n';
			abort();
		}
	}

	void from_json(const nlohmann::json& j, Config& c) {
		try {
			for (const auto& jf : j[std::string(Config::filesKey)].get<Config::filesType>()) {
				Config::file f;
				f.paths = jf[std::string(Config::file::pathsKey)].get<Config::file::pathsType>();
				for (const auto& jfe : jf[std::string(Config::file::expressionsKey)].get<Config::file::expressionsType>()) {
					Config::file::expression e;
					e.regex = jfe[std::string(Config::file::expression::regexKey)].get<Config::file::expression::regexType>();
					for (const auto& jfem : jfe[std::string(Config::file::expression::metricsKey)].get<Config::file::expression::metricsType>()) {
						Config::file::expression::metric m;
						m.name = jfem[std::string(Config::file::expression::metric::nameKey)].get<Config::file::expression::metric::nameType>();
						m.value = jfem[std::string(Config::file::expression::metric::valueKey)].get<Config::file::expression::metric::valueType>();
						m.unit = jfem[std::string(Config::file::expression::metric::unitKey)].get<Config::file::expression::metric::unitType>();
						m.tags = jfem[std::string(Config::file::expression::metric::tagsKey)].get<Config::file::expression::metric::tagsType>();
						m.computeRate = jfem[std::string(Config::file::expression::metric::computeRateKey)].get<Config::file::expression::metric::computeRateType>();
						m.convertToUnitsPerSecond = jfem[std::string(Config::file::expression::metric::convertToUnitsPerSecondKey)].get<Config::file::expression::metric::convertToUnitsPerSecondType>();
						e.metrics.push_back(std::move(m));
					}
					f.expressions.push_back(std::move(e));
				}
				c.files.push_back(std::move(f));
			}
		}
		catch(const std::exception& e) {
			std::cerr << "Error while parsing configuration file:\n" << e.what() << '\n';
			abort();
		}
	}
}
