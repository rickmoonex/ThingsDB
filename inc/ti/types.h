/*
 * ti/types.h
 */
#ifndef TI_TYPES_H_
#define TI_TYPES_H_

typedef struct ti_types_s ti_types_t;

#include <stdint.h>
#include <util/smap.h>
#include <util/imap.h>
#include <ti/type.h>
#include <ti/raw.h>
#include <ex.h>

ti_types_t * ti_types_create(void);
void ti_types_destroy(ti_types_t * types);
int ti_types_add(ti_types_t * types, ti_type_t * type);
void ti_types_del(ti_types_t * types, ti_type_t * type);
uint16_t ti_types_get_new_id(ti_types_t * types, ex_t * e);

struct ti_types_s
{
    imap_t * imap;
    smap_t * smap;
};

#endif /* TI_TYPES_H_ */
