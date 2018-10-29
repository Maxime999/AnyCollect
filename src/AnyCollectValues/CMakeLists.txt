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


function(find_static_library LIB_NAME OUT)
    if (WIN32 OR MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    elseif (UNIX)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    endif()

    find_library(FOUND_${LIB_NAME}_STATIC ${LIB_NAME})

    if (FOUND_${LIB_NAME}_STATIC)
        get_filename_component(ABS_FILE ${FOUND_${LIB_NAME}_STATIC} ABSOLUTE)
    else()
        message(SEND_ERROR "Unable to find library ${LIB_NAME}")
    endif()

    set(${OUT} ${ABS_FILE} PARENT_SCOPE)
endfunction()


add_executable(AnyCollectValues ${AnyCollectValuesSources})

target_compile_options(AnyCollectValues PUBLIC ${GLOBAL_CXX_COMPILE_OPTIONS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AnyCollectValues AnyCollect)

find_static_library(boost_system BOOST_SYSTEM_LIB)
find_static_library(boost_filesystem BOOST_FILESYSTEM_LIB)

target_link_libraries(AnyCollectValues -Wl,--as-needed)
target_link_libraries(AnyCollectValues ${BOOST_SYSTEM_LIB} ${BOOST_FILESYSTEM_LIB})


if(GPERFTOOLS_CPU_PROFILE)
	find_library(PROFILER_LIB profiler)
	target_link_libraries(AnyCollectValues ${PROFILER_LIB})
endif()

install(TARGETS AnyCollectValues
	DESTINATION bin)
