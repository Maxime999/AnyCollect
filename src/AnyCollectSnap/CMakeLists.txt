#
# CMakeList.txt
# AnyCollectSnap program cmake file
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


FILE(GLOB AnyCollectSnapSources *.cc)
FILE(GLOB AnyCollectSnapHeaders *.h)


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


add_executable(AnyCollectSnap ${AnyCollectSnapSources})

target_compile_options(AnyCollectSnap PUBLIC ${GLOBAL_CXX_COMPILE_OPTIONS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AnyCollectSnap AnyCollect)

find_static_library(snap SNAP_LIB)
find_static_library(grpc++ GRPCPP_LIB)
find_static_library(grpc GRPC_LIB)
find_static_library(protobuf PROTOBUF_LIB)
find_static_library(cppnetlib-uri CPPNETLIB_URI_LIB)
find_static_library(cppnetlib-client-connections CPPNETLIB_CLIENT_LIB)
find_static_library(cppnetlib-server-parsers CPPNETLIB_SERVER_LIB)
find_static_library(boost_program_options BOOST_PROGRAM_OPTIONS_LIB)
find_static_library(boost_system BOOST_SYSTEM_LIB)
find_static_library(boost_filesystem BOOST_FILESYSTEM_LIB)

target_link_libraries(AnyCollectSnap -Wl,--as-needed)
target_link_libraries(AnyCollectSnap ${SNAP_LIB})
target_link_libraries(AnyCollectSnap ${PROTOBUF_LIB} ${GRPCPP_LIB} ${GRPC_LIB})
target_link_libraries(AnyCollectSnap ${CPPNETLIB_URI_LIB} ${CPPNETLIB_CLIENT_LIB} ${CPPNETLIB_SERVER_LIB})
target_link_libraries(AnyCollectSnap ${BOOST_PROGRAM_OPTIONS_LIB} ${BOOST_SYSTEM_LIB} ${BOOST_FILESYSTEM_LIB})
target_link_libraries(AnyCollectSnap rt z pthread ssl crypto)

# target_link_libraries(AnyCollectSnap snap grpc grpc++ protobuf
#     cppnetlib-uri cppnetlib-client-connections cppnetlib-server-parsers
#     boost_program_options boost_system boost_filesystem
#     z pthread ssl crypto)

install(TARGETS AnyCollectSnap
	DESTINATION bin)
