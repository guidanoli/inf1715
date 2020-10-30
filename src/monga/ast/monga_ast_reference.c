#include "monga_ast_reference.h"

#include <stdio.h>
#include <stdlib.h>

static const char* reference_kind_names[] = {
    [MONGA_AST_REFERENCE_VARIABLE] = "variable",
    [MONGA_AST_REFERENCE_TYPE] = "type",
    [MONGA_AST_REFERENCE_FUNCTION] = "function",
    [MONGA_AST_REFERENCE_PARAMETER] = "parameter",
    [MONGA_AST_REFERENCE_FIELD] = "field",
};

size_t monga_ast_reference_line(struct monga_ast_reference_t* reference)
{
    switch (reference->tag) {
    case MONGA_AST_REFERENCE_VARIABLE:
        return reference->u.def_variable->line;
    case MONGA_AST_REFERENCE_TYPE:
        return reference->u.def_type->line;
    case MONGA_AST_REFERENCE_FUNCTION:
        return reference->u.def_function->line;
    case MONGA_AST_REFERENCE_PARAMETER:
        return reference->u.parameter->line;
    case MONGA_AST_REFERENCE_FIELD:
        return reference->u.field->line;
    default:
        monga_unreachable();
        return 0;
    }
}

const char* monga_ast_reference_kind_name(enum monga_ast_reference_tag_t tag)
{
    monga_assert(tag >= 0 && tag < MONGA_AST_REFERENCE_CNT);
    return reference_kind_names[tag];
}

void monga_ast_reference_check_kind(struct monga_ast_reference_t* reference, enum monga_ast_reference_tag_t expected_tag, size_t line)
{
    if (reference->tag != expected_tag) {
        const char* id = reference->id;
        const char* kind = monga_ast_reference_kind_name(reference->tag);
        const char* expected_kind = monga_ast_reference_kind_name(expected_tag);
        fprintf(stderr, "Expected %s name instead of %s name \"%s\" in line %zu\n", expected_kind, kind, id, line);
        exit(MONGA_ERR_REFERENCE_KIND);
    }
}