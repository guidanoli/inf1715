add_library(monga_ast STATIC
    monga_ast_bind.c
    monga_ast_bind_stack.c
    monga_ast_destroy.c
    monga_ast_print.c)
target_link_libraries(monga_ast monga_utils)
target_include_directories(monga_ast PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})