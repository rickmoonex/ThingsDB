/*
 * ti/syncer.h
 */
#ifndef TI_SYNCER_H_
#define TI_SYNCER_H_

typedef struct ti_syncer_s ti_syncer_t;

#include <ti/pkg.h>
#include <ti/ex.h>
#include <ti/user.h>
#include <ti/stream.h>

ti_syncer_t * ti_syncer_create(
        ti_stream_t * stream,
        uint64_t start,
        uint64_t until);
static inline void ti_syncer_destroy(ti_syncer_t * syncer);

/* extends ti_watch_t */
struct ti_syncer_s
{
    ti_stream_t * stream;       /* weak reference */
    uint64_t start;             /* first required event */
    uint64_t until;             /* until this event (exclusive and
                                   may be 0 in case we need until the end)
                                 */
};

static inline void ti_syncer_destroy(ti_syncer_t * syncer)
{
    free(syncer);
}

#endif  /* TI_SYNCER_H_ */
