# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.28)

# Nombre del proyecto y lenguaje
project(Rasterizer CXX)

# Estándar de C++ a usar
set(CMAKE_CXX_STANDARD 17)

# Archivos fuente — todos los .cpp que quieres compilar
add_executable(Rasterizer
        main.cpp
)

# Decirle a CMake dónde están los headers de SDL3
target_include_directories(Rasterizer PRIVATE
        C:/SDL3/include
)

# Decirle a CMake dónde están los archivos .lib de SDL3
target_link_directories(Rasterizer PRIVATE
        C:/SDL3/lib
)

# Linkear las librerías necesarias
target_link_libraries(Rasterizer PRIVATE
        SDL3
)