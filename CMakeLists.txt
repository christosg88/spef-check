cmake_minimum_required(VERSION 3.25)

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

# in case of compilation error in the pegtl templates, this will show the full
# stacktrace
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>)

set(CMAKE_C_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug build" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug build" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "Release build" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "Release build" FORCE)
set(CMAKE_C_FLAGS_ASAN "-fsanitize=address -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_C_FLAGS_MSAN "-fsanitize=memory -fno-sanitize-recover=all -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Memory Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fno-sanitize-recover=all -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Memory Sanitizer build" FORCE)
set(CMAKE_C_FLAGS_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all -g -O0" CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all -g -O0" CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_C_FLAGS_TSAN "-fsanitize=thread -fno-sanitize-recover=all -g -O1" CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -fno-sanitize-recover=all -g -O1" CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_C_FLAGS_PROFILE "-pg -g" CACHE STRING "gprof profiler" FORCE)
set(CMAKE_CXX_FLAGS_PROFILE "-pg -g" CACHE STRING "gprof profiler" FORCE)
set(CMAKE_C_FLAGS_HEAPPROFILE "-g -O0" CACHE STRING "Heap Profiler" FORCE)
set(CMAKE_CXX_FLAGS_HEAPPROFILE "-g -O0" CACHE STRING "Heap Profiler" FORCE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "MSAN" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message(FATAL_ERROR "Memory sanitizer is not currently supported by gcc. Try clang instead.")
endif()

set(ZLIB_USE_STATIC_LIBS ON)

include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        f5e54359df4c26b6230fc61d38aa294581393084  # 10.1.1
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  pegtl
  GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
  GIT_TAG        cf639f7f4ee125f68e1ccfba8d99ebc0de57b9fe  # 3.2.7
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        7e635fca68d014934b4af8a1cf874f63989352b7  # v1.12.0
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  thread-pool
  GIT_REPOSITORY https://github.com/bshoshany/thread-pool
  GIT_TAG        cabb3df5876c9a6824b07fcb0ff73d4a0e506ca0  # v3.5.0
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        09155eaa2f9270dc4ed1fa13e2b4b2613e6e4851  # 1.3
  OVERRIDE_FIND_PACKAGE
)
find_package(fmt REQUIRED)
find_package(pegtl REQUIRED)
find_package(spdlog REQUIRED)
find_package(thread-pool REQUIRED)
find_package(zlib REQUIRED)
find_package(Threads REQUIRED)

add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE ${FETCHCONTENT_BASE_DIR}/thread-pool-src/include)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(src)
