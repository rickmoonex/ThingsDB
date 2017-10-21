/*
 * rql.c
 *
 *  Created on: Sep 29, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#include <assert.h>
#include <stdlib.h>
#include <rql/rql.h>
#include <rql/db.h>
#include <rql/user.h>
#include <rql/signals.h>
#include <util/fx.h>
#include <util/strx.h>
#include <util/qpx.h>
#include <util/lock.h>
#include <rql/misc.h>
#include <rql/event.h>
#include <rql/store.h>

const char * rql_name = "_";
const uint8_t rql_def_redundancy = 3;
const char * rql_fn = "rql.qp";
const int rql_fn_schema = 0;

static qp_packer_t * rql__pack(rql_t * rql);
static int rql__unpack(rql_t * rql, qp_res_t * res);
static void rql__close_handles(uv_handle_t * handle, void * arg);

rql_t * rql_create(void)
{
    rql_t * rql = (rql_t *) malloc(sizeof(rql_t));
    if (!rql) return NULL;

    rql->flags = 0;
    rql->redundancy = rql_def_redundancy;
    rql->fn = NULL;
    rql->node = NULL;

    rql->args = rql_args_new();
    rql->cfg = rql_cfg_new();
    rql->back = rql_back_create(rql);
    rql->front = rql_front_create(rql);
    rql->dbs = vec_new(0);
    rql->nodes = vec_new(0);
    rql->users = vec_new(0);
    rql->events = rql_events_create(rql);
    rql->access = vec_new(0);
    rql->maint = rql_maint_new(rql);

    if (!rql->args ||
        !rql->cfg ||
        !rql->back ||
        !rql->front ||
        !rql->dbs ||
        !rql->nodes ||
        !rql->users ||
        !rql->events ||
        !rql->access ||
        !rql->maint)
    {
        rql_destroy(rql);
        return NULL;
    }

    return rql;
}

void rql_destroy(rql_t * rql)
{
    if (!rql) return;

    free(rql->fn);
    free(rql->args);
    free(rql->cfg);
    free(rql->maint);
    rql_back_destroy(rql->back);
    rql_front_destroy(rql->front);
    rql_events_destroy(rql->events);
    vec_destroy(rql->dbs, (vec_destroy_cb) rql_db_drop);
    vec_destroy(rql->nodes, (vec_destroy_cb) rql_node_drop);
    vec_destroy(rql->users, (vec_destroy_cb) rql_user_drop);
    vec_destroy(rql->access, free);
    free(rql);
}

void rql_init_logger(rql_t * rql)
{
    int n;
    char lname[255];
    size_t len = strlen(rql->args->log_level);

#ifndef DEBUG
    /* force colors while debugging... */
    if (rql->args->log_colorized)
#endif
    {
        Logger.flags |= LOGGER_FLAG_COLORED;
    }

    for (n = 0; n < LOGGER_NUM_LEVELS; n++)
    {
        strcpy(lname, LOGGER_LEVEL_NAMES[n]);
        strx_lower_case(lname);
        if (strlen(lname) == len && strcmp(rql->args->log_level, lname) == 0)
        {
            logger_init(stdout, n);
            return;
        }
    }
    assert (0);
}

int rql_init_fn(rql_t * rql)
{
    rql->fn = strx_cat(rql->cfg->rql_path, rql_fn);
    return (rql->fn) ? 0 : -1;
}

int rql_build(rql_t * rql)
{
    ex_ptr(e);
    int rc = -1;
    rql_event_t * event = NULL;
    qp_packer_t * packer = rql_misc_pack_init_event_request();
    if (!packer) return -1;

    rql->events->commit_id = 0;
    rql->events->next_id = 0;
    rql->events->obj_id = 0;

    rql->node = rql_node_create(0, rql->cfg->addr, rql->cfg->port);
    if (!rql->node || vec_push(&rql->nodes, rql->node)) goto stop;

    event = rql_event_create(rql->events);
    if (!event) goto stop;

    event->id = event->events->next_id;

    if (rql_save(rql)) goto stop;

    rql_event_raw(event, packer->buffer, packer->len, e);
    if (e->errnr)
    {
        log_critical(ex_log(e));
        goto stop;
    }

    if (rql_event_run(event) != 2 || rql_store(rql)) goto stop;

    rc = 0;

stop:

    if (rc)
    {
        fx_rmdir(rql->cfg->rql_path);
        mkdir(rql->cfg->rql_path, 0700);  /* no error checking required */
        rql_node_drop(rql->node);
        rql->node = NULL;
        vec_pop(rql->nodes);
    }
    qp_packer_destroy(packer);
    rql_event_destroy(event);
    return rc;
}

int rql_read(rql_t * rql)
{
    int rc;
    ssize_t n;
    unsigned char * data = fx_read(rql->fn, &n);
    if (!data) return -1;

    qp_unpacker_t unpacker;
    qp_unpacker_init(&unpacker, data, (size_t) n);
    qp_res_t * res = qp_unpacker_res(&unpacker, &rc);
    free(data);
    if (rc)
    {
        log_critical(qp_strerror(rc));
        return -1;
    }
    rc = rql__unpack(rql, res);
    qp_res_destroy(res);

    return rc;
}

int rql_run(rql_t * rql)
{
    uv_loop_init(&rql->loop);

    if (rql_events_init(rql->events) ||
        rql_signals_init(rql) ||
        rql_maint_start(rql->maint) ||
        rql_back_listen(rql->back) ||
        rql_front_listen(rql->front))
    {
        rql_term(SIGTERM);
    }

    rql->node->status = RQL_NODE_STAT_READY;

    uv_run(&rql->loop, UV_RUN_DEFAULT);

    uv_walk(&rql->loop, rql__close_handles, rql);

    uv_run(&rql->loop, UV_RUN_DEFAULT);

    uv_loop_close(&rql->loop);

    return 0;
}

int rql_save(rql_t * rql)
{
    qp_packer_t * packer = rql__pack(rql);
    if (!packer) return -1;

    int rc = fx_write(rql->fn, packer->buffer, packer->len);
    if (rc) log_error("failed to write file: '%s'", rql->fn);
    qp_packer_destroy(packer);
    return rc;
}

int rql_lock(rql_t * rql)
{
    lock_t rc = lock_lock(rql->cfg->rql_path, LOCK_FLAG_OVERWRITE);

    switch (rc)
    {
    case LOCK_IS_LOCKED_ERR:
    case LOCK_PROCESS_NAME_ERR:
    case LOCK_WRITE_ERR:
    case LOCK_READ_ERR:
    case LOCK_MEM_ALLOC_ERR:
        log_error("%s (%s)", lock_str(rc), rql->cfg->rql_path);
        return -1;
    case LOCK_NEW:
        log_info("%s (%s)", lock_str(rc), rql->cfg->rql_path);
        break;
    case LOCK_OVERWRITE:
        log_warning("%s (%s)", lock_str(rc), rql->cfg->rql_path);
        break;
    default:
        break;
    }
    return 0;
}

int rql_unlock(rql_t * rql)
{
    lock_t rc = lock_unlock(rql->cfg->rql_path);
    if (rc != LOCK_REMOVED)
    {
        log_error(lock_str(rc));
        return -1;
    }
    return 0;
}



static int rql__unpack(rql_t * rql, qp_res_t * res)
{
    qp_res_t * schema, * redundancy, * node, * nodes;

    if (res->tp != QP_RES_MAP ||
        !(schema = qpx_map_get(res->via.map, "schema")) ||
        !(redundancy = qpx_map_get(res->via.map, "redundancy")) ||
        !(node = qpx_map_get(res->via.map, "node")) ||
        !(nodes = qpx_map_get(res->via.map, "nodes")) ||
        schema->tp != QP_RES_INT64 ||
        schema->via.int64 != rql_fn_schema ||
        redundancy->tp != QP_RES_INT64 ||
        node->tp != QP_RES_INT64 ||
        nodes->tp != QP_RES_ARRAY ||
        redundancy->via.int64 < 1 ||
        redundancy->via.int64 > 64 ||
        node->via.int64 < 0) goto failed;

    rql->redundancy = (uint8_t) redundancy->via.int64;

    for (uint32_t i = 0; i < nodes->via.array->n; i++)
    {
        qp_res_t * itm = nodes->via.array->values + i;
        qp_res_t * addr, * port;
        if (itm->tp != QP_RES_ARRAY ||
            itm->via.array->n != 2 ||
            !(addr = itm->via.array->values) ||
            !(port = itm->via.array->values + 1) ||
            addr->tp != QP_RES_STR ||
            port->tp != QP_RES_INT64 ||
            port->via.int64 < 1 ||
            port->via.int64 > 65535) goto failed;

        rql_node_t * node = rql_node_create(
                (uint8_t) i,
                addr->via.str,
                (uint16_t) port->via.int64);

        if (!node || vec_push(&rql->nodes, node)) goto failed;
    }

    if (node->via.int64 >= rql->nodes->n) goto failed;

    rql->node = (rql_node_t *) vec_get(rql->nodes, (uint32_t) node->via.int64);

    return 0;

failed:
    LOGC("Here...");
    for (vec_each(rql->nodes, rql_node_t, node))
    {
        rql_node_drop(node);
    }
    rql->node = NULL;
    return -1;
}

static qp_packer_t * rql__pack(rql_t * rql)
{
    qp_packer_t * packer = qp_packer_create(1024);
    if (!packer) return NULL;
    if (qp_add_map(&packer)) goto failed;

    /* schema */
    if (qp_add_raw(packer, "schema", 6) ||
        qp_add_int64(packer, rql_fn_schema)) goto failed;

    /* redundancy */
    if (qp_add_raw(packer, "redundancy", 10) ||
        qp_add_int64(packer, (int64_t) rql->redundancy)) goto failed;

    /* node */
    if (qp_add_raw(packer, "node", 4) ||
        qp_add_int64(packer, (int64_t) rql->node->id)) goto failed;

    /* nodes */
    if (qp_add_raw(packer, "nodes", 5) || qp_add_array(&packer)) goto failed;
    for (uint32_t i = 0; i < rql->nodes->n; i++)
    {
        rql_node_t * node = (rql_node_t *) vec_get(rql->nodes, i);
        if (qp_add_array(&packer) ||
            qp_add_raw(packer, node->addr, strlen(node->addr)) ||
            qp_add_int64(packer, (int64_t) node->port) ||
            qp_close_array(packer)) goto failed;

    }
    if (qp_close_array(packer) || qp_close_map(packer)) goto failed;

    return packer;

failed:
    qp_packer_destroy(packer);
    return NULL;
}

static void rql__close_handles(uv_handle_t * handle, void * arg)
{
    rql_t * rql = (rql_t *) arg;
    (void)(rql);

    if (uv_is_closing(handle)) return;

    switch (handle->type)
    {
    case UV_ASYNC:
    case UV_SIGNAL:
        uv_close(handle, NULL);
        break;
    case UV_TCP:
        rql_sock_close((rql_sock_t *) handle->data);
        break;
    default:
        log_error("unexpected handle type: %d", handle->type);
    }
}
