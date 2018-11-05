//
// Source.h
//
// Created on October 31st 2018
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

#include <pstreams/pstream.h>

#include "Config.h"
#include "Expression.h"


namespace AnyCollect {
	class Source {
		public:
			enum SourceType {
				SourceTypeFile,
				SourceTypeCommand,
			};

		protected:
			SourceType type_;
			std::string path_;
			std::vector<std::string> pathParts_;
			std::ifstream file_;
			redi::ipstream process_;
			std::vector<char> buffer_;

			std::string_view contents_;
			std::chrono::system_clock::time_point timestamp_;

			std::vector<std::shared_ptr<Expression>> expressions_;

			size_t readFile(bool firstTime = false);
			size_t executeCommand(bool firstTime = false);

		public:
			Source(const std::string& filePath) noexcept;
			Source(const std::string& program, const std::vector<std::string>& arguments) noexcept;

			static std::vector<std::string> filePathsMatchingGlobbingPattern(const std::string& pattern) noexcept;

			SourceType type();
			const std::string& path() const noexcept;
			const std::vector<std::string>& pathParts() const noexcept;
			const std::string_view& contents() const noexcept;
			std::chrono::system_clock::time_point timestamp() const noexcept;
			std::vector<std::shared_ptr<Expression>>& expressions() noexcept;
			const std::vector<std::shared_ptr<Expression>>& expressions() const noexcept;

			bool reset() noexcept;
			bool update() noexcept;

			std::string_view::const_iterator begin() const noexcept;
			std::string_view::const_iterator end() const noexcept;
			std::string_view::const_iterator getLine(std::string_view::const_iterator begin) const noexcept;
	};
}
