# Example Code: Cloud Discovery Service Library API in C

## Building the Example :wrench:

To build this example, first run CMake to generate the corresponding build
files. We recommend you use a separate directory to store all the generated
files (e.g., ./build).

```sh
mkdir build
cd build
cmake ..
```

Once you have run CMake, you will find a number of new files in your build
directory (the list of generated files will depend on the specific CMake
Generator). To build the example, run CMake as follows:

```sh
cmake --build .
```

**Note**: If you are using a multi-configuration generator, such as Visual
Studio solutions, you can specify the configuration mode to build as follows:

```sh
cmake --build . --config Release|Debug
```

Alternatively, you can use directly the generated infrastructure (e.g.,
Makefiles or Visual Studio Solutions) to build the example. If you generated
Makefiles in the configuration process, run make to build the example. Likewise,
if you generated a Visual Studio solution, open the solution and follow the
regular build process.

## Running the Example

Once the example is built it will generate the `CloudDiscoveryServiceLibraryAPIC`
executable. This executable takes two arguments that are similar to the Cloud
Discovery Service executable shipped with the Connext installation:

- `-cfgFile`: Specifies the QoS configuration file name
- `-cfgName`: Specified the top level configuration name

You can run the generated executable as follows:

```sh
./CloudDiscoveryServiceLibraryAPIC -cfgFile CloudDiscoveryServiceConfig.xml
```

Additionally the `CloudDiscoveryServiceLibraryAPIC` executable also takes a third
argument called `-runForSecs` that controls how long the application executes.

**Note:** The file `CloudDiscoveryServiceConfig.xml` has only one configuration called
`LibraryAPIDemo` that starts the Cloud Discovery Service instance on port `7400`
while utilizing the `UDPv4` transport. These values can be changed by modifying
the configuration file.

Now you can point your publisher or subscriber applications to this Cloud Discovery
Service instance to achieve discovery between them. For more details on how to modify
this setup for your specific transport requirements refer to the [Tutorials
Section](https://community.rti.com/static/documentation/connext-dds/7.0.0/doc/manuals/addon_products/cloud_discovery_service/tutorials.html)
in the Cloud Discovery Service User's Manual.

## Customizing the Build

### Configuring Build Type and Generator

By default, CMake will generate build files using the most common generator for
your host platform (e.g., Makefiles on Unix-like systems and Visual Studio
Solutions on Windows). You can use the following CMake variables to modify the
default behavior:

-   `-DCMAKE_BUILD_TYPE` - specifies the build mode. Valid values are `Release`
    and `Debug`. See the [CMake documentation for more details
    (Optional)](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).

-   `-DBUILD_SHARED_LIBS` - specifies the link mode. Valid values are `ON` for
    dynamic linking and `OFF` for static linking. See [CMake documentation for
    more details
    (Optional)](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html).

-   `-G` - CMake generator. The generator is the native build system used to
    build the source code. All the valid values are described in the CMake
    documentation for [CMake
    Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

For example, to build an example in Debug/Dynamic mode run CMake as follows:

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON .. -G "Visual Studio 15 2017" -A x64
```

### Configuring Connext Installation Path and Architecture

The CMake build infrastructure will try to guess the location of your Connext
installation and the Connext architecture based on the default settings
for your host platform. If you installed Connext in a custom location, you
can use the `CONNEXTDDS_DIR` variable to indicate the path to your RTI Connext
installation folder. For example:

```sh
cmake -DCONNEXTDDS_DIR=/home/rti/rti_connext_dds-x.y.z ..
```

Also, if you installed libraries for multiple target architectures on your system
(i.e., you installed more than one target `.rtipkg` file), you can use the
`CONNEXTDDS_ARCH` variable to indicate the architecture of the specific libraries
you want to link against. For example:

```sh
cmake -DCONNEXTDDS_ARCH=x64Linux3gcc5.4.0 ..
```

### Cross-compilation

When you need to cross-compile the example, the above
command will not work, the assigned compiler won't be the cross-compiler and
errors may happen when linking against the cross-compiled Connext binaries.
To fix this, you have to create a file with the architecture name and call
CMake with a specific flag called ``-DCMAKE_TOOLCHAIN_FILE``.
An example of the file to create with the toolchain settings (e.g. for an
ARM architectures):

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(toolchain_path "<path to>/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian")
set(CMAKE_C_COMPILER "${toolchain_path}/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${toolchain_path}/bin/arm-linux-gnueabihf-g++")
```

Then you can call CMake like this:

```bash
    cmake -DCONNEXTDDS_DIR=<connext dir> -DCMAKE_TOOLCHAIN_FILE=<toolchain file created above>
            -DCONNEXTDDS_ARCH=<connext architecture> ..
```

### CMake Build Infrastructure

The `CMakeLists.txt` script that builds this example uses a generic CMake
function called `connextdds_add_example` that defines all the necessary constructs
to:

1.  Run RTI Code Generator to generate the serialization/deserialization code
    for the types defined in the IDL file associated with the example.

2.  Build the corresponding Publisher and Subscriber applications.

3.  Copy the `USER_QOS_PROFILES.xml` file into the directory where the publisher
    and subscriber executables are generated.

You will find the definition of `connextdds_add_example`, along with detailed
documentation, in
[resources/cmake/rticonnextdds-cmake-utils/cmake/Modules/ConnextDdsAddExample.cmake
](https://github.com/rticommunity/rticonnextdds-cmake-utils/blob/main/cmake/Modules/ConnextDdsAddExample.cmake).

For a more comprehensive example on how to build an RTI Connext application
using CMake, please refer to the
[hello_world](../../../connext_dds/build_systems/cmake/) example, which includes
a comprehensive `CMakeLists.txt` script with all the steps and instructions
described in detail.
