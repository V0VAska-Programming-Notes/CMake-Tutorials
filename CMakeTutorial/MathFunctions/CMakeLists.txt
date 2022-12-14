add_library(MathFunctions
    MathFunctions.cxx
)

include(GenerateExportHeader)
generate_export_header(MathFunctions)

target_include_directories(MathFunctions
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        $<INSTALL_INTERFACE:include>
)

option(USE_MYMATH "Use tutorial provided math implementation" ON)
if(USE_MYMATH)
    target_compile_definitions(MathFunctions
        PRIVATE
            "USE_MYMATH"
    )

    add_executable(MakeTable MakeTable.cxx)
    target_link_libraries(MakeTable
        PRIVATE
            tutorial_compiler_flags
    )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Table.h"
        COMMAND MakeTable "${CMAKE_CURRENT_BINARY_DIR}/Table.h"
        DEPENDS MakeTable
    )

    add_library(mysqrt STATIC
        mysqrt.cxx
        "${CMAKE_CURRENT_BINARY_DIR}/Table.h"
    )

    target_include_directories(mysqrt
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}"
    )

    set_target_properties(mysqrt
        PROPERTIES
            POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
    )

    target_link_libraries(mysqrt
        PRIVATE
            tutorial_compiler_flags
    )

    target_link_libraries(MathFunctions
        PRIVATE
            mysqrt
    )

endif(USE_MYMATH)

target_link_libraries(MathFunctions
    PRIVATE
        tutorial_compiler_flags
)

set_property(TARGET MathFunctions PROPERTY VERSION "1.0.0")
set_property(TARGET MathFunctions PROPERTY SOVERSION "1")

set(installable_libs MathFunctions tutorial_compiler_flags)
if(TARGET mysqrt)
    list(APPEND installable_libs mysqrt)
endif()

install(TARGETS
    ${installable_libs}
    EXPORT MathFunctionsTargets
    DESTINATION lib)

install(FILES
        MathFunctions.h
        ${CMAKE_CURRENT_BINARY_DIR}/mathfunctions_export.h
    DESTINATION include
)
