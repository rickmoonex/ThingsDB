/*
 * thingsdb.c
 */
#include <assert.h>
#include <props.h>
#include <stdlib.h>
#include <thingsdb.h>
#include <ti/api.h>
#include <ti/db.h>
#include <ti/event.h>
#include <ti/misc.h>
#include <ti/signals.h>
#include <ti/store.h>
#include <ti/user.h>
#include <util/fx.h>
#include <util/strx.h>
#include <util/qpx.h>
#include <util/lock.h>

static const uint8_t thingsdb__def_redundancy = 3;
const char * thingsdb__fn = "thingsdb.qp";
const int thingsdb__fn_schema = 0;          /* thingsdb config */
const int thingsdb__fn_store_schema = 0;    /* store running db */

static uv_loop_t loop_;
static thingsdb_t thingsdb;

static qp_packer_t * thingsdb__pack(void);
static int thingsdb__unpack(qp_res_t * res);
static void thingsdb__close_handles(uv_handle_t * handle, void * arg);

int thingsdb_create(void)
{
    thingsdb.flags = 0;
    thingsdb.redundancy = thingsdb__def_redundancy;
    thingsdb.fn = NULL;
    thingsdb.node = NULL;
    thingsdb.lookup = NULL;
    thingsdb.args = ti_args_new();
    thingsdb.access = vec_new(0);
    thingsdb.maint = ti_maint_new();

    if (    !thingsdb_args_create() ||
            !thingsdb_cfg_create() ||
            !thingsdb_clients_create() ||
            !thingsdb_nodes_create() ||
            !thingsdb_props_create() ||
            !thingsdb_users_create() ||
            !thingsdb_dbs_create() ||
            !thingsdb_events_create() ||
            !thingsdb.args ||
            !thingsdb.cfg ||
            !thingsdb.access ||
            !thingsdb.maint)
    {
        thingsdb_destroy();
        return -1;
    }

    return 0;
}

void thingsdb_destroy(void)
{
    free(thingsdb.fn);
    free(thingsdb.maint);
    ti_lookup_destroy(thingsdb.lookup);
    thingsdb_args_destroy();
    thingsdb_cfg_destroy();
    thingsdb_clients_destroy();
    thingsdb_nodes_destroy();
    thingsdb_events_destroy();
    thingsdb_dbs_destroy();
    thingsdb_nodes_destroy();
    thingsdb_users_destroy();
    thingsdb_props_destroy();
    vec_destroy(thingsdb.access, free);
    memset(&thingsdb, 0, sizeof(thingsdb_t));
}

thingsdb_t * thingsdb_get(void)
{
    return &thingsdb;
}

void thingsdb_init_logger(void)
{
    int n;
    char lname[255];
    size_t len = strlen(thingsdb.args->log_level);

#ifdef NDEBUG
    /* force colors while debugging... */
    if (thingsdb.args->log_colorized)
#endif
    {
        Logger.flags |= LOGGER_FLAG_COLORED;
    }

    for (n = 0; n < LOGGER_NUM_LEVELS; n++)
    {
        strcpy(lname, LOGGER_LEVEL_NAMES[n]);
        strx_lower_case(lname);
        if (strlen(lname) == len && strcmp(thingsdb.args->log_level, lname) == 0)
        {
            logger_init(stdout, n);
            return;
        }
    }
    assert (0);
}

int thingsdb_init_fn(void)
{
    thingsdb.fn = strx_cat(thingsdb.cfg->store_path, thingsdb__fn);
    return (thingsdb.fn) ? 0 : -1;
}

int thingsdb_build(void)
{
    ex_t * e = ex_use();
    int rc = -1;
    ti_event_t * event = NULL;
    qp_packer_t * packer = ti_misc_init_query();
    if (!packer)
        return -1;

    thingsdb.events->commit_id = 0;
    thingsdb.events->next_id = 0;
    thingsdb.next_id_ = 1;

    thingsdb.node = ti_node_create(0, thingsdb.clients->tcp, thingsdb.cfg->port);
    if (!thingsdb.node || vec_push(&thingsdb.nodes, thingsdb.node))
        goto stop;

    event = ti_event_create(thingsdb.events);
    if (!event) goto stop;

    event->id = event->events->next_id;

    if (thingsdb_save()) goto stop;

    ti_event_raw(event, packer->buffer, packer->len, e);
    if (e->nr)
    {
        log_critical(e->msg);
        goto stop;
    }

    if (ti_event_run(event) != 2 || ti_store()) goto stop;

    rc = 0;

stop:
    if (rc)
    {
        fx_rmdir(thingsdb.cfg->ti_path);
        mkdir(thingsdb.cfg->ti_path, 0700);  /* no error checking required */
        ti_node_drop(thingsdb.node);
        thingsdb.node = NULL;
        vec_pop(thingsdb.nodes);
    }
    qp_packer_destroy(packer);
    ti_event_destroy(event);
    return rc;
}

int thingsdb_read(void)
{
    int rc;
    ssize_t n;
    unsigned char * data = fx_read(thingsdb.fn, &n);
    if (!data) return -1;

    qp_unpacker_t unpacker;
    qpx_unpacker_init(&unpacker, data, (size_t) n);
    qp_res_t * res = qp_unpacker_res(&unpacker, &rc);
    free(data);
    if (rc)
    {
        log_critical(qp_strerror(rc));
        return -1;
    }
    rc = thingsdb__unpack(res);
    qp_res_destroy(res);
    if (rc)
    {
        log_critical("unpacking has failed (%s)", thingsdb.fn);
        goto stop;
    }
    thingsdb.lookup = ti_lookup_create(
            thingsdb.nodes->n,
            thingsdb.redundancy,
            thingsdb.nodes);
    if (!thingsdb.lookup) return -1;

stop:
    return rc;
}

int thingsdb_run(void)
{
    uv_loop_init(&loop_);
    thingsdb.loop = &loop_;

    if (ti_events_init(thingsdb.events) ||
        ti_signals_init() ||
        ti_back_listen(thingsdb.back))
    {
        ti_term(SIGTERM);
    }

    if (thingsdb.node)
    {
        if (ti_front_listen(thingsdb.front) || ti_maint_start(thingsdb.maint))
        {
            ti_term(SIGTERM);
        }
        thingsdb.node->status = TI_NODE_STAT_READY;
    }

    uv_run(&thingsdb.loop, UV_RUN_DEFAULT);

    uv_walk(&thingsdb.loop, thingsdb__close_handles, NULL);

    uv_run(&thingsdb.loop, UV_RUN_DEFAULT);

    uv_loop_close(&thingsdb.loop);

    return 0;
}

int thingsdb_save(void)
{
    qp_packer_t * packer = thingsdb__pack();
    if (!packer) return -1;

    int rc = fx_write(thingsdb.fn, packer->buffer, packer->len);
    if (rc) log_error("failed to write file: '%s'", thingsdb.fn);
    qp_packer_destroy(packer);
    return rc;
}

int thingsdb_lock(void)
{
    lock_t rc = lock_lock(thingsdb.cfg->ti_path, LOCK_FLAG_OVERWRITE);

    switch (rc)
    {
    case LOCK_IS_LOCKED_ERR:
    case LOCK_PROCESS_NAME_ERR:
    case LOCK_WRITE_ERR:
    case LOCK_READ_ERR:
    case LOCK_MEM_ALLOC_ERR:
        log_error("%s (%s)", lock_str(rc), thingsdb.cfg->ti_path);
        return -1;
    case LOCK_NEW:
        log_info("%s (%s)", lock_str(rc), thingsdb.cfg->ti_path);
        break;
    case LOCK_OVERWRITE:
        log_warning("%s (%s)", lock_str(rc), thingsdb.cfg->ti_path);
        break;
    default:
        break;
    }
    thingsdb.flags |= THINGSDB_FLAG_LOCKED;
    return 0;
}

int thingsdb_unlock(void)
{
    if (thingsdb.flags & THINGSDB_FLAG_LOCKED)
    {
        lock_t rc = lock_unlock(thingsdb.cfg->ti_path);
        if (rc != LOCK_REMOVED)
        {
            log_error(lock_str(rc));
            return -1;
        }
    }
    return 0;
}

int thingsdb_store(const char * fn)
{
    int rc = -1;
    qp_packer_t * packer = qp_packer_create(64);
    if (!packer) return -1;

    if (qp_add_map(&packer) ||
        qp_add_raw_from_str(packer, "schema") ||
        qp_add_int64(packer, thingsdb__fn_store_schema) ||
        qp_add_raw_from_str(packer, "commit_id") ||
        qp_add_int64(packer, (int64_t) thingsdb.events->commit_id) ||
        qp_add_raw_from_str(packer, "next_id") ||
        qp_add_int64(packer, (int64_t) thingsdb.next_id_) ||
        qp_close_map(packer)) goto stop;

    rc = fx_write(fn, packer->buffer, packer->len);

stop:
    if (rc) log_error("failed to write file: '%s'", fn);
    qp_packer_destroy(packer);
    return rc;
}

int thingsdb_restore(const char * fn)
{
    int rcode, rc = -1;
    ssize_t n;
    unsigned char * data = fx_read(fn, &n);
    if (!data) return -1;

    qp_unpacker_t unpacker;
    qpx_unpacker_init(&unpacker, data, (size_t) n);
    qp_res_t * res = qp_unpacker_res(&unpacker, &rcode);
    free(data);

    if (rcode)
    {
        log_critical(qp_strerror(rcode));
        return -1;
    }

    qp_res_t * schema, * commit_id, * next_id;

    if (res->tp != QP_RES_MAP ||
        !(schema = qpx_map_get(res->via.map, "schema")) ||
        !(commit_id = qpx_map_get(res->via.map, "commit_id")) ||
        !(next_id = qpx_map_get(res->via.map, "next_id")) ||
        schema->tp != QP_RES_INT64 ||
        schema->via.int64 != thingsdb__fn_store_schema ||
        commit_id->tp != QP_RES_INT64 ||
        next_id->tp != QP_RES_INT64) goto stop;

    thingsdb.events->commit_id = (uint64_t) commit_id->via.int64;
    thingsdb.events->next_id = thingsdb.events->commit_id;
    thingsdb.next_id_ = (uint64_t) next_id->via.int64;

    rc = 0;

stop:
    if (rc) log_critical("failed to restore from file: '%s'", fn);
    qp_res_destroy(res);
    return rc;
}

uint64_t thingsdb_get_next_id(void)
{
    return thingsdb.next_id_++;
}

_Bool thingsdb_has_id(uint64_t id)
{
    return ti_node_has_id(thingsdb.node, thingsdb.lookup, id);
}

static qp_packer_t * thingsdb__pack(void)
{
    qp_packer_t * packer = qp_packer_create(1024);
    if (!packer) return NULL;
    if (qp_add_map(&packer)) goto failed;

    /* schema */
    if (qp_add_raw_from_str(packer, "schema") ||
        qp_add_int64(packer, thingsdb__fn_schema)) goto failed;

    /* redundancy */
    if (qp_add_raw_from_str(packer, "redundancy") ||
        qp_add_int64(packer, (int64_t) thingsdb.redundancy)) goto failed;

    /* node */
    if (qp_add_raw_from_str(packer, "node") ||
        qp_add_int64(packer, (int64_t) thingsdb.node->id)) goto failed;

    /* nodes */
    if (qp_add_raw_from_str(packer, "nodes") ||
        qp_add_array(&packer)) goto failed;
    for (uint32_t i = 0; i < thingsdb.nodes->n; i++)
    {
        ti_node_t * node = (ti_node_t *) vec_get(thingsdb.nodes, i);
        if (qp_add_array(&packer) ||
            qp_add_raw_from_str(packer, node->addr) ||
            qp_add_int64(packer, (int64_t) node->port) ||
            qp_close_array(packer)) goto failed;

    }
    if (qp_close_array(packer) || qp_close_map(packer)) goto failed;

    return packer;

failed:
    qp_packer_destroy(packer);
    return NULL;
}

static int thingsdb__unpack(qp_res_t * res)
{
    qp_res_t * schema, * redundancy, * node, * nodes;

    if (res->tp != QP_RES_MAP ||
        !(schema = qpx_map_get(res->via.map, "schema")) ||
        !(redundancy = qpx_map_get(res->via.map, "redundancy")) ||
        !(node = qpx_map_get(res->via.map, "node")) ||
        !(nodes = qpx_map_get(res->via.map, "nodes")) ||
        schema->tp != QP_RES_INT64 ||
        schema->via.int64 != thingsdb__fn_schema ||
        redundancy->tp != QP_RES_INT64 ||
        node->tp != QP_RES_INT64 ||
        nodes->tp != QP_RES_ARRAY ||
        redundancy->via.int64 < 1 ||
        redundancy->via.int64 > 64 ||
        node->via.int64 < 0) goto failed;

    thingsdb.redundancy = (uint8_t) redundancy->via.int64;

    for (uint32_t i = 0; i < nodes->via.array->n; i++)
    {
        qp_res_t * itm = nodes->via.array->values + i;
        qp_res_t * addr, * port;
        if (itm->tp != QP_RES_ARRAY ||
            itm->via.array->n != 2 ||
            !(addr = itm->via.array->values) ||
            !(port = itm->via.array->values + 1) ||
            addr->tp != QP_RES_RAW ||
            port->tp != QP_RES_INT64 ||
            port->via.int64 < 1 ||
            port->via.int64 > 65535) goto failed;

        char * addrstr = ti_raw_to_str(addr->via.raw);
        if (!addrstr) goto failed;

        ti_node_t * node = ti_node_create(
                (uint8_t) i,
                addrstr,
                (uint16_t) port->via.int64);

        free(addrstr);
        if (!node || vec_push(&thingsdb.nodes, node)) goto failed;
    }

    if (node->via.int64 >= thingsdb.nodes->n) goto failed;

    thingsdb.node = (ti_node_t *) vec_get(thingsdb.nodes, (uint32_t) node->via.int64);

    return 0;

failed:
    for (vec_each(thingsdb.nodes, ti_node_t, node))
    {
        ti_node_drop(node);
    }
    thingsdb.node = NULL;
    return -1;
}

static void thingsdb__close_handles(uv_handle_t * handle, void * arg)
{
    if (uv_is_closing(handle)) return;

    switch (handle->type)
    {
    case UV_ASYNC:
    case UV_SIGNAL:
        uv_close(handle, NULL);
        break;
    case UV_TCP:
        ti_stream_close((ti_stream_t *) handle->data);
        break;
    default:
        log_error("unexpected handle type: %d", handle->type);
    }
}
