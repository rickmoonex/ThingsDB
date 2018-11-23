/*
 * raw.h
 */
#ifndef TI_RAW_H_
#define TI_RAW_H_

#include <qpack.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct ti_raw_s ti_raw_t;

ti_raw_t * ti_raw_create(const unsigned char * raw, size_t n);
void ti_raw_drop(ti_raw_t * raw);
ti_raw_t * ti_raw_from_packer(qp_packer_t * packer);
ti_raw_t * ti_raw_from_ti_string(const char * src, size_t n);
ti_raw_t * ti_raw_upper(ti_raw_t * raw);
ti_raw_t * ti_raw_lower(ti_raw_t * raw);
char * ti_raw_to_str(const ti_raw_t * raw);
int ti_raw_cmp(const ti_raw_t * a, const ti_raw_t * b);
int ti_raw_cmp_strn(const ti_raw_t * a, const char * s, size_t n);
ti_raw_t * ti_raw_cat(const ti_raw_t * a, const ti_raw_t * b);
ti_raw_t * ti_raw_cat_strn(const ti_raw_t * a, const char * s, size_t n);
ti_raw_t * ti_raw_icat_strn(const ti_raw_t * b, const char * s, size_t n);
ti_raw_t * ti_raw_cat_strn_strn(
        const char * as,
        size_t an,
        const char * bs,
        size_t bn);
static inline _Bool ti_raw_equal(const ti_raw_t * a, const ti_raw_t * b);
static inline _Bool ti_raw_equal_strn(
        const ti_raw_t * a,
        const char * s,
        size_t n);

struct ti_raw_s
{
    uint32_t ref;
    uint32_t n;
    unsigned char data[];
};

static inline _Bool ti_raw_equal(const ti_raw_t * a, const ti_raw_t * b)
{
    return a->n == b->n && !memcmp(a->data, b->data, a->n);
}

static inline _Bool ti_raw_equal_strn(
        const ti_raw_t * a,
        const char * s,
        size_t n)
{
    return a->n == n && !memcmp(a->data, s, n);
}

#endif /* TI_RAW_H_ */