//
// Metric.cc
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

#include "Metric.h"


namespace AnyCollect {
	std::hash<std::string> Metric::hasher{};

	Metric::Metric(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags, const std::string& unit) noexcept :
		roundKey_(-1),
		name_(name),
		unit_(unit),
		tags_(tags)
	{
		this->key_ = Metric::generateKey(this->name_, this->tags_);
	}


	Metric::Metric(std::vector<std::string>&& name, std::map<std::string, std::string>&& tags, std::string&& unit) noexcept :
		roundKey_(-1),
		name_(name),
		unit_(unit),
		tags_(tags)
	{
		this->key_ = Metric::generateKey(this->name_, this->tags_);
	}


	Metric::Metric(const Metric& other) noexcept :
		key_(other.key_),
		roundKey_(other.roundKey_),
		name_(other.name_),
		previousValue_(other.previousValue_),
		value_(other.value_),
		timestamp_(other.timestamp_),
		unit_(other.unit_),
		tags_(other.tags_)
	{ }

	Metric::Metric(Metric&& other) noexcept :
		key_(other.key_),
		roundKey_(other.roundKey_),
		name_(std::move(other.name_)),
		previousValue_(other.previousValue_),
		value_(other.value_),
		timestamp_(other.timestamp_),
		unit_(std::move(other.unit_)),
		tags_(std::move(other.tags_))
	{ }

	Metric& Metric::operator=(Metric other) noexcept {
		this->key_ = other.key_;
		this->roundKey_ = other.roundKey_;
		std::swap(this->name_, other.name_);
		this->previousValue_ = other.previousValue_;
		this->value_ = other.value_;
		this->timestamp_ = other.timestamp_;
		std::swap(this->unit_, other.unit_);
		std::swap(this->tags_, other.tags_);
		return *this;
	}


	size_t Metric::generateKey(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags) {
		std::string hash;
		for (const auto& n : name)
			hash.append(n);
		for (const auto& [k, v] : tags) {
			hash.append(k);
			hash.append(v);
		}
		return Metric::hasher(hash);
	}

	const std::vector<std::string>& Metric::name() const noexcept {
		return this->name_;
	}

	size_t Metric::key() const noexcept {
		return this->key_;
	}

	size_t Metric::roundKey() const noexcept {
		return this->roundKey_;
	}

	double Metric::value() const noexcept {
		return this->value_;
	}

	std::chrono::system_clock::time_point Metric::timestamp() const noexcept {
		return this->timestamp_;
	}

	const std::string& Metric::unit() const noexcept {
		return this->unit_;
	}

	const std::map<std::string, std::string>& Metric::tags() const noexcept {
		return this->tags_;
	}


	void Metric::setNewValue(double value, bool computeRate, double unitsPerSecondFactor) noexcept {
		if (computeRate)
			this->value_ = value - this->previousValue_;
		else
			this->value_ = value;
		this->previousValue_ = value;
		this->value_ *= unitsPerSecondFactor;
	}

	void Metric::updateValue(double value, double unitsPerSecondFactor) noexcept {
		this->value_ /= unitsPerSecondFactor;
		this->value_ += value;
		this->previousValue_ += value;
		this->value_ *= unitsPerSecondFactor;
	}

	void Metric::setTimestamp(std::chrono::system_clock::time_point timestamp) noexcept {
		this->timestamp_ = timestamp;
	}

	void Metric::setRoundKey(size_t roundKey) noexcept{
		this->roundKey_ = roundKey;
	}
}
