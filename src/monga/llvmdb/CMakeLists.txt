add_executable(monga_llvm monga_llvm.c)
target_link_libraries(monga_llvm monga_y)
add_tester(monga_llvm ${CMAKE_CURRENT_BINARY_DIR})