add_library(MathFunctions mysqrt.cxx)

target_include_directories(MathFunctions
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(MathFunctions tutorial_compiler_flags)

set(installable_libs MathFunctions tutorial_compiler_flags)
install(TARGETS ${installable_libs} DESTINATION lib)

install(FILES MathFunctions.h DESTINATION include)
