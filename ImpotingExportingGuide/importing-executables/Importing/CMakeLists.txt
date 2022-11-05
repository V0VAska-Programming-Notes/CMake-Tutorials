cmake_minimum_required(VERSION 3.15)

project(MyNewExe)

add_executable(myexe IMPORTED)

set_property(TARGET myexe
    PROPERTY
        IMPORTED_LOCATION "../../../../MyExe/out/install/${PRESET_NAME}/bin/myexe"
)

add_custom_command(OUTPUT main.cpp COMMAND myexe)

add_executable(mynewexe main.cpp)
