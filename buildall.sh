#!/bin/bash
#
# build.sh
# Script to build AnyCollect, Snap Plugin C++ library and their dependencies
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


#####################
### Configuration ###
#####################

BOOST_VERSION_TAG="boost-1.68.0"
GRPC_VERSION_TAG="v1.15.1"
CPPNETLIB_VERSION_TAG="cpp-netlib-0.13.0-final"
CPPNETLIB_VERSION_BRANCH="0.13-release"
SPDLOG_VERSION_TAG="v1.1.0"
NLOHMANN_JSON_VERSION_TAG="v3.3.0"


#####################
### Prepare stuff ###
#####################

INTERACTIVE=0	# 0 is true
USE_LOCAL=1		# 1 is false
DEPS_PATH="$(pwd)/third_party"
DEPS_OUTPUT_PATH="$DEPS_PATH/build"
OUTPUT="$(pwd)/output"
CLEANUP=1

CURRENT_DIR=$(readlink -e $(dirname "$0"))
NCPU=$(lscpu -p | egrep -v '^#' | sort -u -t, -k 2,4 | wc -l)
MAKE_ARGS="-j$NCPU"
DEPS_OUTPUT_PATH_REL=../$(perl -e 'use File::Spec; print File::Spec->abs2rel(@ARGV) . "\n"' $DEPS_OUTPUT_PATH $DEPS_PATH)
DEPS_RETURN_PATH=$(perl -e 'use File::Spec; print File::Spec->abs2rel(@ARGV) . "\n"' $(pwd) $DEPS_PATH)


N="\e[0m" # Reset normal formatting
B="\e[1m" # Bold formatting

# Asks user yes/no question
# Usage: confirm [prompt] [y|n: default option]
function confirm() {
	if ! $(exit $INTERACTIVE); then
		case "$2" in
			[yY]) return 0 ;;
			[nN]) return 1 ;;
			"") return 0 ;;
		esac
	fi

    space="[y/n]"
	if [ -n "$2" ]; then
		case "$2" in
			[yY]) space="[Y/n]" ;;
			[nN]) space="[y/N]" ;;
		esac
	fi
	while true; do
		read -n 1 -p "$1 $space " yn
		if [[ "$yn" == "" && "$2" ]]; then
			yn=$2
		else
			echo
		fi
		case $yn in
			[Yy]*) return 0;;
			[Nn]*) return 1;;
		esac
	done
}

# Quits if there was an error
# Usage: quit_if_error [exit code] [error name]
function quit_if_error() {
	if [ "$1" -ne "0" ]; then
		echo
		echo
		echo -e "${B}Error while building $2$N"
		echo " Error code: $1"
		echo
		exit $1
	fi
}


while [[ $# > 0 ]]; do
	case $1 in
		-h|--help)
		echo "Usage: build.sh [options] -- [make options]"
		echo "    -i              disable interactive questions"
		echo "    -l              use local dependencies instead of cloning with git"
		echo "    -d path         path where dependencies are (default ./third_party)"
		echo "    -o path         where to put build output (default ./output)"
		echo "    -c              clean after building"
		exit 1 ;;
		-i)
		INTERACTIVE=1 ;;
		-l)
		USE_LOCAL=0 ;;
		-d)
		DEPS_PATH="$2"
		shift ;;
		-o)
		OUTPUT="$2"
		shift ;;
		-c)
		CLEANUP=0 ;;
		--)
		shift
		MAKE_ARGS="$@" ;;
	esac
	shift
done


if $(exit $INTERACTIVE); then
	echo -e "${B}Snap C++ library and AnyCollect builder$N"
	echo

	echo "$NCPU CPUs will be used to build."
	echo

	echo -e "${B}The following packages should be installed on your system:$N"
	echo "   - git"
	echo "   - wget"
	echo "   - build-essential"
	echo "   - pkg-config"
	echo "   - libtool"
	echo "   - cmake"
	echo "   - a C++17 compiler (such as GCC 7.2 from the Red Hat Developer Toolset 7)"
	echo "   - optionally doxygen (for documentation)"
	if ! confirm "Is everything set-up correctly?" y; then
		echo "Please install these dependencies first."
		exit 1
	fi
	echo
fi

source /opt/rh/devtoolset-7/enable


###########################################
### Build and install Snap dependencies ###
###########################################

mkdir -p $DEPS_OUTPUT_PATH/include
mkdir -p $DEPS_PATH
cd $DEPS_PATH

if ! $(exit $USE_LOCAL); then
	git submodule update --init

	git clone https://github.com/boostorg/boost
	cd boost
	git checkout tags/$BOOST_VERSION_TAG
	git submodule update --init
	cd ..

	git clone https://github.com/grpc/grpc
	cd ./grpc
	git checkout tags/$GRPC_VERSION_TAG
	git submodule update --init
	cd ..

	git clone https://github.com/cpp-netlib/cpp-netlib
	cd cpp-netlib
#	git checkout tags/$CPPNETLIB_VERSION_TAG
	git checkout $CPPNETLIB_VERSION_BRANCH
	git submodule update --init
	cd ..

	git clone https://github.com/gabime/spdlog
	cd spdlog
	git checkout tags/$SPDLOG_VERSION_TAG
	cd ..

	wget --no-check-certificate -O json.hpp https://github.com/nlohmann/json/releases/download/$NLOHMANN_JSON_VERSION_TAG/json.hpp
fi


echo
echo
echo -e "$B    Boost$N"
echo

cd boost
./bootstrap.sh --prefix=$DEPS_OUTPUT_PATH
quit_if_error $? "Boost (bootstrap.sh)"
./b2 $MAKE_ARGS
quit_if_error $? "Boost (b2)"
./b2 install
quit_if_error $? "Boost (b2 install)"
./b2 headers install
quit_if_error $? "Boost (b2 headers install)"
cd ..


echo
echo
echo -e "$B    Building gRPC & Protobuf$N"
echo

cd grpc
make $MAKE_ARGS
quit_if_error $? "gRPC (make)"
make install prefix=$DEPS_OUTPUT_PATH_REL
quit_if_error $? "gRPC (make install)"

cd ./third_party/protobuf
make install prefix=$DEPS_OUTPUT_PATH
quit_if_error $? "Protobuf (make install)"
cd ../../..


echo
echo
echo -e "$B    Building cpp-netlib$N"
echo

mkdir -p cpp-netlib-build
cd cpp-netlib-build
cmake -DCMAKE_BUILD_TYPE=Release -DCPP-NETLIB_BUILD_TESTS=OFF -DCPP-NETLIB_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=$DEPS_OUTPUT_PATH ../cpp-netlib
quit_if_error $? "cpp-netlib (cmake)"
make $MAKE_ARGS
quit_if_error $? "cpp-netlib (make)"
make install
quit_if_error $? "cpp-netlib (make install)"
cd ..


echo
echo
echo -e "$B    Building tinyexpr$N"
echo

mkdir -p tinyexpr-build
cd tinyexpr-build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$DEPS_OUTPUT_PATH ../tinyexpr
quit_if_error $? "tinyexpr (cmake)"
make $MAKE_ARGS
quit_if_error $? "tinyexpr (make)"
make install
quit_if_error $? "tinyexpr (make install)"
cd ..


echo
echo
echo -e "$B    Copying header libraries$N"
echo

cp -r ./spdlog/include/spdlog $DEPS_OUTPUT_PATH/include
quit_if_error $? "spdlog (cp)"

cp ./json.hpp $DEPS_OUTPUT_PATH/include/json.hpp
quit_if_error $? "nlohmann-json (cp)"

mkdir -p $DEPS_OUTPUT_PATH/include/pstreams
cp ./pstreams/pstream.h $DEPS_OUTPUT_PATH/include/pstreams/pstream.h
quit_if_error $? "pstreams (cp)"


######################################
### Build and install Snap library ###
######################################

echo
echo
echo -e "$B    Building Snap C++ library$N"
echo

cd snap-plugin-lib-cpp
cd src/snap/rpc
$DEPS_OUTPUT_PATH/bin/protoc --cpp_out=. plugin.proto
quit_if_error $? "Snap C++ library (protoc)"
$DEPS_OUTPUT_PATH/bin/protoc --grpc_out=. --plugin=protoc-gen-grpc=$DEPS_OUTPUT_PATH/bin/grpc_cpp_plugin plugin.proto
quit_if_error $? "Snap C++ library (protoc gRPC)"
cd ../../..

./autogen.sh
quit_if_error $? "Snap C++ library (autogen.sh)"
CXXFLAGS="-I$DEPS_OUTPUT_PATH/include" ./configure --prefix=$DEPS_OUTPUT_PATH
quit_if_error $? "Snap C++ library (configure)"
make $MAKE_ARGS
quit_if_error $? "Snap C++ library (make)"
make install
quit_if_error $? "Snap C++ library (make install)"
cd ..


cd $DEPS_RETURN_PATH


###################################
### Build and install AnyCollect ###
###################################

echo
echo
echo -e "$B    Building AnyCollect$N"
echo

mkdir -p build
cd ./build

CXXFLAGS="-I$DEPS_OUTPUT_PATH/include" cmake -DCMAKE_INSTALL_PREFIX=$OUTPUT -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$DEPS_OUTPUT_PATH ../
quit_if_error $? "AnyCollect (cmake)"
make $MAKE_ARGS
quit_if_error $? "AnyCollect (make)"
make install
quit_if_error $? "AnyCollect (make install)"


cd ..


###############
### Cleanup ###
###############

if $(exit $CLEANUP); then
	echo
	echo
	echo -e "$B    Cleaning-up$N"
	echo

	cd $DEPS_PATH

	cd boost
	./b2 clean
	cd ..

	cd grpc
	make clean
	cd ..

	rm -rf cpp-netlib-build

	cd snap-plugin-lib-cpp
	make clean
	rm -f ./src/snap/rpc/plugin.grpc.pb.cc ./src/snap/rpc/plugin.grpc.pb.h ./src/snap/rpc/plugin.pb.cc ./src/snap/rpc/plugin.pb.h
	cd ..

	cd $DEPS_RETURN_PATH
	rm -rf $DEPS_OUTPUT_PATH
	rm -rf build
fi
