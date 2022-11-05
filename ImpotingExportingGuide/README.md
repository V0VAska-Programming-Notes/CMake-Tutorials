# Importing-Exporting Guide
Сам гайд расположен по этой [ссылке](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html), но здесь он разбит на несколько частей просто для удобства.

## Introduction
In this guide, we will present the concept of IMPORTED targets and demonstrate how to import existing executable or library files from disk into a CMake project. We will then show how CMake supports exporting targets from one CMake-based project and importing them into another. Finally, we will demonstrate how to package a project with a configuration file to allow for easy integration into other CMake projects. This guide and the complete example source code can be found in the Help/guide/importing-exporting directory of the CMake [source code tree](https://gitlab.kitware.com/cmake/cmake).

## Importing Targets
IMPORTED targets are used to convert files outside of a CMake project into logical targets inside of the project. IMPORTED targets are created using the IMPORTED option of the add_executable() and add_library() commands. No build files are generated for IMPORTED targets. Once imported, IMPORTED targets may be referenced like any other target within the project and provide a convenient, flexible reference to outside executables and libraries.

By default, the IMPORTED target name has scope in the directory in which it is created and below. We can use the GLOBAL option to extended visibility so that the target is accessible globally in the build system.

Details about the IMPORTED target are specified by setting properties whose names begin in IMPORTED_ and INTERFACE_. For example, IMPORTED_LOCATION contains the full path to the target on disk.

## Importing Executables
[тыц](importing-executables)

## Importing Libraries
In a similar manner, libraries from other projects may be accessed through IMPORTED targets.

> Note: The full source code for the examples in this section is not provided and is left as an exercise for the reader.

In the CMakeLists file, add an IMPORTED library and specify its location on disk:

```cmake
add_library(foo STATIC IMPORTED)
set_property(TARGET foo PROPERTY
             IMPORTED_LOCATION "/path/to/libfoo.a")
```

Then use the IMPORTED library inside of our project:

```cmake
add_executable(myexe src1.c src2.c) target_link_libraries(myexe PRIVATE foo)
```

On Windows, a .dll and its .lib import library may be imported together:

```cmake
add_library(bar SHARED IMPORTED)
set_property(TARGET bar PROPERTY IMPORTED_LOCATION "c:/path/to/bar.dll")
set_property(TARGET bar PROPERTY IMPORTED_IMPLIB "c:/path/to/bar.lib")
add_executable(myexe src1.c src2.c)
target_link_libraries(myexe PRIVATE bar)
```

A library with multiple configurations may be imported with a single target:

```cmake
find_library(math_REL NAMES m)
find_library(math_DBG NAMES md)
add_library(math STATIC IMPORTED GLOBAL)
set_target_properties(math PROPERTIES
	IMPORTED_LOCATION "${math_REL}"
	IMPORTED_LOCATION_DEBUG "${math_DBG}"
	IMPORTED_CONFIGURATIONS "RELEASE;DEBUG" )
add_executable(myexe src1.c src2.c)
target_link_libraries(myexe PRIVATE math)
```

The generated build system will link myexe to m.lib when built in the release configuration, and md.lib when built in the debug configuration.

## Exporting - Exporting
Следующие две основные части объединены в одной [директории](import-export), поскольку взаимосвязаны.
> В оригинальном туторе это главы, начиная с [Exporting Targets](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#id6) по [Using the Package Configuration File](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#id13) включительно.

## Adding Components
Данная часть тутора является продолжением 'Exporting - Exporting' и расположена [здесь](adding-components)
