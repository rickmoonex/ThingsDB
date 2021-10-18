/*
 * ti/val.t.h
 */
#ifndef TI_VAL_T_H_
#define TI_VAL_T_H_

#define VAL__CAST_MAX 9223372036854775808.0

/*

## Return values

Type            | MessagePack | Examples
--------------- | ----------- | -------
TI_VAL_NIL      | MP_NIL      | `null`
TI_VAL_INT      | MP_INT      | `123` / `0`
TI_VAL_FLOAT    | MP_FLOAT    | `3.14`
TI_VAL_BOOL     | MP_BOOL     | `true` / `false`
TI_VAL_DATETIME | MP_STR      | `"2021-07-22T11:23:46Z"`
TI_VAL_NAME     | MP_STR      | `"some string value"`
TI_VAL_STR      | MP_STR      | `"some string value"`
TI_VAL_BYTES    | MP_BIN      | `b"some_bytes"` *(JSON requires encoding)*
TI_VAL_REGEX    | ?           | ?
TI_VAL_THING    | MP_OBJ      | `{"#": 123}` / `{"name": "some test"}`
TI_VAL_WRAP     | MP_OBJ      | `{"#": 123}` / `{"name": "some test"}`
TI_VAL_ROOM     | ?           | ?
TI_VAL_ARR      | MP_ARR      | `[...]` *(Array with values)*
TI_VAL_SET      | MP_ARR      | `[...]` *(Array with things)*
TI_VAL_ERROR    | ?           | ?
TI_VAL_MEMBER   | MP_X        | *Enumerator value*
TI_VAL_MPDATA   | MP_X        | *Packed data*
TI_VAL_CLOSURE  | ?           | ?
TI_VAL_FUTURE   | MP_ARR      | `[..]` *(Array with future result)*
TI_VAL_TEMPLATE | N.A.        | *Never returned to the client as template*

*/


/*
 * enum cache is not a real value type but used for stored closure to pre-cache
 *
 */
#define TI_VAL_ENUM_CACHE 255

#define TI_VAL_NIL_S        "nil"
#define TI_VAL_INT_S        "int"
#define TI_VAL_FLOAT_S      "float"
#define TI_VAL_BOOL_S       "bool"
#define TI_VAL_MPDATA_S     "mpdata"
#define TI_VAL_STR_S        "str"
#define TI_VAL_BYTES_S      "bytes"
#define TI_VAL_REGEX_S      "regex"
#define TI_VAL_THING_S      "thing"
#define TI_VAL_LIST_S       "list"
#define TI_VAL_TUPLE_S      "tuple"
#define TI_VAL_ROOM_S       "room"
#define TI_VAL_SET_S        "set"
#define TI_VAL_CLOSURE_S    "closure"
#define TI_VAL_ERROR_S      "error"
#define TI_VAL_DATETIME_S   "datetime"
#define TI_VAL_TIMEVAL_S    "timeval"
#define TI_VAL_FUTURE_S     "future"
#define TI_VAL_TASK_S       "task"


/* negative value is used for packing tasks */
#define TI_VAL_PACK_TASK -1
#define TI_VAL_PACK_FILE -2

#define TI_KIND_S_INSTANCE  "."
#define TI_KIND_S_THING     "#"
#define TI_KIND_S_SET       "$"
#define TI_KIND_S_ERROR     "!"
#define TI_KIND_S_WRAP      "&"
#define TI_KIND_S_MEMBER    "%"
#define TI_KIND_S_DATETIME  "'"
#define TI_KIND_S_TIMEVAL   "\""
#define TI_KIND_S_CLOSURE_OBSOLETE_   "/"
#define TI_KIND_S_REGEX_OBSOLETE_     "*"

typedef enum
{
    TI_VAL_NIL,
    TI_VAL_INT,
    TI_VAL_FLOAT,
    TI_VAL_BOOL,
    TI_VAL_DATETIME,
    TI_VAL_NAME,
    TI_VAL_STR,
    TI_VAL_BYTES,       /* MP,STR and BIN all use RAW as underlying type */
    TI_VAL_REGEX,
    TI_VAL_THING,       /* instance or object */
    TI_VAL_WRAP,
    TI_VAL_ROOM,
    TI_VAL_TASK,
    TI_VAL_ARR,         /* array, list or tuple */
    TI_VAL_SET,         /* set of things */
    TI_VAL_ERROR,
    TI_VAL_MEMBER,      /* enum member */

    TI_VAL_MPDATA,      /* msgpack data */
    TI_VAL_CLOSURE,
    /*
     * {
     *   "!": "closure",
     *   "code": "||
     */
    TI_VAL_FUTURE,      /* future */
    TI_VAL_TEMPLATE,    /* template to generate TI_VAL_STR
                           note that a template is never stored like a value,
                           rather it may build from either a query or a stored
                           closure; therefore template does not need to be
                           handled like all other value type. */
} ti_val_enum;

enum
{
    TI_VFLAG_LOCK            =1<<7,      /* thing or value in use;
                                            used to prevent illegal changes */
};

typedef enum
{
    /*
     * Reserved (may be implemented in the future):
     *   All specials are in the binary 0010xxxx range.
     *   + positive big type
     *   - negative big type
     */
    TI_KIND_C_INSTANCE  ='.',
    TI_KIND_C_THING     ='#',
    TI_KIND_C_SET       ='$',
    TI_KIND_C_ERROR     ='!',
    TI_KIND_C_WRAP      ='&',
    TI_KIND_C_MEMBER    ='%',
    TI_KIND_C_DATETIME  ='\'',
    TI_KIND_C_TIMEVAL   ='"',
    TI_KIND_C_CLOSURE_OBSOLETE_     ='/',
    TI_KIND_C_REGEX_OBSOLETE_       ='*',
} ti_val_kind;

typedef struct ti_val_s ti_val_t;

#include <inttypes.h>

struct ti_val_s
{
    uint32_t ref;
    uint8_t tp;
    uint8_t flags;
    uint16_t _pad16;
};

typedef (*ti_val_destroy_cb) (ti_val_t *);
/* TODO  typedef (*ti_val_to_pk_cb) (ti_val_t *, ti_vp_t *, int); */

typedef struct
{
    ti_val_destroy_cb destroy;
    _Bool allowed_as_vtask_arg;     /* allowed in the @thingsdb scope */
} ti_val_type_t;


static ti_val_type_t ti_val_type_props[20] = {
    /* TI_VAL_NIL */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_INT */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_FLOAT */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_BOOL */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_DATETIME */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_NAME */
    {
        .destroy = (ti_val_destroy_cb) ti_name_destroy,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_STR */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_BYTES */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_REGEX */
    {
        .destroy = (ti_val_destroy_cb) ti_regex_destroy,
        .allowed_as_vtask_arg = true,
    },
    /* TI_VAL_THING */
    {
        .destroy = (ti_val_destroy_cb) ti_thing_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_WRAP */
    {
        .destroy = (ti_val_destroy_cb) ti_wrap_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_ROOM */
    {
        .destroy = (ti_val_destroy_cb) ti_room_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_TASK */
    {
        .destroy = (ti_val_destroy_cb) ti_vtask_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_ARR */
    {
        .destroy = (ti_val_destroy_cb) ti_varr_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_SET */
    {
        .destroy = (ti_val_destroy_cb) ti_vset_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_ERROR */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_MEMBER */
    {
        .destroy = (ti_val_destroy_cb) ti_member_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_MPDATA */
    {
        .destroy = (ti_val_destroy_cb) free,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_CLOSURE */
    {
        .destroy = (ti_val_destroy_cb) ti_closure_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_FUTURE */
    {
        .destroy = (ti_val_destroy_cb) ti_future_destroy,
        .allowed_as_vtask_arg = false,
    },
    /* TI_VAL_TEMPLATE */
    {
        .destroy = (ti_val_destroy_cb) ti_template_destroy,
        .allowed_as_vtask_arg = false,
    },
};

#define ti_val(__val) (ti_val_type_props[(__val)->tp])

#endif /* TI_VAL_T_H_ */
