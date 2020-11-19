#include "monga_ast_typedesc.h"

#include <stdlib.h>

#include "monga_ast_builtin.h"

void monga_ast_typedesc_write(FILE* f, struct monga_ast_typedesc_t* typedesc)
{
    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        fprintf(f, "%s", monga_ast_builtin_typedesc_id(typedesc->u.builtin_typedesc));
        break;
    case MONGA_AST_TYPEDESC_ID:
        fprintf(f, "%s", typedesc->u.id_typedesc.id);
        break;
    case MONGA_AST_TYPEDESC_ARRAY:
        fprintf(f, "[");
        monga_ast_typedesc_write(f, typedesc->u.array_typedesc);
        fprintf(f, "]");
        break;
    case MONGA_AST_TYPEDESC_RECORD:
        fprintf(f, "{");
        {
            struct monga_ast_field_t* field;
            for (field = typedesc->u.record_typedesc.field_list->first; field; field = field->next) {
                fprintf(f, "%s : ", field->id);
                monga_ast_typedesc_write(f, field->type.u.def_type->typedesc);
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

struct monga_ast_typedesc_t* monga_ast_typedesc_resolve_id(struct monga_ast_typedesc_t *typedesc)
{
    while (typedesc->tag == MONGA_AST_TYPEDESC_ID) {
        struct monga_ast_reference_t* id_typedesc = &typedesc->u.id_typedesc;
        monga_assert(id_typedesc->tag == MONGA_AST_REFERENCE_TYPE);
        typedesc = id_typedesc->u.def_type->typedesc;
    }
    return typedesc;
}

bool monga_ast_typedesc_numeric(struct monga_ast_typedesc_t* typedesc)
{
    typedesc = monga_ast_typedesc_resolve_id(typedesc);
    if (typedesc->tag == MONGA_AST_TYPEDESC_BUILTIN) {
        enum monga_ast_typedesc_builtin_t builtin = typedesc->u.builtin_typedesc;
        return builtin == MONGA_AST_TYPEDESC_BUILTIN_INT || builtin == MONGA_AST_TYPEDESC_BUILTIN_FLOAT;
    } else {
        return false; /* all the other kinds of type descriptors aren't numeric */
    }
}

void monga_ast_typedesc_check_self_reference(struct monga_ast_typedesc_t* typedesc)
{
    struct monga_ast_def_type_t* def_type = NULL; /* definition of self-referecing type */

    switch (typedesc->tag) {
    case MONGA_AST_TYPEDESC_BUILTIN:
        break; /* built-in types never reference each other */
    case MONGA_AST_TYPEDESC_ID:
    {
        struct monga_ast_reference_t* reference = &typedesc->u.id_typedesc;
        monga_assert(reference->tag == MONGA_AST_REFERENCE_TYPE);

        def_type = reference->u.def_type;
        break;
    }
    case MONGA_AST_TYPEDESC_ARRAY:
    {
        struct monga_ast_typedesc_t* array_type = typedesc;
        
        /* get array first non-array typedesc */
        while (array_type->tag == MONGA_AST_TYPEDESC_ARRAY) {
            array_type = array_type->u.array_typedesc;
        }
        
        if (array_type->tag == MONGA_AST_TYPEDESC_ID) {
            struct monga_ast_reference_t* reference = &array_type->u.id_typedesc;
            monga_assert(reference->tag == MONGA_AST_REFERENCE_TYPE);
            def_type = reference->u.def_type;
        }
        break;
    }
    case MONGA_AST_TYPEDESC_RECORD:
        break; /* self reference in records is allowed */
    default:
        monga_unreachable();
    }

    if (def_type && def_type->typedesc == typedesc) {
        fprintf(stderr, "Type \"%s\" references itself (line %zu)\n", def_type->id, def_type->line);
        exit(MONGA_ERR_REDECLARATION);
    }
}

bool monga_ast_typedesc_castable(struct monga_ast_typedesc_t *totypedesc, struct monga_ast_typedesc_t *fromtypedesc)
{
    fromtypedesc = monga_ast_typedesc_resolve_id(fromtypedesc);
    totypedesc = monga_ast_typedesc_resolve_id(totypedesc);

    if (monga_ast_typedesc_assignable(totypedesc, fromtypedesc))
        return true;

    if (fromtypedesc->tag == totypedesc->tag && fromtypedesc->tag == MONGA_AST_TYPEDESC_BUILTIN)
        return monga_ast_builtin_castable(totypedesc->u.builtin_typedesc, fromtypedesc->u.builtin_typedesc);
    
    return false;
}

bool monga_ast_typedesc_sibling(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2)
{
    typedesc1 = monga_ast_typedesc_resolve_id(typedesc1);
    typedesc2 = monga_ast_typedesc_resolve_id(typedesc2);

    return monga_ast_typedesc_assignable(typedesc1, typedesc2) ||
           monga_ast_typedesc_assignable(typedesc2, typedesc1);
}

bool monga_ast_typedesc_equal(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2)
{
    typedesc1 = monga_ast_typedesc_resolve_id(typedesc1);
    typedesc2 = monga_ast_typedesc_resolve_id(typedesc2);

    return monga_ast_typedesc_assignable(typedesc1, typedesc2) &&
           monga_ast_typedesc_assignable(typedesc2, typedesc1);
}

struct monga_ast_typedesc_t* monga_ast_typedesc_parent(struct monga_ast_typedesc_t *typedesc1, struct monga_ast_typedesc_t *typedesc2)
{
    typedesc1 = monga_ast_typedesc_resolve_id(typedesc1);
    typedesc2 = monga_ast_typedesc_resolve_id(typedesc2);

    if (monga_ast_typedesc_assignable(typedesc1, typedesc2))
        return typedesc1;
    else if (monga_ast_typedesc_assignable(typedesc2, typedesc1))
        return typedesc2;
    else
        return NULL;
}

bool monga_ast_typedesc_assignable(struct monga_ast_typedesc_t *vartypedesc, struct monga_ast_typedesc_t *exptypedesc)
{
    vartypedesc = monga_ast_typedesc_resolve_id(vartypedesc);
    exptypedesc = monga_ast_typedesc_resolve_id(exptypedesc);

    if (vartypedesc->tag == exptypedesc->tag) {
        switch (vartypedesc->tag) {
        case MONGA_AST_TYPEDESC_BUILTIN:
            return vartypedesc->u.builtin_typedesc == exptypedesc->u.builtin_typedesc; /* same built-in type */
        case MONGA_AST_TYPEDESC_ID:
            monga_unreachable(); /* monga_ast_typedesc_resolve_id guarantees it */
            break;
        case MONGA_AST_TYPEDESC_ARRAY:
            return monga_ast_typedesc_equal(vartypedesc->u.array_typedesc, exptypedesc->u.array_typedesc); /* propagate to subtype */
        case MONGA_AST_TYPEDESC_RECORD:
            return vartypedesc == exptypedesc; /* same type definition */
        default:
            monga_unreachable();
        }
    }
    
    return false;
}