#
# CMakeList.txt
# AnyCollectValues program cmake file
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


FILE(GLOB AnyCollectValuesSources *.cc)

add_executable(AnyCollectValues ${AnyCollectValuesSources})

target_compile_options(AnyCollectValues PUBLIC ${GLOBAL_CXX_COMPILE_OPTIONS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AnyCollectValues AnyCollect)

if(GPERFTOOLS_CPU_PROFILE)
	find_library(PROFILER_LIB profiler)
	target_link_libraries(AnyCollectValues ${PROFILER_LIB})
endif()

install(TARGETS AnyCollectValues
	DESTINATION bin)
