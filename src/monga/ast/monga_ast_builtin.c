#include "monga_ast_builtin.h"

#include <stddef.h>
#include <string.h>

static struct monga_ast_typedesc_t monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = { MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_INT }, 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = { MONGA_AST_TYPEDESC_BUILTIN, { MONGA_AST_TYPEDESC_BUILTIN_FLOAT }, 0 },
};

static struct monga_ast_def_type_t monga_ast_builtin_def_types[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = { "int", &monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_INT], 0 },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = { "float", &monga_ast_builtin_typedescs[MONGA_AST_TYPEDESC_BUILTIN_FLOAT], 0 },
};

static bool monga_ast_builtin_typedesc_cast_matrix[][MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
    },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
    },
};

static bool monga_ast_builtin_typedesc_visible_array[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = true,
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = true,
};

static const char* monga_ast_builtin_llvm_equivalents[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "i32",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "float",
};

static const char* monga_ast_builtin_llvm_cast_instructions[][MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = NULL,
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fptosi",
    },
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = {
        [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sitofp",
        [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = NULL,
    },
};

static const char* monga_ast_builtin_llvm_add_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "add nsw",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fadd",
};

static const char* monga_ast_builtin_llvm_sub_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sub nsw",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fsub",
};

static const char* monga_ast_builtin_llvm_mul_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "mul nsw",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fmul",
};

static const char* monga_ast_builtin_llvm_div_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sdiv",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fdiv",
};

static const char* monga_ast_builtin_llvm_zeros[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "0",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "0.0",
};

static const char* monga_ast_builtin_llvm_cmp_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "icmp",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "fcmp",
};

static const char* monga_ast_builtin_llvm_le_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sle",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "ole",
};

static const char* monga_ast_builtin_llvm_ge_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sge",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "oge",
};

static const char* monga_ast_builtin_llvm_lt_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "slt",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "olt",
};

static const char* monga_ast_builtin_llvm_gt_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "sgt",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "ogt",
};

static const char* monga_ast_builtin_llvm_eq_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "eq",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "oeq",
};

static const char* monga_ast_builtin_llvm_ne_instructions[MONGA_AST_TYPEDESC_BUILTIN_CNT] = {
    [MONGA_AST_TYPEDESC_BUILTIN_INT] = "ne",
    [MONGA_AST_TYPEDESC_BUILTIN_FLOAT] = "one",
};

void monga_ast_builtin_typedesc_check(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_assert(builtin >= 0 && builtin < MONGA_AST_TYPEDESC_BUILTIN_CNT);
}

const char* monga_ast_builtin_typedesc_id(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_def_types[builtin].id;
}

const char* monga_ast_builtin_typedesc_llvm(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_equivalents[builtin];
}

struct monga_ast_typedesc_t* monga_ast_builtin_typedesc(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return &monga_ast_builtin_typedescs[builtin];
}

struct monga_ast_def_type_t* monga_ast_builtin_def_type(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return &monga_ast_builtin_def_types[builtin];
}

bool monga_ast_builtin_castable(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from)
{
    monga_ast_builtin_typedesc_check(to);
    monga_ast_builtin_typedesc_check(from);
    return monga_ast_builtin_typedesc_cast_matrix[to][from];
}

bool monga_ast_builtin_visible(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_typedesc_visible_array[builtin];
}

const char* monga_ast_builtin_typedesc_zero_llvm(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_zeros[builtin];
}

const char* monga_ast_builtin_llvm_cast_instruction(enum monga_ast_typedesc_builtin_t to, enum monga_ast_typedesc_builtin_t from)
{
    monga_ast_builtin_typedesc_check(to);
    monga_ast_builtin_typedesc_check(from);
    return monga_ast_builtin_llvm_cast_instructions[to][from];
}

const char* monga_ast_builtin_llvm_add_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_add_instructions[builtin];
}

const char* monga_ast_builtin_llvm_sub_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_sub_instructions[builtin];
}

const char* monga_ast_builtin_llvm_mul_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_mul_instructions[builtin];
}

const char* monga_ast_builtin_llvm_div_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_div_instructions[builtin];
}

const char* monga_ast_builtin_llvm_cmp_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_cmp_instructions[builtin];
}

const char* monga_ast_builtin_llvm_le_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_le_instructions[builtin];
}

const char* monga_ast_builtin_llvm_ge_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_ge_instructions[builtin];
}

const char* monga_ast_builtin_llvm_lt_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_lt_instructions[builtin];
}

const char* monga_ast_builtin_llvm_gt_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_gt_instructions[builtin];
}

const char* monga_ast_builtin_llvm_eq_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_eq_instructions[builtin];
}

const char* monga_ast_builtin_llvm_ne_instruction(enum monga_ast_typedesc_builtin_t builtin)
{
    monga_ast_builtin_typedesc_check(builtin);
    return monga_ast_builtin_llvm_ne_instructions[builtin];
}
