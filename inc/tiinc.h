/*
 * tiinc.h
 */
#ifndef TIINC_H_
#define TIINC_H_

#define TI_URL "https://thinkdb.net"
#define TI_DOCS TI_URL"/docs/"

#define TI_DEFAULT_CLIENT_PORT 9200
#define TI_DEFAULT_NODE_PORT 9220

#define TI_COLLECTION_ID "`collection:%"PRIu64"`"
#define TI_EVENT_ID "`event:%"PRIu64"`"
#define TI_NODE_ID "`node:%u`"
#define TI_THING_ID "`#%"PRIu64"`"
#define TI_USER_ID "`user:%"PRIu64"`"

/*
 * File name schema to check version info on created files.
 */
#define TI_FN_SCHEMA 0

/*
 * If a system has a WORDSIZE of 64 bits, we can take advantage of storing
 * some data in void pointers.
 */
#define TI_USE_VOID_POINTER __WORDSIZE == 64

typedef unsigned char uchar;

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#define TI_CLOCK_MONOTONIC CLOCK_MONOTONIC_RAW

#define ti_grab(x) ((x) && ++(x)->ref ? (x) : NULL)
#define ti_incref(x) (++(x)->ref)
#define ti_decref(x) (--(x)->ref)  /* use only when x->ref > 1 */

/* SUSv2 guarantees that "Host names are limited to 255 bytes,
 * excluding terminating null byte" */
#define TI_MAX_HOSTNAME_SZ 256

enum
{
    TI_FLAG_SIGNAL          =1<<0,
    TI_FLAG_STOP            =1<<1,
    TI_FLAG_INDEXING        =1<<2,
    TI_FLAG_LOCKED          =1<<3,
};


#endif  /* TIINC_H_ */
