cmake_minimum_required (VERSION 2.6)
project (CppAuParser)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")
endif ()

if (MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif ()

add_subdirectory(src/ lib/)
add_subdirectory(sample/)
add_subdirectory(tool/)
