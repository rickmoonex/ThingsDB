/*
 * event.c
 */
#include <assert.h>
#include <qpack.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ti.h>
#include <ti/db.h>
#include <ti/job.h>
#include <ti/event.h>
#include <ti/ex.h>
#include <ti/node.h>
#include <ti/proto.h>
#include <ti/task.h>
#include <util/omap.h>
#include <util/logger.h>
#include <util/qpx.h>


ti_event_t * ti_event_create(ti_event_tp_enum tp)
{
    ti_event_t * ev = malloc(sizeof(ti_event_t));
    if (!ev)
        return NULL;

    ev->status = TI_EVENT_STAT_NEW;
    ev->target = NULL;
    ev->tp = tp;
    ev->tasks = tp == TI_EVENT_TP_MASTER ? omap_create() : NULL;

    if (    (tp == TI_EVENT_TP_MASTER && !ev->tasks) ||
            clock_gettime(TI_CLOCK_MONOTONIC, &ev->time))
    {
        free(ev);
        return NULL;
    }

    return ev;
}

void ti_event_destroy(ti_event_t * ev)
{
    if (!ev)
        return;

    ti_db_drop(ev->target);

    if (ev->tp == TI_EVENT_TP_SLAVE)
        ti_node_drop(ev->via.node);

    if (ev->tp == TI_EVENT_TP_EPKG)
        ti_epkg_drop(ev->via.epkg);

    omap_destroy(ev->tasks, (omap_destroy_cb) ti_task_destroy);

    free(ev);
}

/* (Log function)
 * Returns 0 when successful, or -1 in case of an error.
 * This function creates the event tasks.
 *
 *  { [0, 0]: {0: [ {'job':...} ] } }
 */
int ti_event_run(ti_event_t * ev)
{
    assert (ev->tp == TI_EVENT_TP_EPKG);
    assert (ev->via.epkg);
    assert (ev->via.epkg->event_id == ev->id);
    assert (ev->via.epkg->pkg->tp == TI_PROTO_NODE_EVENT);

    ti_pkg_t * pkg = ev->via.epkg->pkg;
    qp_unpacker_t unpacker;
    qp_obj_t qp_target, thing_or_map;
    uint64_t target_id;

    qp_unpacker_init(&unpacker, pkg->data, pkg->n);

    if (    !qp_is_map(qp_next(&unpacker, NULL)) ||
            !qp_is_array(qp_next(&unpacker, NULL)) ||       /* fixed size 2 */
            !qp_is_int(qp_next(&unpacker, NULL)) ||         /* event_id     */
            !qp_is_int(qp_next(&unpacker, &qp_target)) ||   /* target       */
            !qp_is_map(qp_next(&unpacker, NULL)))           /* map with
                                                               thing_id:task */
    {
        log_critical("invalid or corrupt event: `"PRIu64"`", ev->id);
        return -1;
    }

    target_id = (uint64_t) qp_target.via.int64;
    if (!target_id)
        ev->target = NULL;      /* target 0 is root */
    else
    {
        ev->target = ti_dbs_get_by_id(target_id);
        if (!ev->target)
        {
            log_critical(
                    "target `%"PRIu64"` for event `%"PRIu64"` is not found",
                    target_id, ev->id);
            return -1;
        }
    }

    qp_next(&unpacker, &thing_or_map);

    while (qp_is_int(thing_or_map.tp) && qp_is_array(qp_next(&unpacker, NULL)))
    {
        ti_thing_t * thing;
        uint64_t thing_id = (uint64_t) thing_or_map.via.int64;

        thing = ev->target == NULL
                ? ti()->thing0
                : ti_db_thing_by_id(ev->target, thing_id);

        if (!thing)
        {
            assert (ev->target);

            log_critical(
                    "thing "TI_THING_ID" in event `%"PRIu64"` "
                    "is not found in database `%.*s`",
                    thing_id, ev->id,
                    (int) ev->target->name->n,
                    (const char *) ev->target->name->data);

            return -1;
        }

        while (qp_is_map(qp_next(&unpacker, &thing_or_map)))
            if (ti_job_run(ev->target, thing, &unpacker))
                if (ev->target)
                    log_critical(
                            "job for thing "TI_THING_ID" in "
                            "event `%"PRIu64"` for database `%.*s` failed",
                            thing_id, ev->id,
                            (int) ev->target->name->n,
                            (const char *) ev->target->name->data);

        if (qp_is_close(thing_or_map.tp))
            qp_next(&unpacker, &thing_or_map);
    }

    return 0;
}


//void ti_event_cancel(ti_event_t * ev)
//{
//    ti_rpkg_t * rpkg;
//    ti_pkg_t * pkg;
//    vec_t * vec_nodes = ti()->nodes->vec;
//    qpx_packer_t * packer = qpx_packer_create(9, 0);
//    if (!packer)
//    {
//        log_error(EX_ALLOC_S);
//        return;
//    }
//
//    (void) qp_add_int64(packer, ev->id);
//
//    pkg = qpx_packer_pkg(packer, TI_PROTO_NODE_EVENT_CANCEL);
//    rpkg = ti_rpkg_create(pkg);
//    if (!rpkg)
//    {
//        free(pkg);
//        log_error(EX_ALLOC_S);
//        return;
//    }
//
//    for (vec_each(vec_nodes, ti_node_t, node))
//    {
//        if (node == ti()->node ||
//            node->status <= TI_NODE_STAT_CONNECTING ||
//            ti_stream_is_closed(node->stream))
//            continue;
//
//        if (ti_stream_write_rpkg(node->stream, rpkg))
//            log_error(EX_INTERNAL_S);
//    }
//    ti_rpkg_drop(rpkg);
//}
