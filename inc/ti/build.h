/*
 * ti/build.h
 */
#ifndef TI_BUILD_H_
#define TI_BUILD_H_

enum
{
    TI_BUILD_WAITING,
    TI_BUILD_SET_NODE_ID,
    TI_BUILD_REQ_SETUP,
    TI_BUILD_READY,
};

typedef struct ti_build_s ti_build_t;

#include <ti/stream.h>
#include <util/omap.h>

int ti_build_create(void);
void ti_build_destroy(void);
int ti_build_set_node_id(uint8_t node_id);
int ti_build_setup(uint8_t node_id, ti_stream_t * stream);

struct ti_build_s
{
    uint8_t status;
    uint8_t node_id;            /* this node id */
    uint8_t req_node_id;        /* to this node id we have send the request */
    omap_t * streams;           /* key is node_id and value is a stream with
                                   reference
                                */
    ti_pkg_t * req_setup_pkg;
};

#endif  /* TI_BUILD_H */