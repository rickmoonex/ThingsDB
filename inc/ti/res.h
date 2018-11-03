/*
 * ti/res.h
 */
#ifndef TI_RES_H_
#define TI_RES_H_

typedef struct ti_res_s ti_res_t;

#include <ti/db.h>
#include <ti/val.h>
#include <ti/ex.h>
#include <util/imap.h>
#include <cleri/cleri.h>


ti_res_t * ti_res_create(ti_db_t * db);
void ti_res_destroy(ti_res_t * res);
int ti_res_scope(ti_res_t * res, cleri_node_t * nd, ex_t * e);

struct ti_res_s
{
    ti_db_t * db;       /* this is using a borrowed reference */
    ti_val_t * val;
    imap_t * collect;
};

#endif /* TI_RES_H_ */
