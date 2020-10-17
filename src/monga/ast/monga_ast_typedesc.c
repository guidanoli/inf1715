#include "monga_ast_typedesc.h"

#include <string.h>

#include "monga_ast_builtin.h"

void monga_ast_typedesc_copy(const struct monga_ast_typedesc_t *orig, struct monga_ast_typedesc_t *dest)
{
    memcpy(dest, orig, sizeof(*dest));
}

void monga_ast_typedesc_make_array(struct monga_ast_typedesc_t *array_type, struct monga_ast_typedesc_t *typedesc)
{
    typedesc->tag = MONGA_AST_TYPEDESC_ARRAY;
    typedesc->array_typedesc = array_type;
}

void monga_ast_typedesc_write(FILE* f, struct monga_ast_typedesc_t* typedesc)
{
    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        fprintf(f, "%s", monga_ast_builtin_typedesc_id(typedesc->builtin_typedesc));
        break;
    case MONGA_AST_TYPEDESC_ID:
        fprintf(f, "%s", typedesc->id_typedesc.id);
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        fprintf(f, "[");
        monga_ast_typedesc_write(f, typedesc->array_typedesc);
        fprintf(f, "]");
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        fprintf(f, "{");
        {
            struct monga_ast_field_t* field;
            for (field = typedesc->record_typedesc->first; field; field = field->next) {
                fprintf(f, "%s : ", field->id);
                monga_ast_typedesc_write(f, field->type.def_type->typedesc);
                if (field->next)
                    fprintf(f, "; ");
            }

        }
        fprintf(f, "}");
        break;
    default:
        monga_unreachable();
    }
}