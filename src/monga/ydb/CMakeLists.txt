add_executable(monga_ydb monga_ydb.c)
target_link_libraries(monga_ydb monga_y)
add_tester(monga_ydb ${CMAKE_CURRENT_BINARY_DIR})