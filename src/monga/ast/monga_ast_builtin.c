#include "monga_ast_builtin.h"

#include <stddef.h>
#include <string.h>

static struct monga_ast_typedesc_t monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = { MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_INT }, 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = { MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_FLOAT }, 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = { MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_NULL }, 0 },
};

static struct monga_ast_def_type_t monga_ast_builtin_def_types[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = { "int", &monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_INT], 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = { "float", &monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_FLOAT], 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = { "null", &monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_NULL], 0 },
};

static bool monga_ast_builtin_typedesc_cast_matrix[][MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_NULL] = false,
    },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_NULL] = false,
    },
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = false,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = false,
        [MONGA_AST_TYPEDESC_BUILTIN_NULL] = true,
    },
};

static bool monga_ast_builtin_typedesc_visible_array[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = false,
};

static const char* monga_ast_builtin_llvm_equivalents[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "int32",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "float",
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = "opaque",
};

const char* monga_ast_builtin_typedesc_id(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_def_types[builtin].id;
}

const char* monga_ast_builtin_typedesc_llvm(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_llvm_equivalents[builtin];
}

struct monga_ast_typedesc_t* monga_ast_builtin_typedesc(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return &monga_ast_builtin_typedescs[builtin];
}

struct monga_ast_def_type_t* monga_ast_builtin_def_type(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return &monga_ast_builtin_def_types[builtin];
}

bool monga_ast_builtin_castable(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from)
{
    monga_assert(to >= 0 && to < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    monga_assert(from >= 0 && from < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_typedesc_cast_matrix[to][from];
}

bool monga_ast_builtin_visible(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_typedesc_visible_array[builtin];
}