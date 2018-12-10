/*
 * node.c
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ti.h>
#include <ti/lookup.h>
#include <ti/node.h>
#include <ti/nodes.h>
#include <ti/proto.h>
#include <ti/req.h>
#include <ti/tcp.h>
#include <ti/version.h>
#include <ti/write.h>
#include <util/logger.h>
#include <util/qpx.h>
#include <util/cryptx.h>

static void node__on_connect(uv_connect_t * req, int status);
static void node__on_connect_req(ti_req_t * req, ex_enum status);

/*
 * Nodes are created ti_nodes_new_node() to ensure a correct id
 * is generated for each node.
 */
ti_node_t * ti_node_create(
        uint8_t id,
        struct sockaddr_storage * addr,
        const char * secret)
{
    assert (strlen(secret) == CRYPTX_SZ - 1);

    ti_node_t * node = (ti_node_t *) malloc(sizeof(ti_node_t));
    if (!node)
        return NULL;

    node->ref = 1;
    node->flags = 0;
    node->cevid = 0;
    node->sevid = 0;
    node->next_thing_id = 0;
    node->id = id;
    node->stream = NULL;
    node->status = TI_NODE_STAT_OFFLINE;
    node->addr = *addr;
    node->next_retry = 0;
    node->retry_counter = 0;
    memcpy(&node->secret, secret, CRYPTX_SZ);

    return node;
}

void ti_node_drop(ti_node_t * node)
{
    if (node && !--node->ref)
    {
        ti_stream_drop(node->stream);
        free(node);
    }
}

const char * ti_node_name(ti_node_t * node)
{
    return ti()->node == node ? ti_name() : ti_stream_name(node->stream);
}

const char * ti_node_status_str(ti_node_status_t status)
{
    switch (status)
    {
    case TI_NODE_STAT_OFFLINE:          return "OFFLINE";
    case TI_NODE_STAT_CONNECTING:       return "CONNECTING";
    case TI_NODE_STAT_BUILDING:         return "BUILDING";
    case TI_NODE_STAT_SYNCHRONIZING:    return "SYNCHRONIZING";
    case TI_NODE_STAT_AWAY:             return "AWAY";
    case TI_NODE_STAT_AWAY_SOON:        return "AWAY_SOON";
    case TI_NODE_STAT_SHUTTING_DOWN:    return "SHUTTING_DOWN";
    case TI_NODE_STAT_READY:            return "READY";
    }
    return "UNKNOWN";
}

const char * ti_node_flags_str(ti_node_flags_t flags)
{
    if (flags & TI_NODE_FLAG_MIGRATING)
        return "MIGRATING";
    return "NO_FLAGS_SET";
}

int ti_node_connect(ti_node_t * node)
{
    assert (!node->stream);
    assert (node->status == TI_NODE_STAT_OFFLINE);

    uv_connect_t * req;

    node->stream = ti_stream_create(TI_STREAM_TCP_OUT_NODE, ti_nodes_pkg_cb);
    if (!node->stream)
        goto fail0;

    node->stream->via.node = ti_grab(node);

    req = malloc(sizeof(uv_connect_t));
    if (!req)
        goto fail0;

    log_debug("connecting to "TI_NODE_ID" using stream `%s`",
            node->id,
            ti_stream_name(node->stream));

    node->status = TI_NODE_STAT_CONNECTING;
    ti_incref(node->stream);

    if (uv_tcp_connect(
            req,
            (uv_tcp_t *) &node->stream->uvstream,
            (const struct sockaddr*) &node->addr,
            node__on_connect))
    {
        node->status = TI_NODE_STAT_OFFLINE;
        assert (node->stream->ref > 1);
        ti_decref(node->stream);
        goto fail0;
    }

    return 0;

fail0:
    ti_stream_drop(node->stream);
    return -1;
}

ti_node_t * ti_node_winner(ti_node_t * node_a, ti_node_t * node_b, uint64_t u)
{
    ti_node_t * min = node_a->id < node_b->id ? node_a : node_b;
    ti_node_t * max = node_a->id > node_b->id ? node_a : node_b;

    return ti_lookup_id_is_ordered(
            ti()->lookup,
            min->id,
            max->id,
            u) ? min : max;
}

static void node__on_connect(uv_connect_t * req, int status)
{
    int rc;
    qpx_packer_t * packer;
    ti_pkg_t * pkg;
    ti_stream_t * stream = req->handle->data;
    ti_node_t * node = stream->via.node, * ti_node = ti()->node;

    if (status)
    {
        log_error("connecting to "TI_NODE_ID" has failed (%s)",
                node->id,
                uv_strerror(status));
        goto failed;
    }

    if (node->stream != (ti_stream_t *) req->handle->data)
    {
        log_warning("connection to `%s` is established from the other side",
                ti_node_name(node));
        goto failed;
    }

    log_debug("node connection to `%s` created", ti_node_name(node));

    rc = uv_read_start(req->handle, ti_stream_alloc_buf, ti_stream_on_data);
    if (rc)
    {
        log_error("cannot read on socket: `%s`", uv_strerror(rc));
        goto failed;
    }

    stream->flags &= ~TI_STREAM_FLAG_CLOSED;

    packer = qpx_packer_create(192, 1);
    if (!packer)
    {
        log_error(EX_ALLOC_S);
        goto failed;
    }

    (void) qp_add_array(&packer);
    (void) qp_add_int64(packer, node->id);
    (void) qp_add_raw(packer, (const uchar *) node->secret, CRYPTX_SZ);
    (void) qp_add_int64(packer, ti_node->id);
    (void) qp_add_raw_from_str(packer, TI_VERSION);
    (void) qp_add_raw_from_str(packer, TI_MINIMAL_VERSION);
    (void) qp_add_int64(packer, ti_node->next_thing_id);
    (void) qp_add_int64(packer, ti_node->cevid);
    (void) qp_add_int64(packer, ti_node->sevid);
    (void) qp_add_int64(packer, ti_node->status);
    (void) qp_add_int64(packer, ti_node->flags);
    (void) qp_close_array(packer);

    pkg = qpx_packer_pkg(packer, TI_PROTO_NODE_REQ_CONNECT);

    if (ti_req_create(
            node->stream,
            pkg,
            TI_PROTO_NODE_REQ_CONNECT_TIMEOUT,
            node__on_connect_req,
            node))
    {
        log_error(EX_INTERNAL_S);
        goto failed;
    }

    goto done;

failed:
    ti_stream_drop(stream);
done:
    free(req);
}


static void node__on_connect_req(ti_req_t * req, ex_enum status)
{
    qp_unpacker_t unpacker;
    ti_pkg_t * pkg = req->pkg_res;
    ti_node_t * node = req->data;
    qp_obj_t qp_next_thing_id, qp_cevid, qp_sevid, qp_status, qp_flags;

    if (status)
        goto done;  /* logging is done */

    qp_unpacker_init2(&unpacker, pkg->data, pkg->n, 0);

    if (    !qp_is_array(qp_next(&unpacker, NULL)) ||
            !qp_is_int(qp_next(&unpacker, &qp_next_thing_id)) ||
            !qp_is_int(qp_next(&unpacker, &qp_cevid)) ||
            !qp_is_int(qp_next(&unpacker, &qp_sevid)) ||
            !qp_is_int(qp_next(&unpacker, &qp_status)) ||
            !qp_is_int(qp_next(&unpacker, &qp_flags)))
    {
        log_error(
                "invalid response from "TI_NODE_ID" (package type: `%s`)",
                node->id,
                ti_proto_str(pkg->tp));
        goto done;
    }

    node->next_thing_id = (uint64_t) qp_next_thing_id.via.int64;
    node->cevid = (uint64_t) qp_cevid.via.int64;
    node->sevid = (uint64_t) qp_sevid.via.int64;
    node->status = (uint8_t) qp_status.via.int64;
    node->flags = (uint8_t) qp_status.via.int64;

    /* reset the connection retry counters */
    node->next_retry = 0;
    node->retry_counter = 0;

    /* the flow is completed so we should have at least 2 references */
    assert (node->stream->ref > 1);

    goto done;

done:
    ti_stream_drop(node->stream);
    free(pkg);
    ti_req_destroy(req);
}
