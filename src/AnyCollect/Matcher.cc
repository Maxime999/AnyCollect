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

#include <tinyexpr/tinyexpr.h>

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
		bool escaped = false;
		while (*buffer != '\0') {
			if (escaped) {
				if (*buffer == Matcher::matchSubstitutionPrefix || *buffer ==  Matcher::matchEscapeChar) {
					size_t pos = buffer - str.data() - 1;
					str.erase(pos, 1);
					buffer = str.data() + pos;
				}
				escaped = false;
				buffer++;
			}
			else if (*buffer == Matcher::matchEscapeChar) {
				escaped = true;
				buffer++;
			}
			else if (*buffer == Matcher::matchSubstitutionPrefix) {
				size_t pos = buffer - str.data();
				buffer++;
				bool replace = false;
				std::string_view replacementString;
				// Match $123
				if (isdigit(*buffer)) {
					replace = true;
					size_t i = parseUint(buffer);
					if (i < match.size() && match[i].matched)
						replacementString = std::string_view{match[i].first, static_cast<size_t>(std::distance(match[i].first, match[i].second))};
				}
				// Match $path_123
				else if (pos + Matcher::matchSubstitutionPathPrefix.size() + 1 < str.size() && strncmp(buffer, Matcher::matchSubstitutionPathPrefix.data(), Matcher::matchSubstitutionPathPrefix.size()) == 0 && isdigit(*(buffer + Matcher::matchSubstitutionPathPrefix.size()))) {
					replace = true;
					buffer += Matcher::matchSubstitutionPathPrefix.size();
					size_t i = parseUint(buffer);
					if (i < pathParts.size())
						replacementString = std::string_view{pathParts[i].data()};
				}

				if (replace) {
					str.replace(pos, buffer - str.data() - pos, replacementString);
					buffer = str.data() + pos + replacementString.size();
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
		std::string expression = this->value_;
		replaceMatches(expression, match, pathParts);
		int error = 0;
		double value = te_interp(expression.c_str(), &error);
		if (!error)
			return std::make_optional(value);
		return std::optional<double>{};
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
