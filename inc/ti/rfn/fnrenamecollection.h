#include <ti/rfn/fn.h>

static int rq__f_rename_collection(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    assert (!rq__is_not_thingsdb(query, nd, e));
    assert (e->nr == 0);
    assert (query->ev);
    assert (nd->cl_obj->tp == CLERI_TP_LIST);
    assert (query->rval == NULL);

    int n;
    ti_task_t * task;
    ti_collection_t * collection;

    n = langdef_nd_n_function_params(nd);
    if (n != 2)
    {
        ex_set(e, EX_BAD_DATA,
            "function `rename_collection` requires 2 arguments "
            "but %d %s given",
            n, n == 1 ? "was" : "were");
        return e->nr;
    }

    if (rq__scope(query, nd->children->node, e))
        return e->nr;

    assert (e->nr == 0);
    collection = ti_collections_get_by_val(query->rval, e);
    if (e->nr)
        return e->nr;
    assert (collection);

    if (rq__scope(query, nd->children->next->next->node, e))
        return e->nr;

    if (!ti_val_is_raw(query->rval))
    {
        ex_set(e, EX_BAD_DATA,
            "function `rename_collection` expects argument 2 to be of "
            "type `"TI_VAL_RAW_S"` but got type `%s` instead",
            ti_val_str(query->rval));
        return e->nr;
    }

    if (ti_collection_rename(collection, (ti_raw_t *) query->rval, e))
        return e->nr;

    task = ti_task_get_task(query->ev, ti()->thing0, e);
    if (!task)
        return e->nr;

    if (ti_task_add_rename_collection(task, collection))
        ex_set_alloc(e);  /* task cleanup is not required */

    ti_val_drop(query->rval);
    query->rval = (ti_val_t *) ti_nil_get();

    return e->nr;
}
