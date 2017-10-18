/*
 * events.c
 *
 *  Created on: Oct 5, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#include <stdlib.h>
#include <qpack.h>
#include <rql/events.h>
#include <rql/event.h>

const int rql_events_fn_schema = 0;

static void rql__events_loop(uv_async_t * handle);

rql_events_t * rql_events_create(rql_t * rql)
{
    rql_events_t * events = (rql_events_t *) malloc(sizeof(rql_events_t));
    if (!events) return NULL;
    events->rql = rql;
    events->queue = queue_new(4);
    events->done = vec_new(4);
    return events;
}

void rql_events_destroy(rql_events_t * events)
{
    if (!events) return;
    queue_destroy(events->queue, (queue_destroy_cb) rql_event_destroy);
    vec_destroy(events->done, free);
    free(events);
}

int rql_events_init(rql_events_t * events)
{
    return uv_async_init(&events->rql->loop, &events->loop, rql__events_loop);
}

int rql_events_store(rql_events_t * events, const char * fn)
{
    int rc = -1;
    qp_packer_t * packer = qp_packer_create(64);
    if (!packer) return -1;

    if (qp_add_map(&packer)) goto stop;

    /* schema */
    if (qp_add_raw(packer, "schema", 6) ||
        qp_add_int64(packer, rql_events_fn_schema)) goto stop;

    if (qp_add_raw(packer, "commit_id", 9) ||
        qp_add_int64(packer, (int64_t) events->commit_id) ||
        qp_close_map(packer)) goto stop;

    rc = fx_write(fn, packer->buffer, packer->len);
    if (rc) log_error("failed to write file: '%s'", fn);

stop:
    qp_packer_destroy(packer);
    return rc;
}

int rql_events_restore(rql_events_t * events, const char * fn)
{
    int rc = -1;
    ssize_t n;
    unsigned char * data = fx_read(fn, &n);
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

    qp_res_t * schema, * commit_id;

    if (res->tp != QP_RES_MAP ||
        !(schema = qpx_map_get(res->via.map, "schema")) ||
        !(commit_id = qpx_map_get(res->via.map, "commit_id")) ||
        schema->tp != QP_RES_INT64 ||
        schema->via.int64 != rql_events_fn_schema ||
        commit_id->tp != QP_RES_INT64 ||
        commit_id->via.int64 < 0) goto stop;

    events->commit_id = (uint64_t) commit_id->via.int64;

    if (events->next_id < events->commit_id)
    {
        events->next_id = events->commit_id;
    }

    rc = 0;

stop:
    qp_res_destroy(res);
    return rc;
}

static void rql__events_loop(uv_async_t * handle)
{
    rql_events_t * events = (rql_events_t *) handle->data;

    while (events->queue->n)
    {
        rql_event_t * event = (rql_event_t *) queue_get(
                events->queue,
                events->queue->n - 1);
        if (event->id == events->commit_id)
        {
            rql_event_run(event);
            queue_pop(events->queue);
            rql_event_done(event);
            rql_event_destroy(event);
        }
    }
}