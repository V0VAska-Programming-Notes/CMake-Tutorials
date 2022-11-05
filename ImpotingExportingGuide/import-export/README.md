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

