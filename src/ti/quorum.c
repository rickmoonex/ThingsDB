/*
 * ti/quorum.c
 */
#include <assert.h>
#include <stdlib.h>
#include <ti.h>
#include <ti/nodes.h>
#include <ti/quorum.h>
#include <ti/proto.h>

ti_quorum_t * ti_quorum_new(ti_quorum_cb cb, void * data)
{
    assert (ti()->nodes->vec->n);

    ti_quorum_t * quorum;
    uint8_t nn = ti()->nodes->vec->n;
    uint8_t sz = nn - 1;

    quorum = malloc(sizeof(ti_quorum_t) + sz * sizeof(ti_req_t *));
    if (!quorum)
        return NULL;

    quorum->id = 0;
    quorum->rnode_id = -1;
    quorum->n = 0;
    quorum->accepted = 0;
    quorum->sz = sz;
    quorum->nn = nn;
    quorum->quorum = ti_nodes_quorum();
    quorum->data = data;
    quorum->cb_ = cb;

    return quorum;
}

void ti_quorum_go(ti_quorum_t * quorum)
{
    assert (quorum->id);  /* call ti_quorum_set_id() for id > 0 */

    if (quorum->cb_)
    {
        uint8_t reject;
        if (quorum->accepted == quorum->quorum)
        {
            quorum->cb_(quorum->data, true);
            quorum->cb_ = NULL;
            goto done;
        }

        reject = (uint8_t) (quorum->nn + 1) / 2;
        if (reject == quorum->n - quorum->accepted)
        {
            LOGC("Reject: %d", accept);
            quorum->cb_(quorum->data, false);
            quorum->cb_ = NULL;
        }
    }

done:
    if (quorum->n == quorum->sz)
    {
        _Bool accept = false;

        if (quorum->cb_)
        {
            if (!(quorum->nn & 1) && quorum->accepted == quorum->quorum - 1)
            {
                ti_node_t * node = ti_nodes_node_by_id(quorum->rnode_id);
                ti_node_t * ti_node = ti()->node;
                accept =  (
                        node &&
                        node != ti_node &&
                        ti_node_winner(ti_node, node, quorum->id) == ti_node
                );
            }
            LOGC("Accept: %d", accept);
            quorum->cb_(quorum->data, accept);
        }

        for (size_t i = 0; i < quorum->sz; ++i)
            ti_req_destroy(quorum->requests[i]);

        free(quorum);
    }
}

void ti_quorum_req_cb(ti_req_t * req, ex_enum status)
{
    ti_quorum_t * quorum = req->data;
    quorum->requests[quorum->n++] = req;
    if (status == 0)
    {
        switch (req->pkg_res->tp)
        {
        case TI_PROTO_NODE_RES_EVENT_ID:
        case TI_PROTO_NODE_RES_AWAY:
            ++quorum->accepted;
            break;
        case TI_PROTO_NODE_ERR_EVENT_ID:
        case TI_PROTO_NODE_ERR_AWAY:
            if (req->pkg_res->n == sizeof(uint8_t))
            {
                uint8_t * node_id = (uint8_t *) req->pkg_res->data;
                quorum->rnode_id = *node_id;
            }
            break;
        default:
            ti_pkg_log(req->pkg_res);
        }
    }
    ti_quorum_go(quorum);
}
