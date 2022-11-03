add_library(MathFunctions mysqrt.cxx)

target_include_directories(MathFunctions
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(MathFunctions tutorial_compiler_flags)
