/*
 * ti/opr.c
 */
#include <assert.h>
#include <ti/opr.h>
#include <ti/raw.h>
#include <util/logger.h>

#define CAST_MAX 9223372036854775808.0

static int opr__eq(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__ge(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__gt(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__le(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__lt(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__ne(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__add(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__sub(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__mul(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__div(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__idiv(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__mod(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__and(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__xor(ti_val_t * a, ti_val_t * b, ex_t * e);
static int opr__or(ti_val_t * a, ti_val_t * b, ex_t * e);


int ti_opr_a_to_b(ti_val_t * a, cleri_node_t * nd, ti_val_t * b, ex_t * e)
{
    switch (nd->len)
    {
    case 1:
        switch (*nd->str)
        {
        case '%':
            return opr__mod(a, b, e);
        case '&':
            return opr__and(a, b, e);
        case '*':
            return opr__mul(a, b, e);
        case '+':
            return opr__add(a, b, e);
        case '-':
            return opr__sub(a, b, e);
        case '/':
            return opr__div(a, b, e);
        case '<':
            return opr__lt(a, b, e);
        case '>':
            return opr__gt(a, b, e);
        case '^':
            return opr__xor(a, b, e);
        case '|':
            return opr__or(a, b, e);
        }
        break;
    case 2:
        switch (*nd->str)
        {
        case '!':
            assert (nd->str[1] == '=');
            return opr__ne(a, b, e);
        case '%':
            assert (nd->str[1] == '=');
            return opr__mod(a, b, e);
        case '&':
            assert (nd->str[1] == '=');
            return opr__and(a, b, e);
        case '*':
            assert (nd->str[1] == '=');
            return opr__mul(a, b, e);
        case '+':
            assert (nd->str[1] == '=');
            return opr__add(a, b, e);
        case '-':
            assert (nd->str[1] == '=');
            return opr__sub(a, b, e);
        case '/':
            return nd->str[1] == '=' && a->tp == TI_VAL_FLOAT
                ? opr__div(a, b, e)
                : opr__idiv(a, b, e);
        case '<':
            assert (nd->str[1] == '=');
            return opr__le(a, b, e);
        case '=':
            assert (nd->str[1] == '=');
            return opr__eq(a, b, e);
        case '>':
            assert (nd->str[1] == '=');
            return opr__ge(a, b, e);
        case '^':
            assert (nd->str[1] == '=');
            return opr__xor(a, b, e);
        case '|':
            assert (nd->str[1] == '=');
            return opr__or(a, b, e);
        }
    }
    assert (0);
    return e->nr;
}

static int opr__eq(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = false;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
        goto type_err;
    case TI_VAL_NIL:
        bool_ =  a->tp == b->tp;
        break;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* false */
        case TI_VAL_INT:
            bool_ = a->via.int_ == b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ == b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ == b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* false */
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* false */
        case TI_VAL_INT:
            bool_ = a->via.float_ == b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ == b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ == b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* false */
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* false */
        case TI_VAL_INT:
            bool_ = a->via.bool_ == b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ == b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ == b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* false */
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            break;  /* false */
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = ti_raw_equal(a->via.raw, b->via.raw);
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* false */
        }
        break;
    case TI_VAL_REGEX:
        bool_ = b->tp == TI_VAL_REGEX && a->via.regex == b->via.regex;
        break;
    case TI_VAL_ARRAY:
        bool_ = ti_val_is_arr(b) && a->via.array == b->via.array;
        break;
    case TI_VAL_TUPLE:
        bool_ = ti_val_is_arr(b) && a->via.tuple == b->via.tuple;
        break;
    case TI_VAL_THING:
        bool_ = b->tp == TI_VAL_THING && a->via.thing == b->via.thing;
        break;
    case TI_VAL_THINGS:
        bool_ = ti_val_is_arr(b) && a->via.things == b->via.things;
        break;
    case TI_VAL_ARROW:
        bool_ = b->tp == TI_VAL_ARROW && a->via.arrow == b->via.arrow;
        break;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`==` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__ge(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = false;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.int_ >= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ >= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ >= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.float_ >= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ >= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ >= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.bool_ >= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ >= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ >= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            goto type_err;
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = ti_raw_cmp(a->via.raw, b->via.raw) >= 0;
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`>=` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__gt(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = false;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.int_ > b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ > b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ > b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.float_ > b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ > b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ > b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.bool_ > b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ > b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ > b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            goto type_err;
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = ti_raw_cmp(a->via.raw, b->via.raw) > 0;
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`>` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__le(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = false;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.int_ <= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ <= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ <= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.float_ <= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ <= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ <= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.bool_ <= b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ <= b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ <= b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            goto type_err;
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = ti_raw_cmp(a->via.raw, b->via.raw) <= 0;
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`<=` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__lt(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = false;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.int_ < b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ < b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ < b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.float_ < b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ < b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ < b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            bool_ = a->via.bool_ < b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ < b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ < b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            goto type_err;
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = ti_raw_cmp(a->via.raw, b->via.raw) < 0;
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`<` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__ne(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    _Bool bool_ = true;
    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
        goto type_err;
    case TI_VAL_NIL:
        bool_ =  a->tp != b->tp;
        break;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* true */
        case TI_VAL_INT:
            bool_ = a->via.int_ != b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.int_ != b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.int_ != b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* true */
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* true */
        case TI_VAL_INT:
            bool_ = a->via.float_ != b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.float_ != b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.float_ != b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* true */
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
            break;  /* true */
        case TI_VAL_INT:
            bool_ = a->via.bool_ != b->via.int_;
            break;
        case TI_VAL_FLOAT:
            bool_ = a->via.bool_ != b->via.float_;
            break;
        case TI_VAL_BOOL:
            bool_ = a->via.bool_ != b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* true */
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
            goto type_err;
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            break;  /* true */
        case TI_VAL_QP:
        case TI_VAL_RAW:
            bool_ = !ti_raw_equal(a->via.raw, b->via.raw);
            break;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            break;  /* true */
        }
        break;
    case TI_VAL_REGEX:
        bool_ = b->tp != TI_VAL_REGEX || a->via.regex != b->via.regex;
        break;
    case TI_VAL_ARRAY:
        bool_ = !ti_val_is_arr(b) || a->via.array != b->via.array;
        break;
    case TI_VAL_TUPLE:
        bool_ = !ti_val_is_arr(b) || a->via.tuple != b->via.tuple;
        break;
    case TI_VAL_THING:
        bool_ = b->tp != TI_VAL_THING || a->via.thing != b->via.thing;
        break;
    case TI_VAL_THINGS:
        bool_ = !ti_val_is_arr(b) || a->via.things != b->via.things;
        break;
    case TI_VAL_ARROW:
        bool_ = b->tp != TI_VAL_ARROW || a->via.arrow != b->via.arrow;
        break;
    }

    ti_val_clear(b);
    ti_val_set_bool(b, bool_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`!=` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__add(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */
    double float_ = 0.0f;   /* set to 0 only to prevent warning */
    ti_raw_t * raw = NULL;  /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if ((b->via.int_ > 0 && a->via.int_ > LLONG_MAX - b->via.int_) ||
                (b->via.int_ < 0 && a->via.int_ < LLONG_MIN - b->via.int_))
                goto overflow;
            int_ = a->via.int_ + b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.int_ + b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            if (a->via.int_ == LLONG_MAX && b->via.bool_)
                goto overflow;
            int_ = a->via.int_ + b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            float_ = a->via.float_ + b->via.int_;
            goto type_float;
        case TI_VAL_FLOAT:
            float_ = a->via.float_ + b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            float_ = a->via.float_ + b->via.bool_;
            goto type_float;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (a->via.bool_ && b->via.int_ == LLONG_MAX)
                goto overflow;
            int_ = a->via.bool_ + b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.bool_ + b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ + b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
        case TI_VAL_INT:
        case TI_VAL_FLOAT:
        case TI_VAL_BOOL:
            goto type_err;
        case TI_VAL_QP:
        case TI_VAL_RAW:
            raw = ti_raw_cat(a->via.raw, b->via.raw);
            goto type_raw;
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    assert (0);

type_raw:
    ti_val_clear(b);

    if (!raw)
        ex_set_alloc(e);
    else
        ti_val_weak_set(b, TI_VAL_RAW, raw);
    return e->nr;

type_float:

    ti_val_clear(b);
    ti_val_set_float(b, float_);

    return e->nr;

type_int:

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`+` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

overflow:
    ex_set(e, EX_OVERFLOW, "integer overflow");
    return e->nr;
}

static int opr__sub(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */
    double float_ = 0.0f;   /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if ((b->via.int_ < 0 && a->via.int_ > LLONG_MAX + b->via.int_) ||
                (b->via.int_ > 0 && a->via.int_ < LLONG_MIN + b->via.int_))
                goto overflow;
            int_ = a->via.int_ - b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.int_ - b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            if (a->via.int_ == LLONG_MIN && b->via.bool_)
                goto overflow;
            int_ = a->via.int_ - b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            float_ = a->via.float_ - b->via.int_;
            goto type_float;
        case TI_VAL_FLOAT:
            float_ = a->via.float_ - b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            float_ = a->via.float_ - b->via.bool_;
            goto type_float;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == LLONG_MIN ||
                (a->via.bool_ && b->via.int_ == -LLONG_MAX))
                goto overflow;
            int_ = a->via.bool_ - b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.bool_ - b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ - b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    assert (0);

type_float:

    ti_val_clear(b);
    ti_val_set_float(b, float_);

    return e->nr;

type_int:

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`-` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

overflow:
    ex_set(e, EX_OVERFLOW, "integer overflow");
    return e->nr;
}

static int opr__mul(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */
    double float_ = 0.0f;   /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if ((a->via.int_ > LLONG_MAX / b->via.int_) ||
                (a->via.int_ < LLONG_MIN / b->via.int_) ||
                (a->via.int_ == -1 && b->via.int_ == LLONG_MIN) ||
                (b->via.int_ == -1 && a->via.int_ == LLONG_MIN))
                goto overflow;
            int_ = a->via.int_ * b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.int_ * b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            int_ = a->via.int_ * b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            float_ = a->via.float_ * b->via.int_;
            goto type_float;
        case TI_VAL_FLOAT:
            float_ = a->via.float_ * b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            float_ = a->via.float_ * b->via.bool_;
            goto type_float;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.bool_ * b->via.int_;
            goto type_int;
        case TI_VAL_FLOAT:
            float_ = a->via.bool_ * b->via.float_;
            goto type_float;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ * b->via.bool_;
            goto type_int;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    assert (0);

type_float:

    ti_val_clear(b);
    ti_val_set_float(b, float_);

    return e->nr;

type_int:

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`*` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

overflow:
    ex_set(e, EX_OVERFLOW, "integer overflow");
    return e->nr;
}

static int opr__div(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    double float_ = 0.0f;   /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            float_ = (double) a->via.int_ / (double) b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            float_ = (double) a->via.int_ / b->via.float_;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            float_ = (double) a->via.int_ / (double) b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            float_ = a->via.float_ / (double) b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            float_ = a->via.float_ / b->via.float_;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            float_ = a->via.float_ / (double) b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            float_ = (double) a->via.bool_ / (double) b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            float_ = (double) a->via.bool_ / b->via.float_;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            float_ = (double) a->via.bool_ / (double) b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_float(b, float_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`/` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

zerodiv:
    ex_set(e, EX_ZERO_DIV, "division or modulo by zero");
    return e->nr;
}

static int opr__idiv(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;   /* set to 0 only to prevent warning */
    double d;

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            if (a->via.int_ == LLONG_MAX && b->via.int_ == -1)
                goto overflow;
            int_ = a->via.int_ / b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            d = a->via.int_ / b->via.float_;
            if (ti_val_overflow_cast(d))
                goto overflow;
            int_ = (int64_t) d;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            int_ = a->via.int_ / b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            d = a->via.float_ / b->via.int_;
            if (ti_val_overflow_cast(d))
                goto overflow;
            int_ = (int64_t) d;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            d = a->via.float_ / b->via.float_;
            if (ti_val_overflow_cast(d))
                goto overflow;
            int_ = (int64_t) d;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            d = a->via.float_ / b->via.bool_;
            if (ti_val_overflow_cast(d))
                goto overflow;
            int_ = (int64_t) d;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            int_ = a->via.bool_ / b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (b->via.float_ == 0.0)
                goto zerodiv;
            d = a->via.bool_ / b->via.float_;
            if (ti_val_overflow_cast(d))
                goto overflow;
            int_ = (int64_t) d;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            int_ = a->via.bool_ / b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`//` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

overflow:
    ex_set(e, EX_OVERFLOW, "integer overflow");
    return e->nr;

zerodiv:
    ex_set(e, EX_ZERO_DIV, "division or modulo by zero");
    return e->nr;
}

static int opr__mod(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            int_ = a->via.int_ % b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (ti_val_overflow_cast(b->via.float_))
                goto overflow;
            int_ = (int64_t) b->via.float_;
            if (int_ == 0)
                goto zerodiv;
            int_ = a->via.int_ % int_;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            int_ = a->via.int_ % b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (ti_val_overflow_cast(a->via.float_))
                goto overflow;
            if (b->via.int_ == 0)
                goto zerodiv;
            int_ = (int64_t) a->via.float_ % b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (ti_val_overflow_cast(a->via.float_) ||
                ti_val_overflow_cast(b->via.float_))
                goto overflow;
            int_ = (int64_t) b->via.float_;
            if (int_ == 0)
                goto zerodiv;
            int_ = (int64_t) a->via.float_ % int_;
            break;
        case TI_VAL_BOOL:
            if (ti_val_overflow_cast(a->via.float_))
                goto overflow;
            if (b->via.bool_ == 0)
                goto zerodiv;
            int_ = (int64_t) a->via.float_ % b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            if (b->via.int_ == 0)
                goto zerodiv;
            int_ = a->via.bool_ % b->via.int_;
            break;
        case TI_VAL_FLOAT:
            if (ti_val_overflow_cast(b->via.float_))
                goto overflow;
            int_ = (int64_t) b->via.float_;
            if (int_ == 0)
                goto zerodiv;
            int_ = a->via.bool_ % int_;
            break;
        case TI_VAL_BOOL:
            if (b->via.bool_ == 0)
                goto zerodiv;
            int_ = a->via.bool_ % b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "`%` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;

overflow:
    ex_set(e, EX_OVERFLOW, "integer overflow");
    return e->nr;

zerodiv:
    ex_set(e, EX_ZERO_DIV, "division or modulo by zero");
    return e->nr;
}

static int opr__and(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.int_ & b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.int_ & b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        goto type_err;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.bool_ & b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ & b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "bitwise `&` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__xor(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.int_ ^ b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.int_ ^ b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        goto type_err;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.bool_ ^ b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ ^ b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "bitwise `^` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

static int opr__or(ti_val_t * a, ti_val_t * b, ex_t * e)
{
    int64_t int_ = 0;       /* set to 0 only to prevent warning */

    switch ((ti_val_enum) a->tp)
    {
    case TI_VAL_ATTR:
    case TI_VAL_NIL:
        goto type_err;
    case TI_VAL_INT:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.int_ | b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.int_ | b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_FLOAT:
        goto type_err;
    case TI_VAL_BOOL:
        switch ((ti_val_enum) b->tp)
        {
        case TI_VAL_ATTR:
        case TI_VAL_NIL:
            goto type_err;
        case TI_VAL_INT:
            int_ = a->via.bool_ | b->via.int_;
            break;
        case TI_VAL_FLOAT:
            goto type_err;
        case TI_VAL_BOOL:
            int_ = a->via.bool_ | b->via.bool_;
            break;
        case TI_VAL_QP:
        case TI_VAL_RAW:
        case TI_VAL_REGEX:
        case TI_VAL_ARRAY:
        case TI_VAL_TUPLE:
        case TI_VAL_THING:
        case TI_VAL_THINGS:
        case TI_VAL_ARROW:
            goto type_err;
        }
        break;
    case TI_VAL_QP:
    case TI_VAL_RAW:
    case TI_VAL_REGEX:
    case TI_VAL_ARRAY:
    case TI_VAL_TUPLE:
    case TI_VAL_THING:
    case TI_VAL_THINGS:
    case TI_VAL_ARROW:
        goto type_err;
    }

    ti_val_clear(b);
    ti_val_set_int(b, int_);

    return e->nr;

type_err:
    ex_set(e, EX_BAD_DATA, "bitwise `|` not supported between `%s` and `%s`",
        ti_val_str(a), ti_val_str(b));
    return e->nr;
}

