//
// File.cc
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

#include <glob.h>

#include <boost/filesystem.hpp>

#include "File.h"

namespace fs = boost::filesystem;


namespace AnyCollect {
	File::File(const std::string& path) noexcept {
		this->path_ = path;
		fs::path fspath{path};
		for (const auto& pathPart : fspath.relative_path())
			this->pathParts_.push_back(pathPart.string());
		this->reset();
	}


	std::vector<std::string> File::filePathsMatchingGlobbingPattern(const std::string& pattern) noexcept {
		std::vector<std::string> paths;
		glob_t globbuf;

		int r = glob(pattern.c_str(), 0, NULL, &globbuf);
		if (r == 0) {
			for (size_t i = 0; i < globbuf.gl_pathc; i++) {
				if (fs::is_regular_file(fs::status(globbuf.gl_pathv[i])))
					paths.emplace_back(globbuf.gl_pathv[i]);
			}
		}

		globfree(&globbuf);
		return paths;
	}


	const std::string& File::path() const noexcept {
		return this->path_;
	}

	const std::vector<std::string>& File::pathParts() const noexcept {
		return this->pathParts_;
	}

	const std::string_view& File::contents() const noexcept {
		return this->contents_;
	}

	const std::chrono::system_clock::time_point& File::timestamp() const noexcept {
		return this->timestamp_;
	}

	std::vector<std::shared_ptr<Expression>>& File::expressions() noexcept {
		return this->expressions_;
	}

	const std::vector<std::shared_ptr<Expression>>& File::expressions() const noexcept {
		return this->expressions_;
	}


	bool File::reset() noexcept {
		errno = 0;
		this->buffer_.clear();
		this->buffer_.resize(1024, '\0');
		this->file_ = std::ifstream(this->path_, std::ios::binary);
		if (errno) {
			perror(std::string(this->path_).append(": Error opening file").c_str());
			errno = 0;
			return false;
		}

		this->file_.clear();
		this->file_.seekg(0);
		this->file_.read(this->buffer_.data(), this->buffer_.size());
		if (errno) {
			perror(std::string(this->path_).append(": Error reading file").c_str());
			errno = 0;
			return false;
		}

		while (!this->file_.eof()) {
			this->buffer_.resize(this->buffer_.size() * 2, '\0');
			this->file_.clear();
			this->file_.seekg(0);
			this->file_.read(this->buffer_.data(), this->buffer_.size());
		}
		this->buffer_.resize(this->buffer_.size() * 2, '\0');
		return true;
	}

	bool File::read() noexcept {
		errno = 0;
		this->file_.clear();
		this->file_.seekg(0);
		this->file_.read(this->buffer_.data(), this->buffer_.size());
		if (errno) {
			perror(std::string(this->path_).append(": Error reading file").c_str());
			errno = 0;
			return this->reset();
		}
		this->buffer_[this->file_.gcount()] = '\0';
		this->contents_ = std::string_view{this->buffer_.data(), static_cast<size_t>(this->file_.gcount()) + 1};
		this->timestamp_ = std::chrono::system_clock::now();
		return true;
	}

	std::string_view::const_iterator File::begin() const noexcept {
		return this->contents_.cbegin();
	}

	std::string_view::const_iterator File::end() const noexcept {
		return this->contents_.cend();
	}

	std::string_view::const_iterator File::getLine(std::string_view::const_iterator begin) const noexcept {
		auto end = begin;
		while (end != this->end() && *end != '\n')
			end++;
		if (end != this->end())
			end++;
		return end;
	}
}
