/*
 * ti/store/collection.c
 */
#include <assert.h>
#include <ti.h>
#include <ti/store/collection.h>
#include <util/fx.h>
#include <stdlib.h>

static const char * ti__store_collection_things_fn     = "things.dat";
static const char * ti__store_collection_collection_fn = "collection.dat";
//static const char * ti__store_collection_skeleton_fn   = "skeleton.qp";
static const char * ti__store_collection_props_fn      = "props.qp";
static const char * ti__store_collection_attrs_fn      = "attrs.qp";
static const char * ti__store_collection_access_fn     = "access.qp";

ti_store_collection_t * ti_store_collection_create(
        const char * path,
        ti_collection_t * collection)
{
    char * collection_path;
    ti_store_collection_t * store_collection;

    store_collection = malloc(sizeof(ti_store_collection_t));
    if (!store_collection)
        goto fail0;

    collection_path = store_collection->collection_path = fx_path_join(
            path,
            collection->guid.guid);
    if (!collection_path)
        goto fail0;

    store_collection->access_fn = fx_path_join(
            collection_path,
            ti__store_collection_access_fn);
    store_collection->things_fn = fx_path_join(
            collection_path,
            ti__store_collection_things_fn);
    store_collection->collection_fn = fx_path_join(
            collection_path,
            ti__store_collection_collection_fn);
//    store_collection->skeleton_fn = fx_path_join(
//            collection_path,
//            ti__store_collection_skeleton_fn);
    store_collection->props_fn = fx_path_join(
            collection_path,
            ti__store_collection_props_fn);
    store_collection->attrs_fn = fx_path_join(
            collection_path,
            ti__store_collection_attrs_fn);

    if (    !store_collection->access_fn ||
            !store_collection->things_fn ||
            !store_collection->collection_fn ||
//            !store_collection->skeleton_fn ||
            !store_collection->props_fn ||
            !store_collection->attrs_fn)
        goto fail1;

    return store_collection;
fail1:
    ti_store_collection_destroy(store_collection);
    return NULL;
fail0:
    free(store_collection);
    return NULL;
}

void ti_store_collection_destroy(ti_store_collection_t * store_collection)
{
    if (!store_collection)
        return;
    free(store_collection->access_fn);
    free(store_collection->things_fn);
    free(store_collection->collection_fn);
    free(store_collection->props_fn);
    free(store_collection->attrs_fn);
    free(store_collection->collection_path);
    free(store_collection);
}

int ti_store_collection_store(ti_collection_t * collection, const char * fn)
{
    int rc = 0;
    FILE * f = fopen(fn, "w");
    if (!f)
    {
        log_error("cannot open file `%s` (%s)", fn, strerror(errno));
        return -1;
    }

    if (fwrite(&collection->root->id, sizeof(uint64_t), 1, f) != 1)
    {
        log_error("cannot write to file `%s`", fn);
        rc = -1;
    }

    if (fclose(f))
    {
        log_error("cannot close file `%s` (%s)", fn, strerror(errno));
        rc = -1;
    }

    if (rc == 0)
        log_debug("stored collection info to file: `%s`", fn);

    return rc;
}

int ti_store_collection_restore(ti_collection_t * collection, const char * fn)
{
    int rc = 0;
    ssize_t sz;
    uint64_t id;
    uchar * data = fx_read(fn, &sz);
    if (!data || sz != sizeof(uint64_t))
        goto failed;

    memcpy(&id, data, sizeof(uint64_t));

    collection->root = imap_get(collection->things, id);
    if (!collection->root)
    {
        log_critical("cannot find root thing: %"PRIu64, id);
        goto failed;
    }

    collection->root = ti_grab(collection->root);
    goto done;

failed:
    rc = -1;
    log_critical("failed to restore from file: `%s`", fn);
done:
    free(data);
    return rc;
}
