# Adding Components

## Upstream
Let's edit the MathFunctions project to use components. The source code for this section can be found in `./MathFunctionsComponents` directory. The CMakeLists file for this project adds two subdirectories: Addition and SquareRoot.

```cmake
cmake_minimum_required(VERSION 3.15)
project(MathFunctions)

# make cache variables for install destinations
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(Addition)
add_subdirectory(SquareRoot)
```

Generate and install the package configuration and package version files:

```cmake
# set version
set(version 3.4.1)

include(CMakePackageConfigHelpers)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

# create config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# install config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/MathFunctionsConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions"
)
```

If COMPONENTS are specified when the downstream uses find_package(), they are listed in the _FIND_COMPONENTS variable. We can use this variable to verify that all necessary component targets are included in Config.cmake.in. At the same time, this function will act as a custom check_required_components macro to ensure that the downstream only attempts to use supported components.

```
@PACKAGE_INIT@

set(_MathFunctions_supported_components Addition SquareRoot)

foreach(_comp ${MathFunctions_FIND_COMPONENTS})
    if (NOT _comp IN_LIST _MathFunctions_supported_components)
        set(MathFunctions_FOUND False)
        set(MathFunctions_NOT_FOUND_MESSAGE "Unsupported component: ${_comp}")
    endif()
    include("${CMAKE_CURRENT_LIST_DIR}/MathFunctions${_comp}Targets.cmake")
endforeach()
```

Here, the MathFunctions_NOT_FOUND_MESSAGE is set to a diagnosis that the package could not be found because an invalid component was specified. This message variable can be set for any case where the _FOUND variable is set to False, and will be displayed to the user.

The Addition and SquareRoot directories are similar. Let's look at one of the CMakeLists files:

```cmake
# create library
add_library(SquareRoot STATIC SquareRoot.cxx)

add_library(MathFunctions::SquareRoot ALIAS SquareRoot)

# add include directories
target_include_directories(SquareRoot
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# install the target and create export-set
install(TARGETS SquareRoot
    EXPORT SquareRootTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# install header file
install(FILES SquareRoot.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# generate and install export file
install(EXPORT SquareRootTargets
    FILE MathFunctionsSquareRootTargets.cmake
    NAMESPACE MathFunctions::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MathFunctions"
)
```

Now we can build the project as described in earlier sections.

## Downstream
To test using this package, we can use the project in `./DownstreamComponents`. There's two differences from the previous Downstream project. First, we need to find the package components. Change the find_package line from `find_package(MathFunctions 3.4.1 ...)` to:

```cmake
find_package(MathFunctions 3.4.1 EXACT
    COMPONENTS  Addition SquareRoot
    CONFIG
    PATHS "../MathFunctionsComponents/out/install/${PRESET_NAME}"
)
```

and the target_link_libraries line from:

```cmake
target_link_libraries(myexe PRIVATE MathFunctions::MathFunctions)
```

To:

```cmake
target_link_libraries(myexe PRIVATE MathFunctions::Addition MathFunctions::SquareRoot)
```

In main.cxx, replace `#include MathFunctions.h` with:

```c++
#include "Addition.h"
#include "SquareRoot.h"
```

Finally, use the Addition library too:

```c++
    const double sum = MathFunctions::add(inputValue, inputValue);
    std::cout << inputValue << " + " << inputValue << " = " << sum << std::endl;
```

Build the Downstream project and confirm that it can find and use the package components.
