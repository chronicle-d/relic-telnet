cmake_minimum_required(VERSION 3.15...3.29)
project(relic-telnet LANGUAGES CXX)

# General settings
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(relic-telnet src/rtelnet.cpp)

set_target_properties(relic-telnet PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    OUTPUT_NAME "relic-telnet-tester"
)

install(TARGETS relic-telnet DESTINATION bin)
