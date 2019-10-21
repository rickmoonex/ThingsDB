#include <ti/fn/fn.h>

static int do__f_set_type(ti_query_t * query, cleri_node_t * nd, ex_t * e)
{
    const int nargs = langdef_nd_n_function_params(nd);
    ti_type_t * type;
    ti_thing_t * thing;
    ti_task_t * task;
    size_t n;

    if (fn_not_collection_scope("set_type", query, e) ||
        fn_nargs("set_type", DOC_SET_TYPE, 2, nargs, e) ||
        ti_do_statement(query, nd->children->node, e) ||
        fn_arg_str("set_type", DOC_SET_TYPE, 1, query->rval, e))
        return e->nr;

    type = ti_types_by_raw(query->collection->types, (ti_raw_t *) query->rval);
    if (!type)
        return ti_raw_err_not_found((ti_raw_t *) query->rval, "type", e);

    if (type->fields->n)
    {
        ex_set(e, EX_OPERATION_ERROR,
            "function `set_type` works only on a new type; "
            "use `mod_type()` if you want to change an existing type"
            DOC_MOD_TYPE);
        goto fail0;
    }

    if (ti_type_try_lock(type, e))
        return e->nr;

    ti_val_drop(query->rval);
    query->rval = NULL;

    if (ti_do_statement(query, nd->children->next->next->node, e))
        goto fail0;

    if (!ti_val_is_thing(query->rval))
    {
        ex_set(e, EX_TYPE_ERROR,
            "function `set_type` expects argument 2 to be of "
            "type `"TI_VAL_THING_S"` but got type `%s` instead"DOC_NEW_TYPE,
            ti_val_str(query->rval));
        goto fail0;
    }

    n = ti_query_count_type(query, type);
    if (n)
    {
        ex_set(e, EX_OPERATION_ERROR,
            "function `set_type` can only be used on a type without active "
            "instances; %zu active instance%s of type `%s` %s been found"
            DOC_SET_TYPE,
            n, n == 1 ? "" : "s", type->name, n == 1 ? "has" : "have");
        goto fail0;
    }

    thing = (ti_thing_t *) query->rval;
    query->rval = NULL;

    if (ti_type_init_from_thing(type, thing, e))
        goto fail1;

    task = ti_task_get_task(query->ev, query->collection->root, e);
    if (!task)
        goto fail1;

    if (ti_task_add_set_type(task, type))
    {
        ex_set_mem(e);
        goto fail1;
    }

    query->rval = (ti_val_t *) ti_nil_get();
    ti_type_map_cleanup(type);

fail1:
    ti_val_drop((ti_val_t *) thing);
fail0:
    ti_type_unlock(type, true /* lock is set for sure */);
    return e->nr;
}