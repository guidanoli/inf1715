#include "monga_ast_builtin.h"

#include <stddef.h>
#include <string.h>

static const struct monga_ast_typedesc_t monga_ast_builtin_int_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, MONGA_AST_TYPEDESC_BUILTIN_INT };

static const struct monga_ast_typedesc_t monga_ast_builtin_float_typedesc = {
    MONGA_AST_TYPEDESC_BUILTIN, MONGA_AST_TYPEDESC_BUILTIN_FLOAT };

static const struct monga_ast_typedesc_t* const monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = &monga_ast_builtin_int_typedesc,
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = &monga_ast_builtin_float_typedesc,
};

static const char* const monga_ast_builtin_typedesc_ids[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "int",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "float",
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

void monga_ast_builtin_init()
{
    for (size_t i = 0; i < sizeofv(monga_ast_builtin_def_types); ++i) {
        enum monga_ast_typedesc_builtin_t builtin;
        const struct monga_ast_typedesc_t* typedesc;
        const char* typedesc_id;

        builtin = (enum monga_ast_typedesc_builtin_t) i;
        typedesc = monga_ast_builtin_typedesc(builtin);
        typedesc_id = monga_ast_builtin_typedesc_id(builtin);

        monga_ast_builtin_def_types[i] = construct(def_type);
        monga_ast_builtin_def_types[i]->typedesc = (struct monga_ast_typedesc_t*) monga_memdup(typedesc, sizeof(*typedesc));
        monga_ast_builtin_def_types[i]->id = (char*) monga_memdup(typedesc_id, strlen(typedesc_id)+1);
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