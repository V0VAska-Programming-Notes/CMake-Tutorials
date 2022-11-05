# Exporting Targets
While IMPORTED targets on their own are useful, they still require that the project that imports them knows the locations of the target files on disk. The real power of IMPORTED targets is when the project providing the target files also provides a CMake file to help import them. A project can be setup to produce the necessary information so that it can easily be used by other CMake projects be it from a build directory, a local install or when packaged.

In the remaining sections, we will walk through a set of example projects step-by-step. The first project will create and install a library and corresponding CMake configuration and package files. The second project will use the generated package.

Let's start by looking at the MathFunctions project in the `./MathFunctions` directory. Here we have a header file MathFunctions.h that declares a sqrt function:

```c++
#pragma once

namespace MathFunctions {
double sqrt(double x);
}
```

And a corresponding source file MathFunctions.cxx:

```c++
#include <cmath>

#include "MathFunctions.h"

namespace MathFunctions {
double sqrt(double x)
{
    return std::sqrt(x);
}
} // namespace MathFunctions
```

Don't worry too much about the specifics of the C++ files, they are just meant to be a simple example that will compile and run on many build systems.

Now we can create a CMakeLists.txt file for the MathFunctions project. Start by specifying the cmake_minimum_required() version and project() name:

```cmake
cmake_minimum_required(VERSION 3.15)
project(MathFunctions)

# make cache variables for install destinations
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
```

The GNUInstallDirs module is included in order to provide the project with the flexibility to install into different platform layouts by making the directories available as cache variables.

Create a library called MathFunctions with the add_library() command:

```cmake
add_library(MathFunctions STATIC MathFunctions.cxx)
```

And then use the target_include_directories() command to specify the include directories for the target:

```cmake
target_include_directories(MathFunctions
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
```

We need to tell CMake that we want to use different include directories depending on if we're building the library or using it from an installed location. If we don't do this, when CMake creates the export information it will export a path that is specific to the current build directory and will not be valid for other projects. We can use generator expressions to specify that if we're building the library include the current source directory. Otherwise, when installed, include the include directory. See the Creating Relocatable Packages section for more details.

The install(TARGETS) and install(EXPORT) commands work together to install both targets (a library in our case) and a CMake file designed to make it easy to import the targets into another CMake project.

First, in the install(TARGETS) command we will specify the target, the EXPORT name and the destinations that tell CMake where to install the targets.

```cmake
install(TARGETS MathFunctions
        EXPORT MathFunctionsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

Here, the EXPORT option tells CMake to create an export called MathFunctionsTargets. The generated IMPORTED targets have appropriate properties set to define their usage requirements, such as INTERFACE_INCLUDE_DIRECTORIES, INTERFACE_COMPILE_DEFINITIONS and other relevant built-in INTERFACE_ properties. The INTERFACE variant of user-defined properties listed in COMPATIBLE_INTERFACE_STRING and other Compatible Interface Properties are also propagated to the generated IMPORTED targets. For example, in this case, the IMPORTED target will have its INTERFACE_INCLUDE_DIRECTORIES property populated with the directory specified by the INCLUDES DESTINATION property. As a relative path was given, it is treated as relative to the CMAKE_INSTALL_PREFIX.

Note, we have not asked CMake to install the export yet.

We don't want to forget to install the MathFunctions.h header file with the install(FILES) command. The header file should be installed to the include directory, as specified by the target_include_directories() command above.

```cmake
install(FILES MathFunctions.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
```

Now that the MathFunctions library and header file are installed, we also need to explicitly install the MathFunctionsTargets export details. Use the install(EXPORT) command to export the targets in MathFunctionsTargets, as defined by the install(TARGETS) command.

```cmake
install(EXPORT MathFunctionsTargets
        FILE MathFunctionsTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
```

This command generates the MathFunctionsTargets.cmake file and arranges to install it to lib/cmake. The file contains code suitable for use by downstreams to import all targets listed in the install command from the installation tree.

The NAMESPACE option will prepend MathFunctions:: to the target names as they are written to the export file. This convention of double-colons gives CMake a hint that the name is an IMPORTED target when it is used by downstream projects. This way, CMake can issue a diagnostic message if the package providing it was not found.

The generated export file contains code that creates an IMPORTED library.

```cmake
# Create imported target MathFunctions::MathFunctions
add_library(MathFunctions::MathFunctions STATIC IMPORTED)

set_target_properties(MathFunctions::MathFunctions PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
)
```

This code is very similar to the example we created by hand in the Importing Libraries section. Note that ${_IMPORT_PREFIX} is computed relative to the file location.

An outside project may load this file with the include() command and reference the MathFunctions library from the installation tree as if it were built in its own tree. For example:

```
1. include(${INSTALL_PREFIX}/lib/cmake/MathFunctionTargets.cmake)
2. add_executable(myexe src1.c src2.c )
3. target_link_libraries(myexe PRIVATE MathFunctions::MathFunctions)
```

Line 1 loads the target CMake file. Although we only exported a single target, this file may import any number of targets. Their locations are computed relative to the file location so that the install tree may be easily moved. Line 3 references the imported MathFunctions library. The resulting build system will link to the library from its installed location.

Executables may also be exported and imported using the same process.

Any number of target installations may be associated with the same export name. Export names are considered global so any directory may contribute a target installation. The install(EXPORT) command only needs to be called once to install a file that references all targets. Below is an example of how multiple exports may be combined into a single export file, even if they are in different subdirectories of the project.

```cmake
# A/CMakeLists.txt
add_executable(myexe src1.c)
install(TARGETS myexe DESTINATION lib/myproj
        EXPORT myproj-targets)

# B/CMakeLists.txt
add_library(foo STATIC foo1.c)
install(TARGETS foo DESTINATION lib EXPORTS myproj-targets)

# Top CMakeLists.txt
add_subdirectory (A)
add_subdirectory (B)
install(EXPORT myproj-targets DESTINATION lib/myproj)
```

<details><summary>Complete lising of CMakeLists.txt</summary>

```cmake
cmake_minimum_required(VERSION 3.15)
project(MathFunctions)

# make cache variables for install destinations
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(MathFunctions STATIC MathFunctions.cxx)

target_include_directories(MathFunctions
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS MathFunctions
        EXPORT MathFunctionsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES MathFunctions.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT MathFunctionsTargets
        FILE MathFunctionsTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
```

</details>

## Creating Packages

At this point, the MathFunctions project is exporting the target information required to be used by other projects. We can make this project even easier for other projects to use by generating a configuration file so that the CMake find_package() command can find our project.

To start, we will need to make a few additions to the CMakeLists.txt file. First, include the CMakePackageConfigHelpers module to get access to some helper functions for creating config files.

```cmake
include(CMakePackageConfigHelpers)
```

Then we will create a package configuration file and a package version file.

### Creating a Package Configuration File
Use the configure_package_config_file() command provided by the CMakePackageConfigHelpers to generate the package configuration file. Note that this command should be used instead of the plain configure_file() command. It helps to ensure that the resulting package is relocatable by avoiding hardcoded paths in the installed configuration file. The path given to INSTALL_DESTINATION must be the destination where the MathFunctionsConfig.cmake file will be installed. We will examine the contents of the package configuration file in the next section.

```cmake
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
```

Install the generated configuration files with the INSTALL(files) command. Both MathFunctionsConfigVersion.cmake and MathFunctionsConfig.cmake are installed to the same location, completing the package.

```cmake
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)
```

Now we need to create the package configuration file itself. In this case, the `Config.cmake.in` file is very simple but sufficient to allow downstreams to use the IMPORTED targets.

```
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/MathFunctionsTargets.cmake")

check_required_components(MathFunctions)
```

The first line of the file contains only the string @PACKAGE_INIT@. This expands when the file is configured and allows the use of relocatable paths prefixed with PACKAGE_. It also provides the set_and_check() and check_required_components() macros.

The check_required_components helper macro ensures that all requested, non-optional components have been found by checking the __FOUND variables for all required components. This macro should be called at the end of the package configuration file even if the package does not have any components. This way, CMake can make sure that the downstream project hasn't specified any non-existent components. If check_required_components fails, the _FOUND variable is set to FALSE, and the package is considered to be not found.

The set_and_check() macro should be used in configuration files instead of the normal set() command for setting directories and file locations. If a referenced file or directory does not exist, the macro will fail.

If any macros should be provided by the MathFunctions package, they should be in a separate file which is installed to the same location as the MathFunctionsConfig.cmake file, and included from there.

**All required dependencies of a package must also be found in the package configuration file**. Let's imagine that we require the Stats library in our project. In the CMakeLists file, we would add:

```cmake
find_package(Stats 2.6.4 REQUIRED)
target_link_libraries(MathFunctions PUBLIC Stats::Types)
```

As the Stats::Types target is a PUBLIC dependency of MathFunctions, downstreams must also find the Stats package and link to the Stats::Types library. The Stats package should be found in the configuration file to ensure this.

```cmake
include(CMakeFindDependencyMacro)
find_dependency(Stats 2.6.4)
```

The find_dependency macro from the CMakeFindDependencyMacro module helps by propagating whether the package is REQUIRED, or QUIET, etc. The find_dependency macro also sets MathFunctions_FOUND to False if the dependency is not found, along with a diagnostic that the MathFunctions package cannot be used without the Stats package.

> Exercise: Add a required library to the MathFunctions project.

### Creating a Package Version File
The CMakePackageConfigHelpers module provides the write_basic_package_version_file() command for creating a simple package version file. This file is read by CMake when find_package() is called to determine the compatibility with the requested version, and to set some version-specific variables such as _VERSION, _VERSION_MAJOR, _VERSION_MINOR, etc. See cmake-packages documentation for more details.

```cmake
set(version 3.4.1)

set_property(TARGET MathFunctions PROPERTY VERSION ${version})
set_property(TARGET MathFunctions PROPERTY SOVERSION 3)
set_property(TARGET MathFunctions PROPERTY
    INTERFACE_MathFunctions_MAJOR_VERSION 3)
set_property(TARGET MathFunctions APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING MathFunctions_MAJOR_VERSION
)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)
```

In our example, MathFunctions_MAJOR_VERSION is defined as a COMPATIBLE_INTERFACE_STRING which means that it must be compatible among the dependencies of any depender. By setting this custom defined user property in this version and in the next version of MathFunctions, cmake will issue a diagnostic if there is an attempt to use version 3 together with version 4. Packages can choose to employ such a pattern if different major versions of the package are designed to be incompatible.

### Exporting Targets from the Build Tree
Typically, projects are built and installed before being used by an outside project. However, in some cases, it is desirable to export targets directly from a build tree. The targets may then be used by an outside project that references the build tree with no installation involved. The export() command is used to generate a file exporting targets from a project build tree.

If we want our example project to also be used from a build directory we only have to add the following to CMakeLists.txt:

```cmake
export(EXPORT MathFunctionsTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/MathFunctionsTargets.cmake"
    NAMESPACE MathFunctions::
)
```

Here we use the export() command to generate the export targets for the build tree. In this case, we'll create a file called MathFunctionsTargets.cmake in the cmake subdirectory of the build directory. The generated file contains the required code to import the target and may be loaded by an outside project that is aware of the project build tree. This file is specific to the build-tree, and is not relocatable.

An example application of this feature is for building an executable on a host platform when cross-compiling. The project containing the executable may be built on the host platform and then the project that is being cross-compiled for another platform may load it.

### Building and Installing a Package
At this point, we have generated a relocatable CMake configuration for our project that can be used after the project has been installed.

<details><summary>Complete lising of CMakeLists.txt</summary>

```cmake
cmake_minimum_required(VERSION 3.15)
project(MathFunctions)

# make cache variables for install destinations
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(MathFunctions STATIC MathFunctions.cxx)

target_include_directories(MathFunctions
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS MathFunctions
        EXPORT MathFunctionsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES MathFunctions.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT MathFunctionsTargets
        FILE MathFunctionsTargets.cmake
        NAMESPACE MathFunctions::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions
)

set(version 3.4.1)

set_property(TARGET MathFunctions PROPERTY VERSION ${version})
set_property(TARGET MathFunctions PROPERTY SOVERSION 3)
set_property(TARGET MathFunctions PROPERTY
    INTERFACE_MathFunctions_MAJOR_VERSION 3)
set_property(TARGET MathFunctions APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING MathFunctions_MAJOR_VERSION
)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT MathFunctionsTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/MathFunctionsTargets.cmake"
    NAMESPACE MathFunctions::
)
```

</details>

Let's try to build the MathFunctions project:

```
mkdir MathFunctions_build
cd MathFunctions_build
cmake ../MathFunctions
cmake --build .
```

In the build directory, notice that the file MathFunctionsTargets.cmake has been created in the cmake subdirectory.

Now install the project:

```
cmake --install . --prefix "/home/myuser/installdir"
```

> Чтобы не путаться в путях и задаваться вопросами, почему что-то не работает, лучше на этапе обучения пользоваться приложенным к проектам CMakePresets.json и возможностями IDE.

## Creating Relocatable Packages
Packages created by install(EXPORT) are designed to be relocatable, using paths relative to the location of the package itself. They must not reference absolute paths of files on the machine where the package is built that will not exist on the machines where the package may be installed.

When defining the interface of a target for EXPORT, keep in mind that the include directories should be specified as relative paths to the CMAKE_INSTALL_PREFIX but should not explicitly include the CMAKE_INSTALL_PREFIX:

```cmake
target_include_directories(tgt INTERFACE
  # Wrong, not relocatable:
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/TgtName>
)

target_include_directories(tgt INTERFACE
  # Ok, relocatable:
  $<INSTALL_INTERFACE:include/TgtName>
)
```

The $<INSTALL_PREFIX> generator expression may be used as a placeholder for the install prefix without resulting in a non-relocatable package. This is necessary if complex generator expressions are used:

```cmake
target_include_directories(tgt INTERFACE
  # Ok, relocatable:
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/TgtName>
)
```

This also applies to paths referencing external dependencies. It is not advisable to populate any properties which may contain paths, such as INTERFACE_INCLUDE_DIRECTORIES or INTERFACE_LINK_LIBRARIES, with paths relevant to dependencies. For example, this code may not work well for a relocatable package:

```cmake
target_link_libraries(MathFunctions INTERFACE
  ${Foo_LIBRARIES} ${Bar_LIBRARIES}
  )
target_include_directories(MathFunctions INTERFACE
  "$<INSTALL_INTERFACE:${Foo_INCLUDE_DIRS};${Bar_INCLUDE_DIRS}>"
  )
```

The referenced variables may contain the absolute paths to libraries and include directories **as found on the machine the package was made on**. This would create a package with hard-coded paths to dependencies not suitable for relocation.

Ideally such dependencies should be used through their own IMPORTED targets that have their own IMPORTED_LOCATION and usage requirement properties such as INTERFACE_INCLUDE_DIRECTORIES populated appropriately. Those imported targets may then be used with the target_link_libraries() command for MathFunctions:

```cmake
target_link_libraries(MathFunctions INTERFACE Foo::Foo Bar::Bar)
```

With this approach the package references its external dependencies only through the names of IMPORTED targets. When a consumer uses the installed package, the consumer will run the appropriate find_package() commands (via the find_dependency macro described above) to find the dependencies and populate the imported targets with appropriate paths on their own machine.

