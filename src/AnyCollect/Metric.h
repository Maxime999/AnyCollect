//
// Metric.h
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

#include <chrono>
#include <map>
#include <string>
#include <vector>


namespace AnyCollect {
	class Metric {
		protected:
			static std::hash<std::string> hasher;
			size_t key_;
			size_t roundKey_;
			std::vector<std::string> name_;
			double previousValue_;
			double value_;
			std::chrono::system_clock::time_point timestamp_;
			std::string unit_;
			std::map<std::string, std::string> tags_;

		public:
			Metric(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags, const std::string& unit = "") noexcept;

			static size_t generateKey(const std::vector<std::string>& name, const std::map<std::string, std::string>& tags);
			static size_t generateKey(const Metric& metric);

			const std::vector<std::string>& name() const noexcept;
			size_t key() const noexcept;
			size_t roundKey() const noexcept;
			double value() const noexcept;
			std::chrono::system_clock::time_point timestamp() const noexcept;
			const std::string& unit() const noexcept;
			const std::map<std::string, std::string>& tags() const noexcept;

			void setNewValue(double value, bool computeRate, double unitsPerSecondFactor = 1.0) noexcept;
			void updateValue(double value, double unitsPerSecondFactor = 1.0) noexcept;
			void setTimestamp(std::chrono::system_clock::time_point timestamp) noexcept;
			void setRoundKey(size_t roundKey) noexcept;
	};
}
