/*
 * quota.h
 */
#ifndef TI_QUOTA_H_
#define TI_QUOTA_H_

#include <stddef.h>
#include <stdlib.h>
#include <ti/ex.h>
#include <tiinc.h>
#include <qpack.h>

typedef struct ti_quota_s ti_quota_t;

#define TI_QUOTA_NOT_SET SIZE_MAX

typedef enum
{
    QUOTA_THINGS        =1,
    QUOTA_PROPS         =2,
    QUOTA_ARRAY_SIZE    =3,
    QUOTA_RAW_SIZE      =4,
} ti_quota_enum_t;

struct ti_quota_s
{
    size_t max_things;
    size_t max_props;           /* max props per thing */
    size_t max_array_size;
    size_t max_raw_size;
};

ti_quota_t * ti_quota_create(void);
ti_quota_enum_t ti_qouta_tp_from_strn(const char * str, size_t n, ex_t * e);
int ti_quota_val_to_packer(qp_packer_t * packer, size_t quota);
inline static void ti_quota_destroy(ti_quota_t * quota);
inline static _Bool ti_quota_things(ti_quota_t * quota, size_t n, ex_t * e);
inline static _Bool ti_quota_isset(ti_quota_t * quota);

inline static void ti_quota_destroy(ti_quota_t * quota)
{
    free(quota);
}

inline static _Bool ti_quota_things(ti_quota_t * quota, size_t n, ex_t * e)
{
    if (n >= quota->max_things)
    {
        ex_set(e, EX_MAX_QUOTA,
                "maximum things quota of %zu has been reached"
                TI_SEE_DOC("#quotas"), quota->max_things);
        return true;
    }
    return false;
}

inline static _Bool ti_quota_isset(ti_quota_t * quota)
{
    return (
            quota->max_things != TI_QUOTA_NOT_SET ||
            quota->max_props != TI_QUOTA_NOT_SET ||
            quota->max_array_size != TI_QUOTA_NOT_SET ||
            quota->max_raw_size != TI_QUOTA_NOT_SET
    );
}

#endif  /* TI_QUOTA_H_ */
