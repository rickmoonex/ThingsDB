/*
 * db.h
 */
#ifndef TI_DB_H_
#define TI_DB_H_

typedef struct ti_db_s  ti_db_t;

#include <stdint.h>
#include <ti/thing.h>
#include <ti/raw.h>
#include <util/imap.h>
#include <util/guid.h>
#include <ti/ex.h>
#include <ti/limits.h>

ti_db_t * ti_db_create(guid_t * guid, const ti_raw_t * name);
void ti_db_drop(ti_db_t * db);
int ti_db_buid(ti_db_t * db);
int ti_db_name_check(const ti_raw_t * name, ex_t * e);
int ti_db_store(ti_db_t * db, const char * fn);
int ti_db_restore(ti_db_t * db, const char * fn);

struct ti_db_s
{
    uint32_t ref;
    guid_t guid;
    ti_raw_t * name;
    imap_t * things;        /* weak mapping for things */
    vec_t * access;
    ti_thing_t * root;
    ti_limits_t * limits;
};

#endif /* TI_DB_H_ */
