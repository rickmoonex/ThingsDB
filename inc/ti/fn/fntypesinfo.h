#include <ti/fn/fn.h>

static int do__f_types_info(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    const int nargs = langdef_nd_n_function_params(nd);

    if (fn_not_collection_scope("types_info", query, e) ||
        fn_nargs("types_info", DOC_TYPES_INFO, 0, nargs, e))
        return e->nr;

    query->rval = ti_types_as_mpval(query->collection->types);
    if (!query->rval)
        ex_set_mem(e);

    return e->nr;
}
