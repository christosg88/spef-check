add_executable(spef_check spef_check.cpp spef_actions.cpp)
target_include_directories(spef_check SYSTEM PRIVATE ${PEGTL_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(spef_check PRIVATE taocpp::pegtl fmt::fmt ZLIB::ZLIB Threads::Threads)

if(CMAKE_BUILD_TYPE STREQUAL Profile)
  target_link_options(spef_check PRIVATE "-pg")
elseif(CMAKE_BUILD_TYPE STREQUAL HeapProfile)
  target_link_libraries(spef_check PRIVATE tcmalloc)
elseif(CMAKE_BUILD_TYPE STREQUAL MSAN)
  target_link_options(spef_check PRIVATE -fsanitize=memory)
endif()

install(TARGETS spef_check)

add_executable(test_reader test_reader.cpp)
target_include_directories(test_reader SYSTEM PRIVATE ${PEGTL_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(test_reader PRIVATE taocpp::pegtl fmt::fmt ZLIB::ZLIB thread-pool cpp_dbg_out Threads::Threads)

if(CMAKE_BUILD_TYPE STREQUAL Profile)
  target_link_options(test_reader PRIVATE "-pg")
elseif(CMAKE_BUILD_TYPE STREQUAL HeapProfile)
  target_link_libraries(test_reader PRIVATE tcmalloc)
elseif(CMAKE_BUILD_TYPE STREQUAL TSAN)
  target_link_options(test_reader PRIVATE -fsanitize=thread)
elseif(CMAKE_BUILD_TYPE STREQUAL MSAN)
  target_link_options(test_reader PRIVATE -fsanitize=memory)
endif()

install(TARGETS test_reader)
