//
// Source.cc
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

#include <glob.h>

#include <boost/filesystem.hpp>

#include "Source.h"

namespace fs = boost::filesystem;


namespace AnyCollect {
	Source::Source(const std::string& filePath) noexcept :
		type_(SourceTypeFile),
		path_(filePath)
	{
		fs::path fspath{this->path_};
		for (const auto& pathPart : fspath.relative_path())
			this->pathParts_.push_back(pathPart.string());
		this->reset();
	}

	Source::Source(const std::string& program, const std::vector<std::string>& arguments) noexcept :
		type_(SourceTypeCommand)
	{
		this->path_ = program;
		for (const auto& arg : arguments)
			this->path_ += " " + arg;
		this->reset();
	}


	std::vector<std::string> Source::filePathsMatchingGlobbingPattern(const std::string& pattern) noexcept {
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


	Source::SourceType Source::type() {
		return this->type_;
	}

	const std::string& Source::path() const noexcept {
		return this->path_;
	}

	const std::vector<std::string>& Source::pathParts() const noexcept {
		return this->pathParts_;
	}

	const std::string_view& Source::contents() const noexcept {
		return this->contents_;
	}

	std::chrono::system_clock::time_point Source::timestamp() const noexcept {
		return this->timestamp_;
	}

	std::vector<std::shared_ptr<Expression>>& Source::expressions() noexcept {
		return this->expressions_;
	}

	const std::vector<std::shared_ptr<Expression>>& Source::expressions() const noexcept {
		return this->expressions_;
	}


	size_t Source::readFile(bool firstTime) {
		errno = 0;
		if (firstTime) {
			this->file_ = std::ifstream(this->path_, std::ios::binary);
			if (errno) {
				perror(std::string(this->path_).append(": Error opening file").c_str());
				errno = 0;
				return -1;
			}
		}
		this->file_.clear();
		this->file_.seekg(0);
		this->file_.read(this->buffer_.data(), this->buffer_.size());
		if (errno) {
			perror(std::string(this->path_).append(": Error reading file").c_str());
			errno = 0;
			return -1;
		}
		return this->file_.gcount();
	}

	size_t Source::executeCommand(bool ) {
		errno = 0;
		if (this->process_.is_open())
			this->process_.close();
		this->process_.clear();
		this->process_.open(this->path_, redi::pstreambuf::pstdout | redi::pstreambuf::pstderr);
		this->process_.out().read(this->buffer_.data(), this->buffer_.size());
		if (this->process_.bad())
			return -1;
		else
			return this->process_.gcount();
	}


	bool Source::reset() noexcept {
		this->buffer_.clear();
		this->buffer_.resize(1024, '\0');

		switch (this->type_) {
			case SourceTypeFile:
				if (this->readFile(true) == ((size_t)-1))
					return false;
				while (!this->file_.eof()) {
					this->buffer_.resize(this->buffer_.size() * 2, '\0');
					if (this->readFile(true) == ((size_t)-1))
						return false;
				}
				break;
			case SourceTypeCommand:
				if (this->executeCommand(true) == ((size_t)-1))
					return false;
				while (!this->process_.eof()) {
					this->buffer_.resize(this->buffer_.size() * 2, '\0');
					if (this->executeCommand(true) == ((size_t)-1))
						return false;
				}
				break;
		}

		this->buffer_.resize(this->buffer_.size() * 2, '\0');
		return true;
	}

	bool Source::update() noexcept {
		size_t size = 0;
		switch (this->type_) {
			case SourceTypeFile:
				size = this->readFile(false);
				break;
			case SourceTypeCommand:
				size = this->executeCommand(false);
				break;
		}
		if (size == ((size_t)-1)) {
			if (!this->reset())
				return false;
		}

		this->buffer_[size] = '\0';
		this->contents_ = std::string_view{this->buffer_.data(), size};
		this->timestamp_ = std::chrono::system_clock::now();
		return true;
	}

	std::string_view::const_iterator Source::begin() const noexcept {
		return this->contents_.cbegin();
	}

	std::string_view::const_iterator Source::end() const noexcept {
		return this->contents_.cend();
	}

	std::string_view::const_iterator Source::getLine(std::string_view::const_iterator begin) const noexcept {
		auto end = begin;
		while (end != this->end() && *end != '\n')
			end++;
		return end;
	}
}
