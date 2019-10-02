/*
 * ti/field.h
 */
#ifndef TI_FIELD_H_
#define TI_FIELD_H_

typedef struct ti_field_s ti_field_t;

#include <ti/name.h>
#include <ti/raw.h>
#include <ti/val.h>
#include <inttypes.h>
#include <ex.h>

ti_field_t * ti_field_create(
        ti_name_t * name,
        ti_raw_t * spec_raw,
        ti_type_t * type,
        ex_t * e);
int ti_field_mod(ti_field_t * field, ti_raw_t * spec_raw, size_t n, ex_t * e);
int ti_field_del(ti_field_t * field, uint64_t ev_id);
void ti_field_remove(ti_field_t * field);
void ti_field_destroy(ti_field_t * field);
int ti_field_make_assignable(ti_field_t * field, ti_val_t ** val, ex_t * e);
int ti_field_check_field(ti_field_t * to, ti_field_t * from, ex_t * e);
ti_field_t * ti_field_by_name(ti_type_t * type, ti_name_t * name);
ti_field_t * ti_field_by_strn_e(
        ti_type_t * type,
        const char * str,
        size_t n,
        ex_t * e);
int ti_field_init_things(ti_field_t * field, ti_val_t ** vaddr, uint64_t ev_id);

struct ti_field_s
{
    ti_name_t * name;
    ti_raw_t * spec_raw;
    ti_type_t * type;
    uint16_t spec;
    uint16_t nested_spec;       /* array/set have a nested specification */
    uint32_t idx;               /* index of the field withing the type */
};



#endif  /* TI_FIELD_H_ */
