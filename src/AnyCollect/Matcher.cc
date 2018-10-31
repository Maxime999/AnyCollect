//
// Matcher.cc
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

#include <cmath>

#include "Matcher.h"


namespace AnyCollect {
	Matcher::Matcher() noexcept { }

	Matcher::Matcher(const Config::expression::metric& config) noexcept :
		name_(config.name),
		value_(config.value),
		unit_(config.unit),
		tags_(config.tags),
		computeRate_(config.computeRate),
		convertToUnitsPerSecond_(config.convertToUnitsPerSecond)
	{ }


	const std::vector<std::string>& Matcher::name() const noexcept {
		return this->name_;
	}

	const std::string& Matcher::value() const noexcept {
		return this->value_;
	}

	const std::string& Matcher::unit() const noexcept {
		return this->unit_;
	}

	const std::map<std::string, std::string>& Matcher::tags() const noexcept {
		return this->tags_;
	}

	bool Matcher::computeRate() const noexcept {
		return this->computeRate_;
	}

	bool Matcher::convertToUnitsPerSecond() const noexcept {
		return this->convertToUnitsPerSecond_;
	}


	void Matcher::setName(const std::vector<std::string>& name) noexcept {
		this->name_ = name;
	}

	void Matcher::setValue(const std::string& value) noexcept {
		this->value_ = value;
	}

	void Matcher::setUnit(const std::string& unit) noexcept {
		this->unit_ = unit;
	}

	void Matcher::setTags(const std::map<std::string, std::string>& tags) noexcept {
		this->tags_ = tags;
	}

	void Matcher::setComputeRate(bool computeRate) noexcept {
		this->computeRate_ = computeRate;
	}

	void Matcher::setConvertToUnitsPerSecond(bool convertToUnitsPerSecond) noexcept {
		this->convertToUnitsPerSecond_ = convertToUnitsPerSecond;
	}


	inline uint64_t parseUint(const char*& buffer) noexcept {
		uint64_t result = 0;
		while (!std::isdigit(*buffer))
			buffer++;
		while (std::isdigit(*buffer)) {
			result = (result << 1) + (result << 3) + *buffer - '0';
			buffer++;
		}
		return result;
	}

	inline void replaceMatches(std::string& str, const std::cmatch& match, const std::vector<std::string>& pathParts) {
		const char* buffer = str.data();
		while (*buffer != '\0') {
			if (*buffer == '\\') {
				buffer += 2;
			}
			else if (*buffer == Matcher::matchSubstitutionPrefix && isdigit(*(buffer + 1))) {
				size_t pos = buffer - str.data();
				buffer++;
				size_t i = parseUint(buffer);
				if (i < match.size()) {
					str.replace(pos, buffer - str.data() - pos, match[i].str());
					buffer = str.data() + pos + match[i].str().size();
				} else {
					str.replace(pos, buffer - str.data() - pos, "");
					buffer = str.data() + pos;
				}
			}
			else if (*buffer == Matcher::matchSubstitutionPrefix && Matcher::matchSubstitutionPathPrefix.compare(0, Matcher::matchSubstitutionPathPrefix.size(), buffer + 1, Matcher::matchSubstitutionPathPrefix.size()) == 0) {
				size_t pos = buffer - str.data();
				buffer += Matcher::matchSubstitutionPathPrefix.size();
				size_t i = parseUint(buffer);
				if (i < pathParts.size()) {
					str.replace(pos, buffer - str.data() - pos, pathParts[i]);
					buffer = str.data() + pos + pathParts[i].size();
				} else {
					str.replace(pos, buffer - str.data() - pos, "");
					buffer = str.data() + pos;
				}
			}
			else {
				buffer++;
			}
		}
	}


	std::optional<std::vector<std::string>> Matcher::getName(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept {
		std::vector<std::string> name = this->name_;
		for (auto& part : name) {
			replaceMatches(part, match, pathParts);
			if (part.empty())
				return std::optional<std::vector<std::string>>{};
		}
		return std::make_optional(std::move(name));
	}

	std::optional<double> Matcher::getValue(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept {
		std::string value = this->value_;
		replaceMatches(value, match, pathParts);
		try {
			return std::make_optional(std::stod(value));
		}
		catch(const std::exception& e) {
			return std::optional<double>{};
		}
	}

	std::optional<std::string> Matcher::getUnit(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept {
		std::string unit = this->unit_;
		replaceMatches(unit, match, pathParts);
		return std::make_optional(std::move(unit));
	}

	std::optional<std::map<std::string, std::string>> Matcher::getTags(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept {
		std::map<std::string, std::string> tags;
		for (auto& [key, value] : this->tags_) {
			auto keyCopy = key;
			auto valueCopy = value;
			replaceMatches(keyCopy, match, pathParts);
			replaceMatches(valueCopy, match, pathParts);
			if (keyCopy.empty() || valueCopy.empty())
				return std::optional<std::map<std::string, std::string>>{};
			else
				tags.insert_or_assign(std::move(keyCopy), std::move(valueCopy));
		}
		return std::make_optional(std::move(tags));
	}

	std::optional<Metric> Matcher::getMetric(const std::cmatch& match, const std::vector<std::string>& pathParts) const noexcept {
		auto name = this->getName(match, pathParts);
		if (!name.has_value())
			return std::optional<Metric>{};
		auto unit = this->getUnit(match, pathParts);
		if (!unit.has_value())
			return std::optional<Metric>{};
		auto tags = this->getTags(match, pathParts);
		if (!tags.has_value())
			return std::optional<Metric>{};
		return std::make_optional<Metric>(std::move(name.value()), std::move(tags.value()), std::move(unit.value()));
	}
}
