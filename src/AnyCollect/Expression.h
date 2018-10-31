//
// Expression.h
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

#include <memory>
#include <regex>

#include "Matcher.h"


namespace AnyCollect {
	class Expression {
		protected:
			static std::cmatch match;
			std::regex regex_;
			std::vector<std::shared_ptr<Matcher>> matchers_;

		public:
			Expression(const std::string& pattern) noexcept;

			std::vector<std::shared_ptr<Matcher>>& matchers() noexcept;
			const std::vector<std::shared_ptr<Matcher>>& matchers() const noexcept;

			const std::cmatch& apply(std::string_view::const_iterator begin, std::string_view::const_iterator end);
	};
}
