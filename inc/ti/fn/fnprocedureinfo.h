#include <ti/fn/fn.h>

#define PROCEDURE_INFO_DOC_ TI_SEE_DOC("#procedure_info")

static int do__f_procedure_info(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    assert (~query->syntax.flags & TI_SYNTAX_FLAG_NODE);
    assert (e->nr == 0);
    assert (nd->cl_obj->tp == CLERI_TP_LIST);
    assert (query->rval == NULL);

    ti_procedure_t * procedure;
    vec_t * procedures = query->target
            ? query->target->procedures
            : ti()->procedures;

    if (!langdef_nd_fun_has_one_param(nd))
    {
        int nargs = langdef_nd_n_function_params(nd);
        ex_set(e, EX_BAD_DATA,
                "function `procedure_info` takes 1 argument but %d were given"
                PROCEDURE_INFO_DOC_, nargs);
        return e->nr;
    }

    if (ti_do_scope(query, nd->children->node, e))
        return e->nr;

    if (!ti_val_is_raw(query->rval))
    {
        ex_set(e, EX_BAD_DATA,
                "function `procedure_info` expects argument 1 to be of "
                "type `"TI_VAL_RAW_S"` but got type `%s` instead"
                PROCEDURE_INFO_DOC_,
                ti_val_str(query->rval));
        return e->nr;
    }

    procedure = ti_procedures_by_name(procedures, (ti_raw_t *) query->rval);
    if (!procedure)
    {
        ex_set(e, EX_INDEX_ERROR, "procedure `%.*s` not found",
                (int) ((ti_raw_t *) query->rval)->n,
                (char *) ((ti_raw_t *) query->rval)->data);
        return e->nr;
    }

    ti_val_drop(query->rval);
    query->rval = ti_procedure_info_as_qpval(procedure);
    if (!query->rval)
        ex_set_alloc(e);

    return e->nr;
}