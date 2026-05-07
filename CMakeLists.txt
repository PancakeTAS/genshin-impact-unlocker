cmake_minimum_required(VERSION 3.19)

project(genshin-impact-unlocker
  DESCRIPTION "A cut-down version of genshin-fps-unlock"
  HOMEPAGE_URL "https://github.com/PancakeTAS/genshin-impact-unlocker"
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
add_link_options(-static-libgcc -static-libstdc++)
link_libraries(user32 dbghelp ntdll)

add_library(genshin-impact-unlocker SHARED src/dll.cpp)
add_executable(genshin-impact-launcher src/launcher.cpp)
