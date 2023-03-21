cmake_minimum_required(VERSION 3.16)

# set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# enable all warnings and treat them as errors
add_compile_options(-Wall -Wextra -pedantic -Werror)
# enable clang-tidy checks
#set(CMAKE_CXX_CLANG_TIDY clang-tidy)

# produce colored diagnostics when using Ninja
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostic-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

set(allowedBuildTypes Debug Release ASAN UBSAN TSAN Profile)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

add_compile_options(-ftemplate-backtrace-limit=0)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug build" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "Release build" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -g -O0" CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -g -O0" CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_PROFILE "-pg -g" CACHE STRING "gprof profiler" FORCE)

project(spef-check
  VERSION 0.0.1
  DESCRIPTION "A tool to parse and check the correctness of a SPEF file"
  LANGUAGES CXX)

find_package(pegtl)

add_executable(spef_check spef_check.cpp)
target_link_libraries(spef_check PRIVATE taocpp::pegtl)
if(CMAKE_BUILD_TYPE STREQUAL Profile)
  target_link_options(spef_check PRIVATE "-pg")
endif()
