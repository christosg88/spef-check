cmake_minimum_required(VERSION 3.16)

project(spef-check
  VERSION 0.0.1
  DESCRIPTION "A tool to parse and check the correctness of a SPEF file"
  LANGUAGES CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# generate the compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable all warnings and treat them as errors
add_compile_options(-Wall -Wextra -pedantic)
#add_compile_options(-Werror)
# enable clang-tidy checks
#set(CMAKE_CXX_CLANG_TIDY clang-tidy)

# produce colored diagnostics when using Ninja
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

set(allowedBuildTypes Debug Release ASAN MSAN UBSAN TSAN Profile HeapProfile)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

add_compile_options(-ftemplate-backtrace-limit=0)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug build" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "Release build" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O1" CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O1" CACHE STRING "Memory Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -g -O0" CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_PROFILE "-pg -g" CACHE STRING "gprof profiler" FORCE)
set(CMAKE_CXX_FLAGS_HEAPPROFILE "-g -O0" CACHE STRING "Heap Profiler" FORCE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "MSAN" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message(FATAL_ERROR "Memory sanitizer is not currently supported by gcc. Try clang instead.")
endif()

set(CMAKE_PREFIX_PATH
  ${CMAKE_SOURCE_DIR}/third-party/pegtl-3.2.7
  ${CMAKE_SOURCE_DIR}/third-party/fmt-10.0.0
  ${CMAKE_SOURCE_DIR}/third-party/zlib-1.2.13)
find_package(PEGTL REQUIRED)
find_package(FMT REQUIRED)
set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE ${CMAKE_SOURCE_DIR}/third-party/thread-pool-3.5.0/include)
add_library(cpp_dbg_out INTERFACE)
target_include_directories(cpp_dbg_out INTERFACE ${CMAKE_SOURCE_DIR}/third-party/cpp_dbg_out)

message(STATUS "Linking PEGTL version ${PEGTL_VERSION}")
message(STATUS "Linking FMT version ${FMT_VERSION}")
message(STATUS "Linking ZLIB version ${ZLIB_VERSION}")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(src)
