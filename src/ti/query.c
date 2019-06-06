/*
 * ti/query.c
 */
#include <assert.h>
#include <errno.h>
#include <langdef/nd.h>
#include <langdef/translate.h>
#include <qpack.h>
#include <stdlib.h>
#include <ti/closure.h>
#include <ti.h>
#include <ti/collections.h>
#include <ti/cq.h>
#include <ti/epkg.h>
#include <ti/proto.h>
#include <ti/query.h>
#include <ti/rq.h>
#include <ti/task.h>
#include <util/qpx.h>
#include <util/strx.h>


#define QUERY__MAX_DEEP 0x7f

static void query__investigate_array(ti_query_t * query, cleri_node_t * nd);
static void query__investigate_recursive(ti_query_t * query, cleri_node_t * nd);
static _Bool query__swap_opr(
        ti_query_t * query,
        cleri_children_t * parent,
        uint32_t parent_gid);
static void query__event_handle(ti_query_t * query);
static ti_epkg_t * query__epkg_event(ti_query_t * query);
static void query__task_to_watchers(ti_query_t * query);
static inline _Bool query__requires_root_event(cleri_node_t * name_nd);
static int query__node_db_unpack(
        const char * ebad,
        ti_query_t * query,
        uint16_t pkg_id,
        const uchar * data,
        size_t n,
        ex_t * e);

ti_query_t * ti_query_create(ti_stream_t * stream, ti_user_t * user)
{
    ti_query_t * query = malloc(sizeof(ti_query_t));
    if (!query)
        return NULL;

    query->scope = NULL;
    query->rval = NULL;
    query->deep = 1;
    query->flags = 0;
    query->target = NULL;  /* root */
    query->parseres = NULL;
    query->stream = ti_grab(stream);
    query->user = ti_grab(user);
    query->results = NULL;
    query->ev = NULL;
    query->blobs = NULL;
    query->querystr = NULL;
    query->nd_cache_count = 0;
    query->nd_cache = NULL;
    query->tmpvars = NULL;

    return query;
}

void ti_query_destroy(ti_query_t * query)
{
    if (!query)
        return;

    if (query->parseres)
        cleri_parse_free(query->parseres);

    vec_destroy(query->nd_cache, (vec_destroy_cb) ti_val_drop);
    vec_destroy(query->results, (vec_destroy_cb) ti_val_drop);
    vec_destroy(query->tmpvars, (vec_destroy_cb) ti_prop_destroy);
    ti_stream_drop(query->stream);
    ti_user_drop(query->user);
    ti_collection_drop(query->target);
    ti_event_drop(query->ev);
    vec_destroy(query->blobs, (vec_destroy_cb) ti_val_drop);
    free(query->querystr);
    free(query);
}

int ti_query_node_unpack(
        ti_query_t * query,
        uint16_t pkg_id,
        const uchar * data,
        size_t n,
        ex_t * e)
{
    assert (e->nr == 0);
    const char * ebad =
            "invalid `query-node` request, "
            "see "TI_DOCS"#query-node";
    return query__node_db_unpack(ebad, query, pkg_id, data, n, e);
}

int ti_query_thingsdb_unpack(
        ti_query_t * query,
        uint16_t pkg_id,
        const uchar * data,
        size_t n,
        ex_t * e)
{
    assert (e->nr == 0);
    const char * ebad =
            "invalid `query-thingsdb` request, "
            "see "TI_DOCS"#query-thingsdb";
    return query__node_db_unpack(ebad, query, pkg_id, data, n, e);
}

int ti_query_collection_unpack(
        ti_query_t * query,
        uint16_t pkg_id,
        const uchar * data,
        size_t n,
        ex_t * e)
{
    assert (e->nr == 0);
    const char * ebad =
            "invalid `query-collection` request, "
            "see "TI_DOCS"#query-collection";

    qp_unpacker_t unpacker;
    qp_obj_t key, val;
    size_t max_raw = 0;

    query->pkg_id = pkg_id;

    qp_unpacker_init2(&unpacker, data, n, 0);

    if (!qp_is_map(qp_next(&unpacker, NULL)))
    {
        ex_set(e, EX_BAD_DATA, ebad);
        goto finish;
    }

    while(qp_is_raw(qp_next(&unpacker, &key)))
    {
        if (qp_is_raw_equal_str(&key, "query"))
        {
            if (query->querystr || !qp_is_raw(qp_next(&unpacker, &val)))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }

            if (val.len > max_raw)
                max_raw = val.len;

            query->querystr = qpx_obj_raw_to_str(&val);
            if (!query->querystr)
            {
                ex_set_alloc(e);
                goto finish;
            }
            continue;
        }

        if (qp_is_raw_equal_str(&key, "collection"))
        {
            if (query->target)
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }

            (void) qp_next(&unpacker, &val);
            query->target = ti_collections_get_by_qp_obj(&val, e);
            if (e->nr)
                goto finish;
            ti_incref(query->target);
            continue;
        }

        if (qp_is_raw_equal_str(&key, "deep"))
        {
            if (!qp_is_int(qp_next(&unpacker, &val)))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }
            if (val.via.int64 < 0 || val.via.int64 > QUERY__MAX_DEEP)
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }

            query->deep = (uint8_t) val.via.int64;
            continue;
        }

        if (qp_is_raw_equal_str(&key, "all"))
        {
            if (!qp_is_bool(qp_next(&unpacker, &val)))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }
            if (qp_is_true(val.tp))
                query->flags |= TI_QUERY_FLAG_ALL;
            continue;
        }

        if (qp_is_raw_equal_str(&key, "blobs"))
        {
            ssize_t n;
            qp_types_t tp = qp_next(&unpacker, NULL);

            if (query->blobs || !qp_is_array(tp))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }

            n = tp == QP_ARRAY_OPEN ? -1 : (ssize_t) tp - QP_ARRAY0;

            query->blobs = vec_new(n < 0 ? 8 : n);
            if (!query->blobs)
            {
                ex_set_alloc(e);
                goto finish;
            }

            while(n-- && qp_is_raw(qp_next(&unpacker, &val)))
            {
                ti_raw_t * blob = ti_raw_create(val.via.raw, val.len);
                if (!blob || vec_push(&query->blobs, blob))
                {
                    ex_set_alloc(e);
                    goto finish;
                }

                if (blob->n > max_raw)
                    max_raw = blob->n;
            }
            continue;
        }

        log_debug(
                "unexpected `query-collection` key in map: `%.*s`",
                key.len, (const char *) key.via.raw);
    }

    if (!query->querystr || !query->target)
        ex_set(e, EX_BAD_DATA, ebad);

    if (query->target && max_raw >= query->target->quota->max_raw_size)
        ex_set(e, EX_MAX_QUOTA,
                "maximum raw size quota of %zu bytes is reached, "
                "see "TI_DOCS"#quotas", query->target->quota->max_raw_size);

finish:
    return e->nr;
}




int ti_query_parse(ti_query_t * query, ex_t * e)
{
    assert (e->nr == 0);
    query->parseres = cleri_parse2(
            ti()->langdef,
            query->querystr,
            CLERI_FLAG_EXPECTING_DISABLED|
            CLERI_FLAG_EXCLUDE_OPTIONAL);  /* only error position */
    if (!query->parseres)
    {
        ex_set_alloc(e);
        goto finish;
    }
    if (!query->parseres->is_valid)
    {
        int i = cleri_parse_strn(
                e->msg,
                EX_MAX_SZ,
                query->parseres,
                &langdef_translate);

        assert_log(i<EX_MAX_SZ, "expecting >= max size %d>=%d", i, EX_MAX_SZ);

        e->msg[EX_MAX_SZ] = '\0';

        log_warning("invalid syntax: `%s` (%s)", query->querystr, e->msg);

        /* we will certainly not hit the max size, but just to be safe */
        e->n = i < EX_MAX_SZ ? i : EX_MAX_SZ - 1;
        e->nr = EX_SYNTAX_ERROR;
        goto finish;
    }
finish:
    return e->nr;
}

int ti_query_investigate(ti_query_t * query, ex_t * e)
{
    cleri_children_t * child;
    int nstatements = 0;

    assert (e->nr == 0);

    child = query->parseres->tree   /* root */
            ->children->node        /* sequence <comment, list> */
            ->children->next->node  /* list */
            ->children;             /* first child or NULL */

    while(child)
    {
        ++nstatements;
        query__investigate_recursive(query, child->node);  /* scope */

        if (!child->next)
            break;
        child = child->next->next;                  /* skip delimiter */
    }

    nstatements = ((query->flags & TI_QUERY_FLAG_ALL) || !nstatements)
            ? nstatements : 1;

    query->results = vec_new(nstatements);
    if (!query->results || (
            query->nd_cache_count && !(
                    query->nd_cache = vec_new(query->nd_cache_count)
            )
    ))
        ex_set_alloc(e);

    /* remove flag which is not applicable */
    query->flags &= query->target
            ? ~TI_QUERY_FLAG_ROOT_EVENT
            : ~TI_QUERY_FLAG_COLLECTION_EVENT;

    if (query->flags & TI_QUERY_FLAG_OVERFLOW)
        ex_set(e, EX_OVERFLOW, "integer overflow");

    return e->nr;
}

void ti_query_run(ti_query_t * query)
{
    cleri_children_t * child;
    ex_t * e = ex_use();

    child = query->parseres->tree   /* root */
        ->children->node            /* sequence <comment, list> */
        ->children->next->node      /* list */
        ->children;                 /* first child or NULL */

    if (!child)
        goto failed;

    while (child)
    {
        assert (child->node->cl_obj->gid == CLERI_GID_SCOPE);

        if (query->target
                ? ti_cq_scope(query, child->node, e)
                : ti_rq_scope(query, child->node, e))
        {
            ti_val_drop(query->rval);
            query->rval = NULL;
            goto failed;
        }

        if (!child->next || !(child = child->next->next))
            break;

        if (query->flags & TI_QUERY_FLAG_ALL)
            VEC_push(query->results, query->rval);
        else
            ti_val_drop(query->rval);

        query->rval = NULL;
        ti_scope_leave(&query->scope, NULL);
    }

    VEC_push(query->results, query->rval);
    query->rval = NULL;
    ti_scope_leave(&query->scope, NULL);

failed:

    if (query->ev)
        query__event_handle(query);  /* error here will be logged only */

    ti_query_send(query, e);
}

void ti_query_send(ti_query_t * query, ex_t * e)
{
    ti_pkg_t * pkg;
    qpx_packer_t * packer;
    if (e->nr)
        goto pkg_err;

    packer = qpx_packer_create(65536, query->deep + 1);
    if (!packer)
        goto alloc_err;

    (void) ((query->flags & TI_QUERY_FLAG_ALL) && qp_add_array(&packer));

    /* we should have a result for each statement */
    assert (query->results->n == query->results->sz);

    /* ti_query_t or ti_root_t */
    for (vec_each(query->results, ti_val_t, rval))
    {
        assert (rval);
        if (ti_val_to_packer(rval, &packer, 0, query->deep))
            goto alloc_err;
    }

    if ((query->flags & TI_QUERY_FLAG_ALL) && qp_close_array(packer))
        goto alloc_err;

    pkg = qpx_packer_pkg(packer, TI_PROTO_CLIENT_RES_QUERY);
    pkg->id = query->pkg_id;

    goto finish;

alloc_err:
    if (packer)
        qp_packer_destroy(packer);
    ex_set_alloc(e);

pkg_err:
    pkg = ti_pkg_client_err(query->pkg_id, e);

finish:
    if (!pkg || ti_stream_write_pkg(query->stream, pkg))
    {
        free(pkg);
        log_critical(EX_ALLOC_S);
    }

    ti_query_destroy(query);
}

ti_val_t * ti_query_val_pop(ti_query_t * query)
{
    if (query->rval)
    {
        ti_val_t * val = query->rval;
        query->rval = NULL;
        return val;
    }

    if (query->scope->val)
    {
        ti_incref(query->scope->val);
        return query->scope->val;
    }

    ti_incref(query->scope->thing);
    return (ti_val_t *) query->scope->thing;
}

const char * ti_query_val_str(ti_query_t * query)
{
    return query->rval
            ? ti_val_str(query->rval)
            : (query->scope->val
                    ? ti_val_str(query->scope->val)
                    : TI_VAL_THING_S
            );
}

ti_prop_t * ti_query_tmpprop_get(ti_query_t * query, ti_name_t * name)
{
    if (!query->tmpvars)
        return NULL;

    for (vec_each(query->tmpvars, ti_prop_t, prop))
        if (prop->name == name)
            return prop;
    return NULL;
}


static void query__investigate_array(ti_query_t * query, cleri_node_t * nd)
{
    uintptr_t sz = 0;
    cleri_children_t * child = nd          /* sequence */
            ->children->next->node         /* list */
            ->children;
    for (; child; child = child->next->next)
    {
        query__investigate_recursive(query, child->node);
        ++sz;

        if (!child->next)
            break;
    }
    nd->data = (void *) sz;
}

static void query__investigate_recursive(ti_query_t * query, cleri_node_t * nd)
{
    switch (nd->cl_obj->gid)
    {
    case CLERI_GID_ARRAY:
        query__investigate_array(query, nd);
        return;
    case CLERI_GID_FUNCTION:
        switch (nd                              /* sequence */
                ->children->node                /* choice */
                ->children->node->cl_obj->gid)  /* keyword or name */
        {
        case CLERI_GID_F_NOW:
        case CLERI_GID_F_ID:
        case CLERI_GID_F_RET:
        case CLERI_GID_F_LEN:
            return;  /* arguments will be ignored */
        case CLERI_GID_F_POP:
            query->flags |= TI_QUERY_FLAG_COLLECTION_EVENT;
            return; /* arguments will be ignored */
        case CLERI_GID_F_DEL:
        case CLERI_GID_F_PUSH:
        case CLERI_GID_F_REMOVE:
        case CLERI_GID_F_RENAME:
        case CLERI_GID_F_SPLICE:
            query->flags |= TI_QUERY_FLAG_COLLECTION_EVENT;
            break;
        case CLERI_GID_NAME:
            if (query__requires_root_event(nd->children->node->children->node))
                query->flags |= TI_QUERY_FLAG_ROOT_EVENT;
            return;  /* arguments can be ignored */
        }
        /* skip to arguments */
        query__investigate_recursive(
                query,
                nd->children->next->next->node);
        return;
    case CLERI_GID_ASSIGNMENT:
        query->flags |= TI_QUERY_FLAG_COLLECTION_EVENT;
        /* skip to scope */
        query__investigate_recursive(
                query,
                nd->children->next->next->node);
        return;
    case CLERI_GID_CLOSURE:
        {
            uint8_t flags = query->flags;

            query->flags = 0;
            /* investigate the scope if required, the rest can be skipped */
            query__investigate_recursive(
                query,
                nd->children->next->next->next->node);

            nd->data = (void *) ((uintptr_t) (
                    query->flags & TI_QUERY_FLAG_COLLECTION_EVENT
                        ? TI_CLOSURE_FLAG_QBOUND|TI_CLOSURE_FLAG_WSE
                        : TI_CLOSURE_FLAG_QBOUND));

            query->flags |= flags;
        }
        return;
    case CLERI_GID_COMMENT:
        /* all with children we can skip */
        return;
    case CLERI_GID_PRIMITIVES:
        switch (nd->children->node->cl_obj->gid)
        {
            case CLERI_GID_T_INT:
            case CLERI_GID_T_FLOAT:
            case CLERI_GID_T_STRING:
            case CLERI_GID_T_REGEX:
                ++query->nd_cache_count;
                nd->children->node->data = NULL;    /* init data to null */
        }
        return;
    case CLERI_GID_OPERATIONS:
        (void) query__swap_opr(query, nd->children->next, 0);
        if (nd->children->next->next->next)             /* optional (seq) */
        {
            nd = nd->children->next->next->next->node;  /* sequence */
            query__investigate_recursive(
                    query,
                    nd->children->next->node);
            query__investigate_recursive(
                    query,
                    nd->children->next->next->next->node);
        }
        return;
    }

    for (cleri_children_t * child = nd->children; child; child = child->next)
        query__investigate_recursive(query, child->node);
}

static _Bool query__swap_opr(
        ti_query_t * query,
        cleri_children_t * parent,
        uint32_t parent_gid)
{
    cleri_node_t * node;
    cleri_node_t * nd = parent->node;
    cleri_children_t * chilcollection;
    uint32_t gid;

    assert( nd->cl_obj->tp == CLERI_TP_RULE ||
            nd->cl_obj->tp == CLERI_TP_PRIO ||
            nd->cl_obj->tp == CLERI_TP_THIS);

    node = nd->cl_obj->tp == CLERI_TP_PRIO ?
            nd->children->node :
            nd->children->node->children->node;

    if (node->cl_obj->gid == CLERI_GID_SCOPE)
    {
        query__investigate_recursive(query, node);
        return false;
    }

    assert (node->cl_obj->tp == CLERI_TP_SEQUENCE);

    gid = node->children->next->node->children->node->cl_obj->gid;

    assert (gid >= CLERI_GID_OPR0_MUL_DIV_MOD && gid <= CLERI_GID_OPR7_CMP_OR);

    chilcollection = node->children->next->next;

    (void) query__swap_opr(query, node->children, gid);
    if (query__swap_opr(query, chilcollection, gid))
    {
        cleri_node_t * tmp;
        cleri_children_t * bchilda;
        parent->node = chilcollection->node;
        tmp = chilcollection->node->cl_obj->tp == CLERI_TP_PRIO ?
                chilcollection->node->children->node :
                chilcollection->node->children->node->children->node;
        bchilda = tmp->children;
        gid = tmp->cl_obj->gid;
        tmp = bchilda->node;
        bchilda->node = nd;
        chilcollection->node = tmp;
    }

    return gid > parent_gid;
}

static void query__event_handle(ti_query_t * query)
{
    ti_epkg_t * epkg = query__epkg_event(query);
    if (!epkg)
    {
        log_critical(EX_ALLOC_S);
        return;
    }

    /* send tasks to watchers if required */
    if (query->target)
        query__task_to_watchers(query);

    /* store event package in archive */
    if (ti_archive_push(epkg))
        log_critical(EX_ALLOC_S);

    ti_nodes_write_rpkg((ti_rpkg_t *) epkg);
    ti_epkg_drop(epkg);
}

/*
 *  tasks are ordered for low to high thing ids
 *   { [0, 0]: {0: [ {'job':...} ] } }
 */
static ti_epkg_t * query__epkg_event(ti_query_t * query)
{
    ti_epkg_t * epkg;
    ti_pkg_t * pkg;
    qpx_packer_t * packer;
    size_t sz = 0;
    vec_t * tasks = query->ev->_tasks;
//    omap_iter_t iter = omap_ite_r(query->ev->tasks);

    for (vec_each(tasks, ti_task_t, task))
        sz += task->approx_sz;

    /* nest size 3 is sufficient since jobs are already raw */
    packer = qpx_packer_create(sz, 3);
    if (!packer)
        return NULL;

    (void) qp_add_map(&packer);

    (void) qp_add_array(&packer);
    (void) qp_add_int(packer, query->ev->id);
    /* store `no tasks` as target 0, this will save space and a lookup */
    (void) qp_add_int(packer, query->target && tasks->n
            ? query->target->root->id
            : 0);
    (void) qp_close_array(packer);

    (void) qp_add_map(&packer);

    /* reset iterator */
    for (vec_each(tasks, ti_task_t, task))
    {
        (void) qp_add_int(packer, task->thing->id);
        (void) qp_add_array(&packer);
        for (vec_each(task->jobs, ti_raw_t, raw))
        {
            (void) qp_add_qp(packer, raw->data, raw->n);
        }
        (void) qp_close_array(packer);
    }
    (void) qp_close_map(packer);
    (void) qp_close_map(packer);

    pkg = qpx_packer_pkg(packer, TI_PROTO_NODE_EVENT);
    epkg = ti_epkg_create(pkg, query->ev->id);
    if (!epkg)
    {
        free(pkg);
        return NULL;
    }
    return epkg;
}

static void query__task_to_watchers(ti_query_t * query)
{
    vec_t * tasks = query->ev->_tasks;
    for (vec_each(tasks, ti_task_t, task))
    {
        if (ti_thing_has_watchers(task->thing))
        {
            ti_rpkg_t * rpkg;
            ti_pkg_t * pkg = ti_task_pkg_watch(task);
            if (!pkg)
            {
                log_critical(EX_ALLOC_S);
                break;
            }

            rpkg = ti_rpkg_create(pkg);
            if (!rpkg)
            {
                log_critical(EX_ALLOC_S);
                break;
            }

            for (vec_each(task->thing->watchers, ti_watch_t, watch))
            {
                if (ti_stream_is_closed(watch->stream))
                    continue;

                if (ti_stream_write_rpkg(watch->stream, rpkg))
                    log_critical(EX_INTERNAL_S);
            }

            ti_rpkg_drop(rpkg);
        }
    }
}

static inline _Bool query__requires_root_event(cleri_node_t * fname_nd)
{
    /* a function has at least size 1 */
    switch (*fname_nd->str)
    {
    case 'd':
        return (
            langdef_nd_match_str(fname_nd, "del_collection") ||
            langdef_nd_match_str(fname_nd, "del_user")
        );
    case 'g':
        return (
            langdef_nd_match_str(fname_nd, "grant")
        );
    case 'n':
        return (
            langdef_nd_match_str(fname_nd, "new_collection") ||
            langdef_nd_match_str(fname_nd, "new_node") ||
            langdef_nd_match_str(fname_nd, "new_user")
        );
    case 'p':
        return (
            langdef_nd_match_str(fname_nd, "pop_node")
        );
    case 'r':
        return (
            langdef_nd_match_str(fname_nd, "rename_collection") ||
            langdef_nd_match_str(fname_nd, "rename_user") ||
            langdef_nd_match_str(fname_nd, "replace_node") ||
            langdef_nd_match_str(fname_nd, "revoke")
        );
    case 's':
        return (
            langdef_nd_match_str(fname_nd, "set_password") ||
            langdef_nd_match_str(fname_nd, "set_quota")
        );
    }
    return false;
}

static int query__node_db_unpack(
        const char * ebad,
        ti_query_t * query,
        uint16_t pkg_id,
        const uchar * data,
        size_t n,
        ex_t * e)
{
    assert (e->nr == 0);

    qp_unpacker_t unpacker;
    qp_obj_t key, val;

    query->pkg_id = pkg_id;

    qp_unpacker_init2(&unpacker, data, n, 0);

    if (!qp_is_map(qp_next(&unpacker, NULL)))
    {
        ex_set(e, EX_BAD_DATA, ebad);
        goto finish;
    }

    while(qp_is_raw(qp_next(&unpacker, &key)))
    {
        if (qp_is_raw_equal_str(&key, "query"))
        {
            if (query->querystr || !qp_is_raw(qp_next(&unpacker, &val)))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }

            query->querystr = qpx_obj_raw_to_str(&val);
            if (!query->querystr)
            {
                ex_set_alloc(e);
                goto finish;
            }

            continue;
        }

        if (qp_is_raw_equal_str(&key, "all"))
        {
            if (!qp_is_bool(qp_next(&unpacker, &val)))
            {
                ex_set(e, EX_BAD_DATA, ebad);
                goto finish;
            }
            if (qp_is_true(val.tp))
                query->flags |= TI_QUERY_FLAG_ALL;
            continue;
        }

        log_debug(
                "unexpected `query` key in map: `%.*s`",
                key.len, (const char *) key.via.raw);
    }

    if (!query->querystr)
        ex_set(e, EX_BAD_DATA, ebad);

finish:
    return e->nr;
}
