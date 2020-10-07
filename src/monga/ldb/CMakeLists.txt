add_executable(monga_ldb monga_ldb.c)
target_link_libraries(monga_ldb monga_l)
add_tester(monga_ldb ${CMAKE_CURRENT_BINARY_DIR})