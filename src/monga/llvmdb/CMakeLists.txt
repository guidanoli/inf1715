add_executable(monga_llvmdb monga_llvm.c)
target_link_libraries(monga_llvmdb monga_y)
add_tester(monga_llvmdb ${CMAKE_CURRENT_BINARY_DIR})