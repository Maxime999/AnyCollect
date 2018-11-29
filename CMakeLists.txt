#
# CMakeList.txt
# AnyCollect project cmake file
#
# Copyright 2018 CFM (www.cfm.fr)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


cmake_minimum_required(VERSION 2.8.4)

project(AnyCollect)

option(PROFILE "Make executables easier to profile" OFF)
option(GPERFTOOLS_CPU_PROFILE "Enable CPU profiling with GPerf Tools" OFF)
option(GPERFTOOLS_MEM_PROFILE "Enable Memory profiling with GPerf Tools" OFF)

set(VERSION_MAJOR   1   CACHE STRING "Project major version number.")
set(VERSION_MINOR   1   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   2   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)


set(GLOBAL_COMPILE_OPTIONS -pedantic -Wall -Wextra -W)

if(PROFILE)
	set(GLOBAL_COMPILE_OPTIONS ${GLOBAL_COMPILE_OPTIONS} -UPRINT_METRICS -DPROFILING -fno-omit-frame-pointer)
endif()

if(GPERFTOOLS_CPU_PROFILE)
	set(GLOBAL_COMPILE_OPTIONS ${GLOBAL_COMPILE_OPTIONS} -DGPERFTOOLS_CPU_PROFILE)
endif()


set(GLOBAL_C_COMPILE_OPTIONS		${CMAKE_C_FLAGS}   ${GLOBAL_COMPILE_OPTIONS} -std=gnu11)
set(GLOBAL_CXX_COMPILE_OPTIONS		${CMAKE_CXX_FLAGS} ${GLOBAL_COMPILE_OPTIONS} -std=c++17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG			"-O0 -ggdb -DDEBUG  -DPRINT_METRICS")
set(CMAKE_CXX_FLAGS_RELEASE			"-O3 -g    -DNDEBUG -DPRINT_METRICS")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"-O3 -ggdb -DNDEBUG -DPRINT_METRICS")
set(CMAKE_CXX_FLAGS_MINSIZEREL		"-O3       -DNDEBUG -DPRINT_METRICS")


add_subdirectory(src/AnyCollect)
add_subdirectory(src/AnyCollectValues)
add_subdirectory(src/AnyCollectSnap)

add_subdirectory(doc)
