/*
 * ti/counters.h
 */
#ifndef TI_COUNTERS_H_
#define TI_COUNTERS_H_

typedef struct ti_counters_s ti_counters_t;

extern ti_counters_t counters_;

#include <inttypes.h>
#include <sys/time.h>
#include <ti/val.h>
#include <util/mpack.h>

int ti_counters_create(void);
void ti_counters_destroy(void);
void ti_counters_reset(void);
double ti_counters_upd_commit_change(struct timespec * start);
double ti_counters_upd_success_query(struct timespec * start);
int ti_counters_to_pk(msgpack_packer * pk);
ti_val_t * ti_counters_as_mpval(void);

struct ti_counters_s
{
    uint64_t started_at;            /* time when counters were reset */
    uint64_t queries_success;       /* node queries where this node acted as
                                       the master node and the query was
                                       successful finished
                                    */
    uint64_t queries_with_error;    /* node queries where this node acted as
                                       the master node but the query has
                                       returned with an error
                                    */
    uint64_t tasks_success;         /* number of processed tasks (success) */
    uint64_t tasks_with_error;      /* number of processed tasks (error) */
    uint64_t changes_with_gap;      /* changes which are committed but at least
                                       one change id was skipped
                                    */
    uint64_t changes_skipped;       /* changes which cannot be committed because
                                       a change with a higher id is already
                                       been committed
                                    */
    uint64_t changes_failed;        /* changes failed which should be committed
                                       from CPKG, this is a critical counter
                                       since the CPKG changes should proceed
                                       without errors
                                    */
    uint64_t changes_killed;        /* changes killed because it took too long
                                       before the change got the READY status.
                                       these changes may later be received
                                       and still being committed or skipped
                                    */
    uint64_t changes_committed;     /* changes committed */
    uint64_t quorum_lost;           /* number of times a quorum was not reached
                                       for requesting an change id
                                    */
    uint64_t changes_unaligned;     /* number of times a change cannot be
                                       pushed to the end of the queue because
                                       a higher change id is already queued
                                    */
    uint64_t largest_result_size;   /* largest result size in bytes */
    uint64_t queries_from_cache;    /* number of queries which are loaded from
                                       cache.
                                    */
    /*
     * Both `garbage_collected` and `wasted_cache` may be accessed by multiple
     * threads at equal times.
     */
    uint64_t garbage_collected;    /* total garbage collected */
    uint64_t wasted_cache;         /* number of cached queries which are
                                       removed from the cache before the cache
                                       was ever used. Basically this count the
                                       useless caching.
                                    */
    double longest_query_duration;  /* longest duration it took for a
                                       successful query to process (in seconds)
                                    */
    double longest_change_duration; /* longest duration it took for a change
                                       to complete (in seconds)
                                    */
    double total_query_duration;    /* can be used to calculate the average
                                       total_query_duration / queries_success
                                        (in seconds)
                                    */
    double total_change_duration;   /* can be used to calculate the average
                                       total_change_duration / changes_committed
                                        (in seconds)
                                    */
};

#ifdef NATOMIC
#define ti_counters_garbage_collected() \
    (counters_.garbage_collected)
#define ti_counters_add_garbage_collected(n__) \
    (counters_.garbage_collected += n__)
#define ti_counters_zero_garbage_collected() \
    (counters_.garbage_collected =  0)

#define ti_counters_wasted_cache() \
    (counters_.wasted_cache)
#define ti_counters_inc_wasted_cache() \
    (counters_.wasted_cache += 1)
#define ti_counters_zero_wasted_cache() \
    (counters_.wasted_cache = 0)
#else
#define ti_counters_garbage_collected() \
    (__atomic_load_n(&counters_.garbage_collected, __ATOMIC_SEQ_CST))
#define ti_counters_add_garbage_collected(n__) \
    (__atomic_add_fetch(&counters_.garbage_collected, n__, __ATOMIC_SEQ_CST))
#define ti_counters_zero_garbage_collected() \
    (__atomic_store_n(&counters_.garbage_collected, 0, __ATOMIC_SEQ_CST))

#define ti_counters_wasted_cache() \
    (__atomic_load_n(&counters_.wasted_cache, __ATOMIC_SEQ_CST))
#define ti_counters_inc_wasted_cache() \
    (__atomic_add_fetch(&counters_.wasted_cache, 1, __ATOMIC_SEQ_CST))
#define ti_counters_zero_wasted_cache() \
    (__atomic_store_n(&counters_.wasted_cache, 0, __ATOMIC_SEQ_CST))
#endif  /* NATOMIC */

#endif  /* TI_COUNTERS_H_ */
