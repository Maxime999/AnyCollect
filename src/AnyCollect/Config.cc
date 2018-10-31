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
#include "Source.h"


namespace AnyCollect {
	Config::Config(const std::string& path) noexcept {
		try {
			Source configFile = Source{path};
			configFile.update();
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


	void from_json(const nlohmann::json& je, Config::expression& e) noexcept {
		e.regex = getValue<Config::expression::regexType>(je, Config::expression::regexKey);
		for (const auto& jem : getValue<Config::expression::metricsType>(je, Config::expression::metricsKey)) {
			Config::expression::metric m;
			m.name = getValue<Config::expression::metric::nameType>(jem, Config::expression::metric::nameKey);
			m.value = getValue<Config::expression::metric::valueType>(jem, Config::expression::metric::valueKey);
			m.unit = getValue<Config::expression::metric::unitType>(jem, Config::expression::metric::unitKey);
			m.tags = getValue<Config::expression::metric::tagsType>(jem, Config::expression::metric::tagsKey);
			m.computeRate = getValue<Config::expression::metric::computeRateType>(jem, Config::expression::metric::computeRateKey);
			m.convertToUnitsPerSecond = getValue<Config::expression::metric::convertToUnitsPerSecondType>(jem, Config::expression::metric::convertToUnitsPerSecondKey);
			e.metrics.push_back(std::move(m));
		}
	}

	void from_json(const nlohmann::json& j, Config& c) noexcept {
		if (j.count(std::string(Config::filesKey)) > 0) {
			for (const auto& jf : getValue<Config::filesType>(j, Config::filesKey)) {
				Config::file f;
				f.paths = getValue<Config::file::pathsType>(jf, Config::file::pathsKey);
				for (const auto& jfe : getValue<Config::file::expressionsType>(jf, Config::file::expressionsKey)) {
					Config::expression e;
					from_json(jfe, e);
					f.expressions.push_back(std::move(e));
				}
				c.files.push_back(std::move(f));
			}
		}
		if (j.count(std::string(Config::commandsKey)) > 0) {
			for (const auto& jp : getValue<Config::commandsType>(j, Config::commandsKey)) {
				Config::command p;
				p.program = getValue<Config::command::programType>(jp, Config::command::programKey);
				p.arguments = getValue<Config::command::argumentsType>(jp, Config::command::argumentsKey);
				for (const auto& jpe : getValue<Config::command::expressionsType>(jp, Config::command::expressionsKey)) {
					Config::expression e;
					from_json(jpe, e);
					p.expressions.push_back(std::move(e));
				}
				c.commands.push_back(std::move(p));
			}
		}
	}
}
