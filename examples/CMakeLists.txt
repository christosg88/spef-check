add_executable(00-hello_world 00-hello_world.cpp)
target_link_libraries(00-hello_world PRIVATE taocpp::pegtl)

add_executable(01-analyze_grammar 01-analyze_grammar.cpp)
target_link_libraries(01-analyze_grammar PRIVATE taocpp::pegtl)

add_executable(02-tracer 02-tracer.cpp)
target_link_libraries(02-tracer PRIVATE taocpp::pegtl)

add_executable(03-error_handling 03-error_handling.cpp)
target_link_libraries(03-error_handling PRIVATE taocpp::pegtl)
