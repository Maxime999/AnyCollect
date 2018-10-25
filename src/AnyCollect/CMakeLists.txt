#
# CMakeList.txt
# AnyCollect library cmake file
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


FILE(GLOB AnyCollectSources *.cc)
FILE(GLOB AnyCollectHeaders *.h)

add_library(AnyCollect STATIC ${AnyCollectSources})

target_compile_options(AnyCollect PUBLIC ${GLOBAL_CXX_COMPILE_OPTIONS})
include_directories(${CMAKE_SOURCE_DIR}/src)

if(GPERFTOOLS_CPU_PROFILE)
	find_library(PROFILER_LIB profiler)
	target_link_libraries(AnyCollect ${PROFILER_LIB})
endif()

if(GPERFTOOLS_MEM_PROFILE)
	find_library(TCMALLOC_LIB tcmalloc)
	target_link_libraries(AnyCollect ${TCMALLOC_LIB})
endif()

# install (TARGETS AnyCollect
# 	DESTINATION lib)

# install(FILES ${AnyCollectHeaders}
# 	DESTINATION include/AnyCollect)
