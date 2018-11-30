/*
 * ti/query.h
 */
#ifndef TI_QUERY_H_
#define TI_QUERY_H_

enum
{
    TI_QUERY_FLAG_EVENT=1<<0
};

typedef struct ti_query_s ti_query_t;

#include <cleri/cleri.h>
#include <ti/db.h>
#include <ti/event.h>
#include <ti/ex.h>
#include <ti/pkg.h>
#include <ti/raw.h>
#include <ti/stream.h>
#include <util/omap.h>


ti_query_t * ti_query_create(ti_stream_t * stream);
void ti_query_destroy(ti_query_t * query);
int ti_query_unpack(ti_query_t * query, ti_pkg_t * pkg, ex_t * e);
int ti_query_parse(ti_query_t * query, ex_t * e);
int ti_query_investigate(ti_query_t * query, ex_t * e);
void ti_query_run(ti_query_t * query);
void ti_query_send(ti_query_t * query, ex_t * e);
static inline _Bool ti_query_will_update(ti_query_t * query);

struct ti_query_s
{
    uint32_t nd_cache_count;    /* count while investigate */
    uint8_t flags;
    uint64_t pkg_id;
    ti_db_t * target;           /* target NULL means root */
    char * querystr;            /* 0 terminated query string */
    cleri_parse_t * parseres;
    ti_stream_t * stream;       /* with reference */
    vec_t * blobs;              /* ti_raw_t */
    vec_t * statements;         /* ti_res_t or ti_root_t for each statement */
    ti_event_t * ev;            /* with reference, only when an event is
                                   required
                                */
    vec_t * nd_cache;           /* cleri_node_t, for node cache cleanup */
    omap_t * collect;           /* contains a vec_t with attributes
                                   to collect (or type ti_name_t),
                                   and the key is the thing id.
                                */

};

static inline _Bool ti_query_will_update(ti_query_t * query)
{
    return query->flags & TI_QUERY_FLAG_EVENT;
}

#endif /* TI_QUERY_H_ */
