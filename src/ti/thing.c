/*
 * thing.c
 */
#include <assert.h>
#include <stdlib.h>
#include <ti.h>
#include <ti/prop.h>
#include <ti/proto.h>
#include <ti/thing.h>
#include <ti/val.h>
#include <ti/names.h>
#include <util/qpx.h>
#include <util/logger.h>

static void thing__watch_del(ti_thing_t * thing)
{
    assert (thing->watchers);

    ti_pkg_t * pkg;
    ti_rpkg_t * rpkg;
    qpx_packer_t * packer = qpx_packer_create(12, 1);
    if (!packer)
    {
        log_critical(EX_MEMORY_S);
        return;
    }
    (void) ti_thing_id_to_packer(thing, &packer);

    pkg = qpx_packer_pkg(packer, TI_PROTO_CLIENT_WATCH_DEL);

    rpkg = ti_rpkg_create(pkg);
    if (!rpkg)
    {
        log_critical(EX_MEMORY_S);
        return;
    }

    for (vec_each(thing->watchers, ti_watch_t, watch))
    {
        if (ti_stream_is_closed(watch->stream))
            continue;

        if (ti_stream_write_rpkg(watch->stream, rpkg))
            log_critical(EX_INTERNAL_S);
    }

    ti_rpkg_drop(rpkg);
}

ti_thing_t * ti_thing_create(uint64_t id, imap_t * things)
{
    ti_thing_t * thing = malloc(sizeof(ti_thing_t));
    if (!thing)
        return NULL;

    thing->ref = 1;
    thing->tp = TI_VAL_THING;
    thing->flags = TI_VFLAG_THING_SWEEP;

    thing->id = id;
    thing->things = things;
    thing->props = vec_new(0);
    thing->watchers = NULL;

    if (!thing->props)
    {
        ti_thing_destroy(thing);
        return NULL;
    }
    return thing;
}

void ti_thing_destroy(ti_thing_t * thing)
{
    if (!thing)
        return;

    if (thing->id)
        (void) imap_pop(thing->things, thing->id);

    if ((~ti()->flags & TI_FLAG_SIGNAL) && ti_thing_has_watchers(thing))
        thing__watch_del(thing);

    vec_destroy(thing->props, (vec_destroy_cb) ti_prop_destroy);
    vec_destroy(thing->watchers, (vec_destroy_cb) ti_watch_drop);
    free(thing);
}

void ti_thing_clear(ti_thing_t * thing)
{
    ti_prop_t * prop;
    while ((prop = vec_pop(thing->props)))
        ti_prop_destroy(prop);
}

ti_prop_t * ti_thing_prop_weak_get(ti_thing_t * thing, ti_name_t * name)
{
    for (vec_each(thing->props, ti_prop_t, prop))
        if (prop->name == name)
            return prop;
    return NULL;
}

/*
 * Does not increment the `name` and `val` reference counters.
 * Use only when you are sure the property does not yet exist.
 */
ti_prop_t * ti_thing_prop_add(ti_thing_t * thing, ti_name_t * name, ti_val_t * val)
{
    assert (ti_thing_prop_weak_get(thing, name) == NULL);

    ti_prop_t * prop = ti_prop_create(name, val);
    if (!prop || vec_push(&thing->props, prop))
    {
        free(prop);
        return NULL;
    }

    return prop;
}

/*
 * Does not increment the `name` and `val` reference counters.
 */
ti_prop_t * ti_thing_prop_set(ti_thing_t * thing, ti_name_t * name, ti_val_t * val)
{
    ti_prop_t * prop;

    for (vec_each(thing->props, ti_prop_t, p))
    {
        if (p->name == name)
        {
            ti_decref(name);
            ti_val_drop(p->val);
            p->val = val;
            return p;
        }
    }

    prop = ti_prop_create(name, val);
    if (!prop || vec_push(&thing->props, prop))
    {
        free(prop);
        return NULL;
    }

    return prop;
}

/*
 * Does not increment the `name` and `val` reference counters.
 */
ti_prop_t * ti_thing_prop_set_e(
        ti_thing_t * thing,
        ti_name_t * name,
        ti_val_t * val,
        ex_t * e)
{
    ti_prop_t * prop;

    for (vec_each(thing->props, ti_prop_t, p))
    {
        if (p->name == name)
        {
            if (ti_val_is_locked(p->val, e))
                return NULL;
            ti_decref(name);
            ti_val_drop(p->val);
            p->val = val;
            return p;
        }
    }

    prop = ti_prop_create(name, val);
    if (!prop || vec_push(&thing->props, prop))
    {
        free(prop);
        return NULL;
    }

    return prop;
}

/* Returns true if the property is removed, false if not found */
_Bool ti_thing_del(ti_thing_t * thing, ti_name_t * name)
{
    uint32_t i = 0;
    for (vec_each(thing->props, ti_prop_t, prop), ++i)
    {
        if (prop->name == name)
        {
            ti_prop_destroy(vec_swap_remove(thing->props, i));
            return true;
        }
    }
    return false;
}

/* Returns 0 if the property is removed, -1 in case of an error */
int ti_thing_del_e(ti_thing_t * thing, ti_raw_t * rname, ex_t * e)
{
    uint32_t i = 0;
    ti_name_t * name = ti_names_weak_get((const char *) rname->data, rname->n);

    if (name)
    {
        for (vec_each(thing->props, ti_prop_t, prop), ++i)
        {
            if (prop->name == name)
            {
                if (ti_val_is_locked(prop->val, e))
                    return e->nr;
                ti_prop_destroy(vec_swap_remove(thing->props, i));
                return 0;
            }
        }
    }

    if (ti_name_is_valid_strn((const char *) rname->data, rname->n))
    {
        ex_set(e, EX_INDEX_ERROR,
                "thing "TI_THING_ID" has no property `%.*s`",
                thing->id,
                (int) rname->n, (const char *) rname->data);
    }
    else
    {
        ex_set(e, EX_BAD_DATA, "expecting a valid name"TI_SEE_DOC("#names"));
    }

    return e->nr;
}


/*
 * Returns true if `from` is found and replaced by to, false if not found.
 * If found, then the `from` reference which was used by the thing will be
 * decremented, the reference count of `to` will never change so when using
 * this function you should act on the return value.
 */
_Bool ti_thing_rename(ti_thing_t * thing, ti_name_t * from, ti_name_t * to)
{
    uint32_t i = 0;
    for (vec_each(thing->props, ti_prop_t, prop), ++i)
    {
        if (prop->name == from)
        {
            ti_name_drop(prop->name);
            prop->name = to;
            return true;
        }
    }
    return false;
}

int ti_thing_gen_id(ti_thing_t * thing)
{
    assert (!thing->id);

    thing->id = ti_next_thing_id();
    ti_thing_mark_new(thing);

    if (ti_thing_to_map(thing))
        return -1;

    /*
     * Recursion is required since nested things did not generate a task
     * as long as their parent was not attached to the collection.
     */
    for (vec_each(thing->props, ti_prop_t, prop))
        if (ti_val_gen_ids(prop->val))
            return -1;

    return 0;
}

ti_watch_t *  ti_thing_watch(ti_thing_t * thing, ti_stream_t * stream)
{
    ti_watch_t * watch;
    ti_watch_t ** empty_watch = NULL;
    if (!thing->watchers)
    {
        thing->watchers = vec_new(1);
        if (!thing->watchers)
            return NULL;
        watch = ti_watch_create(stream);
        if (!watch)
            return NULL;
        VEC_push(thing->watchers, watch);
        goto finish;
    }
    for (vec_each(thing->watchers, ti_watch_t, watch))
    {
        if (watch->stream == stream)
            return watch;
        if (!watch->stream)
            empty_watch = v__;
    }

    if (empty_watch)
    {
        watch = *empty_watch;
        watch->stream = stream;
        goto finish;
    }

    watch = ti_watch_create(stream);
    if (!watch)
        return NULL;

    if (vec_push(&thing->watchers, watch))
        goto failed;

finish:
    if (!stream->watching)
    {
        stream->watching = vec_new(1);
        if (!stream->watching)
            goto failed;
        VEC_push(stream->watching, watch);
    }
    else if (vec_push(&stream->watching, watch))
        goto failed;

    return watch;

failed:
    /* when this fails, a few bytes might leak */
    watch->stream = NULL;
    return NULL;
}

_Bool ti_thing_unwatch(ti_thing_t * thing, ti_stream_t * stream)
{
    if (!thing->watchers)
        return false;

    for (vec_each(thing->watchers, ti_watch_t, watch))
    {
        if (watch->stream == stream)
        {
            watch->stream = NULL;
            return true;
        }
    }
    return false;
}

int ti_thing_to_packer(ti_thing_t * thing, qp_packer_t ** packer, int options)
{
    assert (options);  /* should be either positive or negative, not 0 */

    if (    qp_add_map(packer) ||
            qp_add_raw(*packer, (const uchar *) TI_KIND_S_THING, 1) ||
            qp_add_int(*packer, thing->id))
        return -1;

    --options;

    for (vec_each(thing->props, ti_prop_t, prop))
    {
        if (    qp_add_raw_from_str(*packer, prop->name->str) ||
                ti_val_to_packer(prop->val, packer, options))
            return -1;
    }

    return qp_close_map(*packer);
}

_Bool ti__thing_has_watchers_(ti_thing_t * thing)
{
    assert (thing->watchers);
    for (vec_each(thing->watchers, ti_watch_t, watch))
        if (watch->stream && (~watch->stream->flags & TI_STREAM_FLAG_CLOSED))
            return true;
    return false;
}
