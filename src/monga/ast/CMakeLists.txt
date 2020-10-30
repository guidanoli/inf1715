add_library(monga_ast STATIC
    monga_ast.c
    monga_ast_builtin.c
    monga_ast_bind.c
    monga_ast_bind_stack.c
    monga_ast_destroy.c
    monga_ast_print.c
    monga_ast_reference.c
    monga_ast_typedesc.c)
target_link_libraries(monga_ast monga_utils)
target_include_directories(monga_ast PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(monga_ast PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)