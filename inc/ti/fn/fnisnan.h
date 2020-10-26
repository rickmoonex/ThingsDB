#include <ti/fn/fn.h>

static int do__f_is_nan(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    const int nargs = langdef_nd_n_function_params(nd);
    _Bool is_nan;

    if (fn_nargs("is_nan", DOC_IS_NAN, 1, nargs, e) ||
        ti_do_statement(query, nd->children->node, e))
        return e->nr;

    switch (query->rval->tp)
    {
    case TI_VAL_BOOL:
    case TI_VAL_INT:
        is_nan = false;
        break;
    case TI_VAL_FLOAT:
        is_nan = isnan(VFLOAT(query->rval));
        break;
    default:
        is_nan = true;
    }

    ti_val_unsafe_drop(query->rval);
    query->rval = (ti_val_t *) ti_vbool_get(is_nan);

    return e->nr;
}

static int do__f_isnan(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    log_debug("function `isnan` is deprecated, use `is_nan` instead");
    return do__f_is_nan(query, nd, e);
}
