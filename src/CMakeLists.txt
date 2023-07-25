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
