cmake_minimum_required(VERSION 3.15)
project(Downstream)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(MathFunctions 3.4.1 EXACT
    COMPONENTS  Addition SquareRoot
    CONFIG
    PATHS "../MathFunctionsComponents/out/install/${PRESET_NAME}"
)

add_executable(myexe main.cxx)

target_link_libraries(myexe PRIVATE MathFunctions::Addition MathFunctions::SquareRoot)
