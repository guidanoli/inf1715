#ifndef MONGA_AST_REFERENCE_H
#define MONGA_AST_REFERENCE_H

#include "monga_ast.h"

const char* monga_ast_reference_kind_name(enum monga_ast_reference_tag_t tag);
size_t monga_ast_reference_line(struct monga_ast_reference_t* reference);
void monga_ast_reference_check_kind(struct monga_ast_reference_t* reference, enum monga_ast_reference_tag_t expected_tag, size_t line);

#endif