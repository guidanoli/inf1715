#include "monga_ast_builtin.h"

#include <stddef.h>
#include <string.h>

static struct monga_ast_typedesc_t monga_ast_builtin_int_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_INT }, 0 };

static struct monga_ast_typedesc_t monga_ast_builtin_float_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_FLOAT }, 0 };

static struct monga_ast_typedesc_t monga_ast_builtin_null_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_NULL }, 0 };

static struct monga_ast_typedesc_t* monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = &monga_ast_builtin_int_typedesc,
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = &monga_ast_builtin_float_typedesc,
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = &monga_ast_builtin_null_typedesc,
};

static const char* monga_ast_builtin_typedesc_ids[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "int",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "float",
    [MONGA_AST_TYPEDESC_BUILTIN_NULL] = "null",
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

static struct monga_ast_def_type_t* monga_ast_builtin_def_types[MONGA_AST_TYPEDESC_BUILTIN_CNT];

const char* monga_ast_builtin_typedesc_id(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_typedesc_ids[builtin];
}

struct monga_ast_typedesc_t* monga_ast_builtin_typedesc(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_typedescs[builtin];
}

struct monga_ast_def_type_t* monga_ast_builtin_def_type(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_def_types[builtin];
}

bool monga_ast_builtin_castable(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from)
{
    monga_assert(to >= 0 && to < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    monga_assert(from >= 0 && from < MONGA_AST_TYPEDESC_BUILTIN_CNT);
    return monga_ast_builtin_typedesc_cast_matrix[to][from];
}

void monga_ast_builtin_init()
{
    for (size_t i = 0; i < sizeofv(monga_ast_builtin_def_types); ++i) {
        enum monga_ast_typedesc_builtin_t builtin;
        struct monga_ast_typedesc_t* typedesc;
        const char* typedesc_id;

        builtin = (enum monga_ast_typedesc_builtin_t) i;
        typedesc = monga_ast_builtin_typedesc(builtin);
        typedesc_id = monga_ast_builtin_typedesc_id(builtin);

        monga_ast_builtin_def_types[i] = construct(def_type);
        monga_ast_builtin_def_types[i]->typedesc = (struct monga_ast_typedesc_t*) monga_memdup(typedesc, sizeof(*typedesc));
        monga_ast_builtin_def_types[i]->id = (char*) monga_memdup(typedesc_id, strlen(typedesc_id)+1);
        monga_ast_builtin_def_types[i]->line = 0;
    }
}

void monga_ast_builtin_close()
{
    for (size_t i = 0; i < sizeofv(monga_ast_builtin_def_types); ++i) {
        monga_free(monga_ast_builtin_def_types[i]->typedesc);
        monga_free(monga_ast_builtin_def_types[i]->id);
        monga_free(monga_ast_builtin_def_types[i]);
    }
}