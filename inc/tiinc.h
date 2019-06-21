/*
 * tiinc.h
 */
#ifndef TIINC_H_
#define TIINC_H_

#define TI_URL "https://thingsdb.github.io"
#define TI_DOC(__fn) TI_URL"/ThingsDocs/"__fn
#define TI_SEE_DOC(__fn) "; see "TI_DOC(__fn)

#define TI_DEFAULT_CLIENT_PORT 9200
#define TI_DEFAULT_NODE_PORT 9220

/* HTTP status port binding, 0=disabled, 1-65535=enabled */
#define TI_DEFAULT_HTTP_STATUS_PORT 0

/* Incremental events are stored until this threshold is reached */
#define TI_DEFAULT_THRESHOLD_FULL_STORAGE 1000UL

#define TI_COLLECTION_ID "`collection:%"PRIu64"`"
#define TI_EVENT_ID "`event:%"PRIu64"`"
#define TI_NODE_ID "`node:%u`"
#define TI_THING_ID "`#%"PRIu64"`"
#define TI_USER_ID "`user:%"PRIu64"`"
#define TI_SYNTAX "syntax v%u"
#define TI_SCOPE_NODE 1
#define TI_SCOPE_THINGSDB 0

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

typedef struct ti_s ti_t;
extern ti_t ti_;

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

typedef enum
{
    TI_FN_0,                /* unknown function */
    /* collection functions */
    TI_FN_ASSERT,
    TI_FN_BLOB,
    TI_FN_BOOL,
    TI_FN_DEL,
    TI_FN_ENDSWITH,
    TI_FN_FILTER,
    TI_FN_FIND,
    TI_FN_FINDINDEX,
    TI_FN_HAS,
    TI_FN_HASPROP,
    TI_FN_ID,
    TI_FN_INDEXOF,
    TI_FN_INT,
    TI_FN_ISARRAY,
    TI_FN_ISASCII,
    TI_FN_ISBOOL,
    TI_FN_ISFLOAT,
    TI_FN_ISINF,
    TI_FN_ISINT,
    TI_FN_ISLIST,
    TI_FN_ISNAN,
    TI_FN_ISNIL,
    TI_FN_ISRAW,
    TI_FN_ISSTR,
    TI_FN_ISTHING,
    TI_FN_ISTUPLE,
    TI_FN_ISUTF8,
    TI_FN_LEN,
    TI_FN_LOWER,
    TI_FN_MAP,
    TI_FN_NOW,
    TI_FN_POP,
    TI_FN_PUSH,
    TI_FN_REFS,
    TI_FN_REMOVE,
    TI_FN_RENAME,
    TI_FN_RET,
    TI_FN_SET,
    TI_FN_SPLICE,
    TI_FN_STARTSWITH,
    TI_FN_STR,
    TI_FN_T,
    TI_FN_TEST,
    TI_FN_TRY,
    TI_FN_UPPER,

    /* thingsdb functions */
    TI_FN_COLLECTION,
    TI_FN_COLLECTIONS,
    TI_FN_DEL_COLLECTION,
    TI_FN_DEL_USER,
    TI_FN_GRANT,
    TI_FN_NEW_COLLECTION,
    TI_FN_NEW_NODE,
    TI_FN_NEW_USER,
    TI_FN_POP_NODE,
    TI_FN_RENAME_COLLECTION,
    TI_FN_RENAME_USER,
    TI_FN_REPLACE_NODE,
    TI_FN_REVOKE,
    TI_FN_SET_PASSWORD,
    TI_FN_SET_QUOTA,
    TI_FN_USER,
    TI_FN_USERS,

    /* node functions */
    TI_FN_COUNTERS,
    TI_FN_NODE,
    TI_FN_NODES,
    TI_FN_RESET_COUNTERS,
    TI_FN_SET_LOGLEVEL,
    TI_FN_SET_ZONE,
    TI_FN_SHUTDOWN,
} ti_fn_enum_t;


#endif  /* TIINC_H_ */
