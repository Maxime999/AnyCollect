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

#include "Config.h"
#include "File.h"


namespace AnyCollect {
	Config::Config(const std::string& path) noexcept {
		try {
			File configFile = File{path};
			configFile.read();
			if (configFile.contents().empty())
				return;

			nlohmann::json configJson = nlohmann::json::parse(configFile.contents());
			from_json(configJson, *this);
		}
		catch(const std::exception& e) {
			std::cerr << "Error while parsing configuration file: invalid JSON contents." <<std::endl;
			std::cerr << "Internal error: " << e.what() << std::endl;
			abort();
		}
	}

	void from_json(const nlohmann::json& j, Config& c) noexcept {
		for (const auto& jf : getValue<Config::filesType>(j, Config::filesKey)) {
			Config::file f;
			f.paths = getValue<Config::file::pathsType>(jf, Config::file::pathsKey);
			for (const auto& jfe : getValue<Config::file::expressionsType>(jf, Config::file::expressionsKey)) {
				Config::file::expression e;
				e.regex = getValue<Config::file::expression::regexType>(jfe, Config::file::expression::regexKey);
				for (const auto& jfem : getValue<Config::file::expression::metricsType>(jfe, Config::file::expression::metricsKey)) {
					Config::file::expression::metric m;
					m.name = getValue<Config::file::expression::metric::nameType>(jfem, Config::file::expression::metric::nameKey);
					m.value = getValue<Config::file::expression::metric::valueType>(jfem, Config::file::expression::metric::valueKey);
					m.unit = getValue<Config::file::expression::metric::unitType>(jfem, Config::file::expression::metric::unitKey);
					m.tags = getValue<Config::file::expression::metric::tagsType>(jfem, Config::file::expression::metric::tagsKey);
					m.computeRate = getValue<Config::file::expression::metric::computeRateType>(jfem, Config::file::expression::metric::computeRateKey);
					m.convertToUnitsPerSecond = getValue<Config::file::expression::metric::convertToUnitsPerSecondType>(jfem, Config::file::expression::metric::convertToUnitsPerSecondKey);
					e.metrics.push_back(std::move(m));
				}
				f.expressions.push_back(std::move(e));
			}
			c.files.push_back(std::move(f));
		}
	}
}
