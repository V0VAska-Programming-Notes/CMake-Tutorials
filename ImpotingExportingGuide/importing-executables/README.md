# Importing Executables
To start, we will walk through a simple example that creates an IMPORTED executable target and then references it from the add_custom_command() command.

We'll need to do some setup to get started. We want to create an executable that when run creates a basic main.cpp file in the current directory. The details of this project are not important. Navigate to MyExe, create a build directory, run cmake and build and install the project.

> Ессно, ежели пользоваться приложенными к исходникам пресетам для CMake, пути будут 'out/build/имя-пресета'. Также install директория там прописана в 'out/install/...'. Как выполнять работу с CMake не врукопашную, а с помощью IDE, так туторов достаточно. Пресеты фурыкают и под VS 2022, так и под VS Code (в линухе не забыть установить ninja).
 
```
cd MyExe
mkdir build
cd build
cmake ..
cmake --build .
cmake --install . --prefix <install location>
<install location>/myexe
ls
[...] main.cpp [...]
```

Now we can import this executable into another CMake project. The source code for this section is available in `Importing` directory. In the CMakeLists file, use the add_executable() command to create a new target called myexe. Use the IMPORTED option to tell CMake that this target references an executable file located outside of the project. No rules will be generated to build it and the IMPORTED target property will be set to true.

```cmake
cmake_minimum_required(VERSION 3.15)

project(MyNewExe)

add_executable(myexe IMPORTED)
```

Next, set the IMPORTED_LOCATION property of the target using the set_property() command. This will tell CMake the location of the target on disk. The location may need to be adjusted to the <install location> specified in the previous step.
> Подразумевается, что для построения MyExe и MyNewExe использовался один и тот же пресет. Не следует путать, что IMPORTED_LOCATION - это не указание каталога, а путь к файлу. Пусть не смущает, ежели в винде нет указания, например, на расширение .exe. Аналогично **вроде** обстоит дело и с библиотеками, касательно линуха тоже, как относительно суффикса-расшинения, так и префикса.

```cmake
set_property(TARGET myexe
    PROPERTY
        IMPORTED_LOCATION "../../../../MyExe/out/install/${PRESET_NAME}/bin/myexe"
)
```

We can now reference this IMPORTED target just like any target built within the project. In this instance, let's imagine that we want to use the generated source file in our project. Use the IMPORTED target in the add_custom_command() command:

```cmake
add_custom_command(OUTPUT main.cpp COMMAND myexe)
```

As COMMAND specifies an executable target name, it will automatically be replaced by the location of the executable given by the IMPORTED_LOCATION property above.
> Не следует забывать о рабочем каталоге, и где сформируется вывод программы myexe.

Finally, use the output from add_custom_command():

```cmake
add_executable(mynewexe main.cpp)
```
