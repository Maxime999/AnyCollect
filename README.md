<!--
Copyright 2018 CFM (www.cfm.fr)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# AnyCollect
This project aims to collect metrics by reading files or executing commands, and using regular expressions. It is designed to work with [Snap Telemetry](http://snap-telemetry.io).

It is composed of multiple packages:
- AnyCollect, which does most of the work: collecting, storing, processing, computing statistics, etc., as well as getting information about which metrics are collected
- AnyCollectValues which provides a standalone interface for the AnyCollect package
- AnyCollectSnap, the Snap plugin interface for the AnyCollect package

The Snap plugins require a special version of the [C++ Snap plugin library](https://github.com/Maxime999/snap-plugin-lib-cpp).


## Dependencies
All project packages except Snap plugins have no dependencies and can be built directly. A C++17 compatible compiler (such as GCC 7) is required.

Before compiling Snap plugins, the [C++ Snap plugin library](https://github.com/Maxime999/snap-plugin-lib-cpp) and its dependencies should be installed on the system. AnyCollect uses static liniking of those libraries for the executables to be portable.


## Compiling
If the Snap and its dependencies are built in the `/usr/local/lib`, execute `ldconfig /usr/local/lib/` to allow ld to look for installed libraries there.

Then you can compile with CMake:

``` bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
make install
```

Other build configured types are `Debug`, `Release`, and `RelWithDebInfo` (to profile with [Linux Perf](https://perf.wiki.kernel.org/index.php/Main_Page) or [GPerfTools](https://gperftools.github.io/gperftools/)).

### `buildall` script
The `buildall.sh` script can be used to build AnyCollect Snap Plugin library and its dependencies automatically. It can either download the dependencies from GitHub or use local ones in the `third_party` folder.


## Documentation
The documentation can be built with Doxygen from the `build` directory. It is installed in `doc/html`.

``` bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make doc
make install
```
