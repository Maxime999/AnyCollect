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
	/**
	 * @brief Class used to represent a source (file or command output) from which metrics can be matched
	 */
	class Source {
		public:
		/**
		 * @brief Enum of existing source type
		 */
			enum SourceType {
				SourceTypeFile,			//!< File contents source
				SourceTypeCommand,		//!< Command output source
			};

		protected:
			SourceType type_;											//!< The type of the source
			std::string path_;											//!< Path of the file or command to execute
			std::vector<std::string> pathParts_;						//!< For file sources, the parts of the file's path
			std::ifstream file_;										//!< For file sources, the file descriptor
			redi::ipstream process_;									//!< For command sources, the child process
			std::vector<char> buffer_;									//!< Buffer for file contents or command output

			std::string_view contents_;									//!< Read-only view on the buffer_
			std::chrono::system_clock::time_point timestamp_;			//!< Last contents or output fetching time

			std::vector<std::shared_ptr<Expression>> expressions_;		//!< Array of expressions used on the source's contents

			size_t readFile(bool firstTime = false);					//!< For file sources, put the file contents into the buffer_
			size_t executeCommand(bool firstTime = false);				//!< For command sources, put the command output into the buffer_

		public:
			/**
			 * @brief Construct a new Source object of file type
			 *
			 * @param filePath the path of the file to read
			 */
			Source(const std::string& filePath) noexcept;

			/**
			 * @brief Construct a new Source object of command type
			 *
			 * @param program main command to execute
			 * @param arguments arguments to be added after the command
			 */
			Source(const std::string& program, const std::vector<std::string>& arguments) noexcept;


			/**
			 * @brief Returns all file paths that match a pattern as defined by POSIX
			 *
			 * @param pattern
			 * @return std::vector<std::string>
			 */
			static std::vector<std::string> filePathsMatchingGlobbingPattern(const std::string& pattern) noexcept;


			/**
			 * @brief Returns the source type
			 */
			SourceType type();

			/**
			 * @brief Returns the file path or command to execute
			 */
			const std::string& path() const noexcept;

			/**
			 * @brief For file sources, returns the different parts of the file's path
			 */
			const std::vector<std::string>& pathParts() const noexcept;

			/**
			 * @brief Returns the stored contents (file contents or command output)
			 */
			const std::string_view& contents() const noexcept;

			/**
			 * @brief Returns the timestamp of stored contents
			 */
			std::chrono::system_clock::time_point timestamp() const noexcept;

			/**
			 * @brief Returns the array of the receiver's expressions
			 */
			std::vector<std::shared_ptr<Expression>>& expressions() noexcept;

			/**
			 * @brief Returns the array of the receiver's expressions
			 */
			const std::vector<std::shared_ptr<Expression>>& expressions() const noexcept;


			/**
			 * @brief Resets the source: attempts to open the file or execute the command, allocates enough space for contents
			 *
			 * @return true if everything is setup correctly
			 * @return false otherwise
			 */
			bool reset() noexcept;

			/**
			 * @brief Updates the source (read the file or execute command) and store the results
			 *
			 * @return true if everything is ok
			 * @return false otherwise
			 */
			bool update() noexcept;


			/**
			 * @brief Returns the iterator to the beginning of the source's contents
			 *
			 * @return iterator to the beginning of the source's contents
			 */
			std::string_view::const_iterator begin() const noexcept;

			/**
			 * @brief Returns the iterator to the end of the source's contents
			 *
			 * @return iterator to the end of the source's contents
			 */
			std::string_view::const_iterator end() const noexcept;

			/**
			 * @brief Returns an iterator to the end of the specified line
			 *
			 * @param begin the line to find an end to
			 * @return iterator to the first end of line (or end of file) character after the specified iterator
			 */
			std::string_view::const_iterator getLine(std::string_view::const_iterator begin) const noexcept;
	};
}
