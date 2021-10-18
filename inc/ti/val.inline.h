/*
 * ti/val.inline.h
 */
#ifndef TI_VAL_INLINE_H_
#define TI_VAL_INLINE_H_

#include <ex.h>
#include <ti/closure.h>
#include <ti/collection.h>
#include <ti/datetime.h>
#include <ti/datetime.h>
#include <ti/name.h>
#include <ti/nil.h>
#include <ti/thing.h>
#include <ti/thing.inline.h>
#include <ti/val.h>
#include <ti/varr.inline.h>
#include <ti/vset.h>

static inline void ti_val_drop(ti_val_t * val)
{
    if (val && !--val->ref)
        ti_val(val)->destroy(val);
}

static inline void ti_val_unsafe_drop(ti_val_t * val)
{
    if (!--val->ref)
        ti_val(val)->destroy(val);
}

static inline void ti_val_unsafe_gc_drop(ti_val_t * val)
{
    if (!--val->ref)
        ti_val(val)->destroy(val);
    else
        ti_thing_may_push_gc((ti_thing_t *) val);
}

static inline void ti_val_gc_drop(ti_val_t * val)
{
    if (val)
        ti_val_unsafe_gc_drop(val);
}

static inline void ti_val_unassign_unsafe_drop(ti_val_t * val)
{
    if (!--val->ref)
        ti_val_call(val)->destroy(val);
    else if (val->tp == TI_VAL_SET || val->tp == TI_VAL_ARR)
        ((ti_varr_t *) val)->parent = NULL;
    else
        ti_thing_may_push_gc((ti_thing_t *) val);
}

static inline void ti_val_unassign_drop(ti_val_t * val)
{
    if (val)
        ti_val_unassign_unsafe_drop(val);
}

static inline _Bool ti_val_is_arr(ti_val_t * val)
{
    return val->tp == TI_VAL_ARR;
}

static inline _Bool ti_val_is_bool(ti_val_t * val)
{
    return val->tp == TI_VAL_BOOL;
}

static inline _Bool ti_val_is_datetime(ti_val_t * val)
{
    return val->tp == TI_VAL_DATETIME;
}

static inline _Bool ti_val_is_datetime_strict(ti_val_t * val)
{
    return val->tp == TI_VAL_DATETIME &&
            ti_datetime_is_datetime((ti_datetime_t *) val);
}

static inline _Bool ti_val_is_timeval(ti_val_t * val)
{
    return val->tp == TI_VAL_DATETIME &&
            ti_datetime_is_timeval((ti_datetime_t *) val);
}

static inline _Bool ti_val_is_closure(ti_val_t * val)
{
    return val->tp == TI_VAL_CLOSURE;
}

static inline _Bool ti_val_is_error(ti_val_t * val)
{
    return val->tp == TI_VAL_ERROR;
}

static inline _Bool ti_val_is_number(ti_val_t * val)
{
    return val->tp == TI_VAL_FLOAT || val->tp == TI_VAL_INT;
}

static inline _Bool ti_val_is_float(ti_val_t * val)
{
    return val->tp == TI_VAL_FLOAT;
}

static inline _Bool ti_val_is_int(ti_val_t * val)
{
    return val->tp == TI_VAL_INT;
}

static inline _Bool ti_val_is_nil(ti_val_t * val)
{
    return val->tp == TI_VAL_NIL;
}

static inline _Bool ti_val_is_str(ti_val_t * val)
{
    return val->tp == TI_VAL_STR || val->tp == TI_VAL_NAME;
}

static inline _Bool ti_val_is_str_regex(ti_val_t * val)
{
    return val->tp == TI_VAL_STR ||
           val->tp == TI_VAL_NAME ||
           val->tp == TI_VAL_REGEX;
}

static inline _Bool ti_val_is_str_closure(ti_val_t * val)
{
    return val->tp == TI_VAL_STR ||
           val->tp == TI_VAL_NAME ||
           val->tp == TI_VAL_CLOSURE;
}

static inline _Bool ti_val_is_bytes(ti_val_t * val)
{
    return val->tp == TI_VAL_BYTES;
}

static inline _Bool ti_val_is_raw(ti_val_t * val)
{
    return val->tp == TI_VAL_STR ||
           val->tp == TI_VAL_NAME ||
           val->tp == TI_VAL_BYTES ||
           val->tp == TI_VAL_MPDATA;
}

static inline _Bool ti_val_is_str_bytes_nil(ti_val_t * val)
{
    return val->tp == TI_VAL_STR ||
           val->tp == TI_VAL_NAME ||
           val->tp == TI_VAL_BYTES ||
           val->tp == TI_VAL_NIL;
}

static inline _Bool ti_val_is_mpdata(ti_val_t * val)
{
    return val->tp == TI_VAL_MPDATA;
}

static inline _Bool ti_val_is_regex(ti_val_t * val)
{
    return val->tp == TI_VAL_REGEX;
}

static inline _Bool ti_val_is_set(ti_val_t * val)
{
    return val->tp == TI_VAL_SET;
}

static inline _Bool ti_val_is_thing(ti_val_t * val)
{
    return val->tp == TI_VAL_THING;
}

static inline _Bool ti_val_is_wrap(ti_val_t * val)
{
    return val->tp == TI_VAL_WRAP;
}

static inline _Bool ti_val_is_room(ti_val_t * val)
{
    return val->tp == TI_VAL_ROOM;
}

static inline _Bool ti_val_is_task(ti_val_t * val)
{
    return val->tp == TI_VAL_ROOM;
}

static inline _Bool ti_val_is_member(ti_val_t * val)
{
    return val->tp == TI_VAL_MEMBER;
}

static inline _Bool ti_val_is_array(ti_val_t * val)
{
    return val->tp == TI_VAL_ARR;
}

static inline _Bool ti_val_is_list(ti_val_t * val)
{
    return val->tp == TI_VAL_ARR && ti_varr_is_list((ti_varr_t *) val);
}

static inline _Bool ti_val_is_tuple(ti_val_t * val)
{
    return val->tp == TI_VAL_ARR && ti_varr_is_tuple((ti_varr_t *) val);
}

static inline _Bool ti_val_is_future(ti_val_t * val)
{
    return val->tp == TI_VAL_FUTURE;
}

static inline _Bool ti_val_overflow_cast(double d)
{
    return !(d >= -VAL__CAST_MAX && d < VAL__CAST_MAX);
}

static inline _Bool ti_val_is_mut_locked(ti_val_t * val)
{
    return (val->tp == TI_VAL_ARR || val->tp == TI_VAL_SET) &&
           (val->flags & TI_VFLAG_LOCK);
}

/*
 * Names
 */
static inline ti_val_t * ti_val_year_name(void)
{
    ti_incref(val__year_name);
    return val__year_name;
}

static inline ti_val_t * ti_val_month_name(void)
{
    ti_incref(val__month_name);
    return val__month_name;
}

static inline ti_val_t * ti_val_day_name(void)
{
    ti_incref(val__day_name);
    return val__day_name;
}

static inline ti_val_t * ti_val_hour_name(void)
{
    ti_incref(val__hour_name);
    return val__hour_name;
}

static inline ti_val_t * ti_val_minute_name(void)
{
    ti_incref(val__minute_name);
    return val__minute_name;
}

static inline ti_val_t * ti_val_second_name(void)
{
    ti_incref(val__second_name);
    return val__second_name;
}

static inline ti_val_t * ti_val_gmt_offset_name(void)
{
    ti_incref(val__gmt_offset_name);
    return val__gmt_offset_name;
}

static inline ti_val_t * ti_val_borrow_year_name(void)
{
    return val__year_name;
}

static inline ti_val_t * ti_val_borrow_month_name(void)
{
    return val__month_name;
}

static inline ti_val_t * ti_val_borrow_day_name(void)
{
    return val__day_name;
}

static inline ti_val_t * ti_val_borrow_hour_name(void)
{
    return val__hour_name;
}

static inline ti_val_t * ti_val_borrow_minute_name(void)
{
    return val__minute_name;
}

static inline ti_val_t * ti_val_borrow_second_name(void)
{
    return val__second_name;
}

static inline ti_val_t * ti_val_borrow_module_name(void)
{
    return val__module_name;
}

static inline ti_val_t * ti_val_borrow_deep_name(void)
{
    return val__deep_name;
}

static inline ti_val_t * ti_val_borrow_load_name(void)
{
    return val__load_name;
}

static inline ti_val_t * ti_val_borrow_beautify_name(void)
{
    return val__beautify_name;
}

/*
 * Return 0 when a new lock is set, or -1 if failed and `e` is set.
 *
 * Use `ti_val_try_lock(..)` if you require a lock but no current lock
 * may be set. This is when you make changes, for example with `push`.
 *
 * Call `ti_val_unlock(..)` with `true` as second argument after a successful
 * lock.
 */
static inline int ti_val_try_lock(ti_val_t * val, ex_t * e)
{
    if (val->flags & TI_VFLAG_LOCK)
    {
        ex_set(e, EX_OPERATION,
            "cannot change type `%s` while the value is being used",
            ti_val_str(val));
        return -1;
    }
    return (val->flags |= TI_VFLAG_LOCK) & 0;
}

/*
 * Returns `lock_was_set`: 0 if already locked, 1 if a new lock is set
 *
 * Use `ti_val_ensure_lock(..)` if you require a lock but it does not matter
 * if the value is already locked by someone else. The return value can be
 * used with the `ti_val_unlock(..)` function.
 *
 * For example in the `map(..)` function requires a lock but since `map`
 * does not make changes it is no problem if another lock was set.
 */
static inline int ti_val_ensure_lock(ti_val_t * val)
{
    return (val->flags & TI_VFLAG_LOCK)
            ? 0
            : !!(val->flags |= TI_VFLAG_LOCK);
}

static inline void ti_val_unlock(ti_val_t * val, int lock_was_set)
{
    if (lock_was_set)
        val->flags &= ~TI_VFLAG_LOCK;
}

static inline _Bool ti_val_is_object(ti_val_t * val)
{
    return val->tp == TI_VAL_THING && ti_thing_is_object((ti_thing_t *) val);
}

static inline _Bool ti_val_is_instance(ti_val_t * val)
{
    return val->tp == TI_VAL_THING && ti_thing_is_instance((ti_thing_t *) val);
}

static inline void ti_val_attach(
        ti_val_t * val,
        ti_thing_t * parent,
        void * key)  /* ti_raw_t or ti_name_t */
{
    switch ((ti_val_enum) val->tp)
    {
    case TI_VAL_NIL:
    case TI_VAL_INT:
    case TI_VAL_FLOAT:
    case TI_VAL_BOOL:
    case TI_VAL_DATETIME:
    case TI_VAL_MPDATA:
    case TI_VAL_NAME:
    case TI_VAL_STR:
    case TI_VAL_BYTES:
    case TI_VAL_REGEX:
    case TI_VAL_THING:
    case TI_VAL_WRAP:
    case TI_VAL_ROOM:
    case TI_VAL_ERROR:
    case TI_VAL_MEMBER:
    case TI_VAL_CLOSURE:
    case TI_VAL_FUTURE:
        return;
    case TI_VAL_ARR:
        ((ti_varr_t *) val)->parent = parent;
        ((ti_varr_t *) val)->key_ = key;
        return;
    case TI_VAL_SET:
        ((ti_vset_t *) val)->parent = parent;
        ((ti_vset_t *) val)->key_ = key;
        return;
    case TI_VAL_TEMPLATE:
        break;
    }
    assert(0);
    return;
}

/*
 * although the underlying pointer might point to a new value after calling
 * this function, the `old` pointer value can still be used and has at least
 * one reference left.
 *
 * errors:
 *      - memory allocation (vector / set /closure creation)
 *      - lock/syntax/bad data errors in closure
 */
static inline int ti_val_make_assignable(
        ti_val_t ** val,
        ti_thing_t * parent,
        void * key,
        ex_t * e)
{
    switch ((ti_val_enum) (*val)->tp)
    {
    case TI_VAL_NIL:
    case TI_VAL_INT:
    case TI_VAL_FLOAT:
    case TI_VAL_BOOL:
    case TI_VAL_DATETIME:
    case TI_VAL_MPDATA:
    case TI_VAL_NAME:
    case TI_VAL_STR:
    case TI_VAL_BYTES:
    case TI_VAL_REGEX:
    case TI_VAL_THING:
    case TI_VAL_WRAP:
    case TI_VAL_ROOM:
    case TI_VAL_TASK:
    case TI_VAL_ERROR:
    case TI_VAL_MEMBER:
        return 0;
    case TI_VAL_ARR:
        if (ti_varr_to_list((ti_varr_t **) val))
        {
            ex_set_mem(e);
            return e->nr;
        }
        ((ti_varr_t *) *val)->parent = parent;
        ((ti_varr_t *) *val)->key_ = key;
        return 0;
    case TI_VAL_SET:
        if (ti_vset_assign((ti_vset_t **) val))
        {
            ex_set_mem(e);
            return e->nr;
        }
        ((ti_vset_t *) *val)->parent = parent;
        ((ti_vset_t *) *val)->key_ = key;
        return 0;
    case TI_VAL_CLOSURE:
        return ti_closure_unbound((ti_closure_t *) *val, e);
    case TI_VAL_FUTURE:
        ti_val_unsafe_drop(*val);
        *val = (ti_val_t *) ti_nil_get();
        return 0;
    case TI_VAL_TEMPLATE:
        break;
    }
    assert(0);
    return -1;
}

static inline int ti_val_make_variable(ti_val_t ** val, ex_t * e)
{
    switch ((ti_val_enum) (*val)->tp)
    {
    case TI_VAL_NIL:
    case TI_VAL_INT:
    case TI_VAL_FLOAT:
    case TI_VAL_BOOL:
    case TI_VAL_DATETIME:
    case TI_VAL_MPDATA:
    case TI_VAL_NAME:
    case TI_VAL_STR:
    case TI_VAL_BYTES:
    case TI_VAL_REGEX:
    case TI_VAL_THING:
    case TI_VAL_WRAP:
    case TI_VAL_ROOM:
    case TI_VAL_ERROR:
    case TI_VAL_MEMBER:
    case TI_VAL_FUTURE:
        return 0;
    case TI_VAL_ARR:
        if (((ti_varr_t *) *val)->parent &&
            ti_varr_is_list((ti_varr_t *) *val) &&
            ti_varr_to_list((ti_varr_t **) val))
            ex_set_mem(e);
        return e->nr;
    case TI_VAL_SET:
        if (((ti_vset_t *) *val)->parent && ti_vset_assign((ti_vset_t **) val))
            ex_set_mem(e);
        return e->nr;
    case TI_VAL_CLOSURE:
        return ti_closure_unbound((ti_closure_t * ) *val, e);
    case TI_VAL_TEMPLATE:
        break;
    }
    assert(0);
    return -1;
}

#endif  /* TI_VAL_INLINE_H_ */



