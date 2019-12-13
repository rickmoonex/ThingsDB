#include <ti/fn/fn.h>

static int do__f_code(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    const int nargs = langdef_nd_n_function_params(nd);
    ti_verror_t * err;

    if (!ti_val_is_error(query->rval))
    {
        ex_set(e, EX_LOOKUP_ERROR,
                "type `%s` has no function `code`",
                ti_val_str(query->rval));
        return e->nr;
    }

    if (fn_nargs("code", DOC_ERROR_CODE, 0, nargs, e))
        return e->nr;

    err = (ti_verror_t *) query->rval;

    query->rval = (ti_val_t *) ti_vint_create(err->code);
    if (!query->rval)
        ex_set_mem(e);

    ti_val_drop((ti_val_t *) err);
    return e->nr;
}