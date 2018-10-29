//
// File.h
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

#include <chrono>
#include <fstream>
#include <memory>
#include <string_view>
#include <vector>


#include "Expression.h"


namespace AnyCollect {
	class File {
		protected:
			std::string path_;
			std::vector<std::string> pathParts_;
			std::ifstream file_;
			std::vector<char> buffer_;

			std::string_view contents_;
			std::chrono::system_clock::time_point timestamp_;

			std::vector<std::shared_ptr<Expression>> expressions_;

		public:
			File(const std::string& path) noexcept;

			static std::vector<std::string> filePathsMatchingGlobbingPattern(const std::string& pattern) noexcept;

			const std::string& path() const noexcept;
			const std::vector<std::string>& pathParts() const noexcept;
			const std::string_view& contents() const noexcept;
			const std::chrono::system_clock::time_point& timestamp() const noexcept;
			std::vector<std::shared_ptr<Expression>>& expressions() noexcept;
			const std::vector<std::shared_ptr<Expression>>& expressions() const noexcept;

			bool reset() noexcept;
			bool read() noexcept;

			std::string_view::const_iterator begin() const noexcept;
			std::string_view::const_iterator end() const noexcept;
			std::string_view::const_iterator getLine(std::string_view::const_iterator begin) const noexcept;
	};
}
