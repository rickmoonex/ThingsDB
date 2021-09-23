/*
 * ti/module.c
 */
#include <ex.h>
#include <stdlib.h>
#include <ti.h>
#include <ti/fn/fn.h>
#include <ti/future.h>
#include <ti/future.inline.h>
#include <ti/module.h>
#include <ti/names.h>
#include <ti/pkg.h>
#include <ti/proc.h>
#include <ti/proto.h>
#include <ti/proto.t.h>
#include <ti/query.h>
#include <ti/raw.inline.h>
#include <ti/raw.t.h>
#include <ti/scope.h>
#include <ti/val.inline.h>
#include <ti/verror.h>
#include <util/fx.h>
#include <curl/curl.h>

#define MODULE__TOO_MANY_RESTARTS 3


static void module__write_req_cb(uv_write_t * req, int status)
{
    if (status)
    {
        ex_t e;
        ti_future_t * future = req->data;

        /* remove the future from the module */
        (void) omap_rm(future->module->futures, future->pid);

        ex_set(&e, EX_OPERATION, uv_strerror(status));
        ti_query_on_future_result(future, &e);
    }

    free(req);
}

static int module__write_req(ti_future_t * future)
{
    int uv_err = 0;
    ti_module_t * module = future->module;
    ti_proc_t * proc = &module->proc;
    uv_buf_t wrbuf;
    ti_future_t * prev;
    uv_write_t * req;

    req = malloc(sizeof(uv_write_t));
    if (!req)
        return UV_EAI_MEMORY;

    req->data = future;
    future->pkg->id = future->pid = module->next_pid;

    prev = omap_set(module->futures, future->pid, future);
    if (!prev)
    {
        free(req);
        return UV_EAI_MEMORY;
    }

    if (prev != future)
        ti_future_cancel(prev);

    wrbuf = uv_buf_init(
            (char *) future->pkg,
            sizeof(ti_pkg_t) + future->pkg->n);

    uv_err = uv_write(
            req,
            (uv_stream_t *) &proc->child_stdin,
            &wrbuf,
            1,
            &module__write_req_cb);

    if (uv_err)
    {
        (void) omap_rm(module->futures, module->next_pid);
        free(req);
        return uv_err;
    }

    ++module->next_pid;
    return 0;
}

static void module__write_conf_cb(uv_write_t * req, int status)
{
    if (status)
    {
        log_error(uv_strerror(status));
        ((ti_module_t *) req->data)->status = status;
    }

    free(req);
}

static int module__write_conf(ti_module_t * module)
{
    int uv_err;
    ti_proc_t * proc = &module->proc;
    uv_buf_t wrbuf;
    uv_write_t * req;

    req = malloc(sizeof(uv_write_t));
    if (!req)
        return UV_EAI_MEMORY;

    req->data = module;

    wrbuf = uv_buf_init(
            (char *) module->conf_pkg,
            sizeof(ti_pkg_t) + module->conf_pkg->n);
    uv_err = uv_write(
            req,
            (uv_stream_t *) &proc->child_stdin,
            &wrbuf,
            1,
            &module__write_conf_cb);

    if (uv_err)
    {
        free(req);
        return uv_err;
    }

    return 0;
}

static void module__cb(ti_future_t * future)
{
    int uv_err;
    ti_thing_t * thing = VEC_get(future->args, 0);
    ti_vp_t vp;
    msgpack_sbuffer buffer;
    size_t alloc_sz = 1024;

    assert (ti_val_is_thing((ti_val_t *) thing));

    if (future->module->status)
    {
        ex_t e;
        ex_set(&e, EX_OPERATION,
                "module `%s` is not running (status: %s)",
                future->module->name->str,
                ti_module_status_str(future->module));
        ti_query_on_future_result(future, &e);
        return;
    }

    if (future->module->proc.process.pid == 0)
    {
        ex_t e;
        ex_set(&e, EX_OPERATION, "missing process ID for module `%s`",
                future->module->name->str);
        ti_query_on_future_result(future, &e);
        return;
    }

    if (future->module->flags & TI_MODULE_FLAG_WAIT_CONF)
    {
        ex_t e;
        ex_set(&e, EX_OPERATION,
                "module `%s` is not configured; "
                "if you keep this error, then please check the module status",
                future->module->name->str);
        ti_query_on_future_result(future, &e);
        return;
    }

    if (mp_sbuffer_alloc_init(&buffer, alloc_sz, sizeof(ti_pkg_t)))
        goto mem_error0;
    msgpack_packer_init(&vp.pk, &buffer, msgpack_sbuffer_write);

    if (ti_thing_to_pk(thing, &vp, ti_future_deep(future)))
        goto mem_error1;

    future->pkg = (ti_pkg_t *) buffer.data;
    pkg_init(future->pkg, 0, TI_PROTO_MODULE_REQ, buffer.size);

    uv_err = module__write_req(future);
    if (uv_err)
    {
        ex_t e;
        ex_set(&e, EX_OPERATION, uv_strerror(uv_err));
        ti_query_on_future_result(future, &e);
    }
    return;

mem_error1:
    msgpack_sbuffer_destroy(&buffer);
mem_error0:
    {
        ex_t e;
        ex_set_internal(&e);
        ti_query_on_future_result(future, &e);
    }
}

ti_pkg_t * ti_module_conf_pkg(ti_val_t * val, ti_query_t * query)
{
    ti_pkg_t * pkg;
    ti_vp_t vp = {
            .query=query,
    };
    msgpack_sbuffer buffer;
    size_t alloc_sz = 1024;

    if (mp_sbuffer_alloc_init(&buffer, alloc_sz, sizeof(ti_pkg_t)))
        return NULL;
    msgpack_packer_init(&vp.pk, &buffer, msgpack_sbuffer_write);

    /*
     * Module configuration will be packed 2 levels deep. This is a fixed
     * setting and should be sufficient to configure a module.
     */
    if (ti_val_to_pk(val, &vp, 2))
    {
        msgpack_sbuffer_destroy(&buffer);
        return NULL;
    }

    pkg = (ti_pkg_t *) buffer.data;
    pkg_init(pkg, 0, TI_PROTO_MODULE_CONF, buffer.size);

    return pkg;
}

static _Bool module__file_is_py(const char * file, size_t n)
{
    return file[n-3] == '.' &&
           file[n-2] == 'p' &&
           file[n-1] == 'y';
}

ti_module_t * ti_module_create(
        const char * name,
        size_t name_n,
        const char * file,
        size_t file_n,
        uint64_t created_at,
        ti_pkg_t * conf_pkg,    /* may be NULL */
        uint64_t * scope_id     /* may be NULL */)
{
    _Bool is_py_module = module__file_is_py(file, file_n);
    ti_module_t * module = malloc(sizeof(ti_module_t));
    if (!module)
        return NULL;

    module->ref = 1;
    module->status = TI_MODULE_STAT_NOT_LOADED;
    module->flags = is_py_module ? TI_MODULE_FLAG_IS_PY_MODULE : 0;
    module->restarts = 0;
    module->next_pid = 0;
    module->cb = (ti_module_cb) &module__cb;
    module->name = ti_names_get(name, name_n);
    module->file = fx_path_join_strn(
            ti.cfg->modules_path,
            strlen(ti.cfg->modules_path),
            file,
            file_n);
    module->args = malloc(sizeof(char*) * (is_py_module ? 3 : 2));
    module->conf_pkg = conf_pkg;
    module->started_at = 0;
    module->created_at = created_at;
    module->scope_id = scope_id;
    module->futures = omap_create();

    if (!module->name || !module->file || !module->futures || !module->args ||
        smap_add(ti.modules, module->name->str, module))
    {
        ti_module_drop(module);
        return NULL;
    }

    module->fn = module->file + (strlen(module->file) - file_n);

    if (is_py_module)
    {
        module->args[0] = ti.cfg->python_interpreter;
        module->args[1] = module->file;
        module->args[2] = NULL;
    }
    else
    {
        module->args[0] = module->file;
        module->args[1] = NULL;
    }

    ti_proc_init(&module->proc, module);

    return module;
}

int ti_module_validate_file(const char * file, size_t file_n, ex_t * e)
{
    const char * pt = file;

    if (!file_n)
    {
        ex_set(e, EX_VALUE_ERROR,
                "file argument must not be an empty string"DOC_NEW_MODULE);
        return e->nr;
    }

    if (!strx_is_printablen((const char *) file, file_n))
    {
        ex_set(e, EX_VALUE_ERROR,
                "file argument contains illegal characters"DOC_NEW_MODULE);
        return e->nr;
    }


    if (*file == '/')
    {
        ex_set(e, EX_VALUE_ERROR,
                "file argument must not start with a `/`"DOC_NEW_MODULE);
        return e->nr;
    }

    for (size_t i = 1; i < file_n; ++i, ++pt)
    {
        if (*pt == '.' && (file[i] == '.' || file[i] == '/'))
        {
            ex_set(e, EX_VALUE_ERROR,
                    "file argument must not contain `..` or `./` to specify "
                    "a relative path"DOC_NEW_MODULE);
            return e->nr;
        }
    }
    return 0;
}

static void module__conf(ti_module_t * module)
{
    if (!module->conf_pkg)
    {
        module->flags &= ~TI_MODULE_FLAG_WAIT_CONF;
        log_debug("no configuration found for module `%s`", module->name->str);
        return;
    }

    module->status = module__write_conf(module);

    if (module->status == TI_MODULE_STAT_RUNNING)
        log_info("wrote configuration to module `%s`", module->name->str);
    else
        log_error(
                "failed to write configuration to module `%s` (%s): %s",
                module->name->str,
                module->file,
                ti_module_status_str(module));
}

void ti_module_load(ti_module_t * module)
{
    if (module->flags & TI_MODULE_FLAG_IN_USE)
    {
        log_debug(
                "module `%s` already loaded (PID %d)",
                module->name->str, module->proc.process.pid);
        return;
    }

    module->flags |= TI_MODULE_FLAG_WAIT_CONF;
    module->status = ti_proc_load(&module->proc);

    if (module->status == TI_MODULE_STAT_RUNNING)
    {
        log_info(
                "loaded module `%s` (%s)",
                module->name->str,
                module->file);
        module__conf(module);
    }
    else
    {
        log_error(
                "failed to start module `%s` (%s): %s",
                module->name->str,
                module->file,
                ti_module_status_str(module));
    }
}

void ti_module_restart(ti_module_t * module)
{
    log_info("restarting module `%s`...", module->name->str);
    if (module->flags & TI_MODULE_FLAG_IN_USE)
    {
        module->flags |= TI_MODULE_FLAG_RESTARTING;
        (void) ti_module_stop(module);
    }
    else
    {
        ti_module_load(module);
    }
}

void ti_module_update_conf(ti_module_t * module)
{
    if (module->flags & TI_MODULE_FLAG_IN_USE)
        module__conf(module);
    else
        ti_module_load(module);
}

void ti_module_destroy(ti_module_t * module)
{
    if (!module)
        return;
    omap_destroy(module->futures, (omap_destroy_cb) ti_future_cancel);
    ti_val_drop((ti_val_t *) module->name);
    free(module->file);
    free(module->args);
    free(module->conf_pkg);
    free(module->scope_id);
    free(module);
}

void ti_module_on_exit(ti_module_t * module)
{
    /* first cancel all open futures */
    ti_module_cancel_futures(module);

    if (module->flags & TI_MODULE_FLAG_DESTROY)
    {
        ti_module_drop(module);
        return;
    }

    module->flags &= ~TI_MODULE_FLAG_IN_USE;

    if (module->flags & TI_MODULE_FLAG_RESTARTING)
    {
        module->restarts = 0;
        module->flags &= ~TI_MODULE_FLAG_RESTARTING;
        goto restart;
    }

    if (module->status == TI_MODULE_STAT_RUNNING)
    {
        if (++module->restarts > MODULE__TOO_MANY_RESTARTS)
        {
            log_error(
                    "module `%s` has been restarted too many (>%d) times",
                    module->name->str,
                    MODULE__TOO_MANY_RESTARTS);
            module->status = TI_MODULE_STAT_TOO_MANY_RESTARTS;
            return;
        }
        goto restart;
    }

    if (module->status == TI_MODULE_STAT_STOPPING)
    {
        module->status = TI_MODULE_STAT_NOT_LOADED;
        return;
    }

restart:
    module->status = TI_MODULE_STAT_NOT_LOADED;
    ti_module_load(module);
}

int ti_module_stop(ti_module_t * module)
{
    int rc, pid = module->proc.process.pid;
    if (!pid)
        return 0;
    rc = uv_kill(pid, SIGTERM);
    if (rc)
    {
        log_error(
                "failed to stop module `%s` (%s)",
                module->name->str,
                uv_strerror(rc));
        module->status = rc;
    }
    else
        module->status = TI_MODULE_STAT_STOPPING;
    return rc;
}

void ti_module_stop_and_destroy(ti_module_t * module)
{
    module->flags |= TI_MODULE_FLAG_DESTROY;
    if (module->flags & TI_MODULE_FLAG_IN_USE)
        (void) ti_module_stop(module);
    else
        ti_module_drop(module);
}

void module__stop_cb(uv_async_t * task)
{
    ti_module_t * module = task->data;
    ti_module_stop_and_destroy(module);
    uv_close((uv_handle_t *) task, (uv_close_cb) free);
}

void ti_module_del(ti_module_t * module)
{
    uv_async_t * task;

    (void) smap_pop(ti.modules, module->name->str);

    task = malloc(sizeof(uv_async_t));
    if (task && uv_async_init(ti.loop, task, module__stop_cb) == 0)
    {
        task->data = module;
        (void) uv_async_send(task);
    }
}

void ti_module_cancel_futures(ti_module_t * module)
{
    omap_clear(module->futures, (omap_destroy_cb) ti_future_cancel);
}

static void module__on_res(ti_future_t * future, ti_pkg_t * pkg)
{
    ex_t e = {0};
    ti_val_t * val;

    if (ti_future_should_load(future))
    {
        mp_unp_t up;
        ti_vup_t vup = {
                .isclient = true,
                .collection = future->query->collection,
                .up = &up,
        };
        mp_unp_init(&up, pkg->data, pkg->n);
        val = ti_val_from_vup_e(&vup, &e);
    }
    else if (!mp_is_valid(pkg->data, pkg->n))
    {
        ex_set(&e, EX_BAD_DATA,
                "got invalid or corrupt MsgPack data from module: `%s`",
                future->module->name->str);
        val = NULL;
    }
    else
        val = (ti_val_t *) ti_mp_create(pkg->data, pkg->n);

    if (!val)
    {
        if (e.nr == 0)
            ex_set_mem(&e);
        ti_query_on_future_result(future, &e);
        return;
    }

    ti_val_unsafe_drop(vec_set(future->args, val, 0));
    future->rval = (ti_val_t *) ti_varr_from_vec(future->args);
    if (!future->rval)
        ex_set_mem(&e);
    else
        future->args = NULL;

    ti_query_on_future_result(future, &e);
}

static void module__on_err(ti_future_t * future, ti_pkg_t * pkg)
{
    ex_t e = {0};
    mp_unp_t up;
    mp_obj_t mp_obj, mp_err_code, mp_err_msg;

    mp_unp_init(&up, pkg->data, pkg->n);

    if (mp_next(&up, &mp_obj) != MP_ARR || mp_obj.via.sz != 2 ||
        mp_next(&up, &mp_err_code) != MP_I64 ||
        mp_next(&up, &mp_err_msg) != MP_STR)
    {
        ex_set(&e, EX_BAD_DATA,
                "invalid error data from module `%s`",
                future->module->name->str);
        goto done;
    }

    if (mp_err_code.via.i64 < EX_MIN_ERR ||
        mp_err_code.via.i64 > EX_MAX_BUILD_IN_ERR)
    {
        ex_set(&e, EX_BAD_DATA,
            "invalid error code (%"PRId64") received from module `%s`",
            mp_err_code.via.i64,
            future->module->name->str);
        goto done;
    }

    if (ti_verror_check_msg(mp_err_msg.via.str.data, mp_err_msg.via.str.n, &e))
        goto done;

    ex_setn(&e,
            mp_err_code.via.i64,
            mp_err_msg.via.str.data,
            mp_err_msg.via.str.n);

done:
    ti_query_on_future_result(future, &e);
}

void ti_module_on_pkg(ti_module_t * module, ti_pkg_t * pkg)
{
    ti_future_t * future;

    switch(pkg->tp)
    {
    case TI_PROTO_MODULE_CONF_OK:
        log_info("module `%s` is successfully configured", module->name->str);
        module->status &= ~TI_MODULE_STAT_CONFIGURATION_ERR;
        module->flags &= ~TI_MODULE_FLAG_WAIT_CONF;
        return;
    case TI_PROTO_MODULE_CONF_ERR:
        log_info("failed to configure module `%s`", module->name->str);
        module->status = TI_MODULE_STAT_CONFIGURATION_ERR;
        module->flags &= ~TI_MODULE_FLAG_WAIT_CONF;
        return;
    }

    future = omap_rm(module->futures, pkg->id);
    if (!future)
    {
        log_error(
                "got a response for future id %u but a future with this id "
                "does not exist; maybe the future has been cancelled?",
                pkg->id);
        return;
    }

    switch(pkg->tp)
    {
    case TI_PROTO_MODULE_RES:
        module__on_res(future, pkg);
        return;
    case TI_PROTO_MODULE_ERR:
        module__on_err(future, pkg);
        return;
    default:
    {
        ex_t e;
        ex_set(&e, EX_BAD_DATA,
                "unexpected package type `%s` (%u) from module `%s`",
                ti_proto_str(pkg->tp), pkg->tp, module->name->str);
        ti_query_on_future_result(future, &e);
    }
    }
}

const char * ti_module_status_str(ti_module_t * module)
{
    switch (module->status)
    {
    case TI_MODULE_STAT_RUNNING:
        return "running";
    case TI_MODULE_STAT_NOT_LOADED:
        return "module not loaded";
    case TI_MODULE_STAT_STOPPING:
        return "stopping module...";
    case TI_MODULE_STAT_TOO_MANY_RESTARTS:
        return "too many restarts detected; most likely the module is broken";
    case TI_MODULE_STAT_PY_INTERPRETER_NOT_FOUND:
        return "the Python interpreter is not found on this node"
                DOC_NODE_INFO""DOC_CONFIGURATION;
    case TI_MODULE_STAT_CONFIGURATION_ERR:
        return "configuration error; "
               "use `set_module_conf(..)` to update the module configuration";
    }
    return uv_strerror(module->status);
}

int ti_module_info_to_pk(ti_module_t * module, msgpack_packer * pk, int flags)
{
    size_t sz = 5 + \
            !!(flags & TI_MODULE_FLAG_WITH_CONF) + \
            !!(flags & TI_MODULE_FLAG_WITH_TASKS) + \
            !!(flags & TI_MODULE_FLAG_WITH_RESTARTS);
    return -(
        msgpack_pack_map(pk, sz)||

        mp_pack_str(pk, "name") ||
        mp_pack_strn(pk, module->name->str, module->name->n) ||

        mp_pack_str(pk, "file") ||
        mp_pack_str(pk, module->file) ||

        mp_pack_str(pk, "created_at") ||
        msgpack_pack_uint64(pk, module->created_at) ||

        ((flags & TI_MODULE_FLAG_WITH_CONF) && (
                mp_pack_str(pk, "conf") ||
                (module->conf_pkg
                        ? mp_pack_append(
                            pk,
                            module->conf_pkg + sizeof(ti_pkg_t),
                            module->conf_pkg->n)
                        : msgpack_pack_nil(pk)))) ||

        mp_pack_str(pk, "status") ||
        mp_pack_str(pk, ti_module_status_str(module)) ||

        ((flags & TI_MODULE_FLAG_WITH_TASKS) && (
                mp_pack_str(pk, "tasks") ||
                msgpack_pack_uint64(pk, module->futures->n))) ||

        ((flags & TI_MODULE_FLAG_WITH_RESTARTS) && (
                mp_pack_str(pk, "restarts") ||
                msgpack_pack_uint16(pk, module->restarts))) ||

        mp_pack_str(pk, "scope") ||
        (module->scope_id
                ? mp_pack_str(pk, ti_scope_name_from_id(*module->scope_id))
                : msgpack_pack_nil(pk))
    );
}

ti_val_t * ti_module_as_mpval(ti_module_t * module, int flags)
{
    ti_raw_t * raw;
    msgpack_packer pk;
    msgpack_sbuffer buffer;

    mp_sbuffer_alloc_init(&buffer, sizeof(ti_raw_t), sizeof(ti_raw_t));
    msgpack_packer_init(&pk, &buffer, msgpack_sbuffer_write);

    if (ti_module_info_to_pk(module, &pk, flags))
    {
        msgpack_sbuffer_destroy(&buffer);
        return NULL;
    }

    raw = (ti_raw_t *) buffer.data;
    ti_raw_init(raw, TI_VAL_MPDATA, buffer.size);

    return (ti_val_t *) raw;
}

int ti_module_write(ti_module_t * module, const void * data, size_t n)
{
    if (ti_module_is_py(module))
        return fx_write(module->file, data, n);

    if (fx_file_exist(module->file) && unlink(module->file))
    {
        log_errno_file("cannot delete file", errno, module->file);
        return -1;
    }

    if (fx_write(module->file, data, n))
        return -1;

    if (chmod(module->file, S_IRWXU))
    {
        log_errno_file("cannot make file executable", errno, module->file);
        return -1;
    }

    return 0;
}

int ti_module_read_args(
        ti_thing_t * thing,
        _Bool * load,
        uint8_t * deep,
        ex_t * e)
{
    ti_name_t * deep_name = (ti_name_t *) ti_val_borrow_deep_name();
    ti_name_t * load_name = (ti_name_t *) ti_val_borrow_load_name();
    ti_val_t * deep_val = ti_thing_val_weak_by_name(thing, deep_name);
    ti_val_t * load_val = ti_thing_val_weak_by_name(thing, load_name);

    if (deep_val)
    {
        int64_t deepi;

        if (!ti_val_is_int(deep_val))
        {
            ex_set(e, EX_TYPE_ERROR,
                    "expecting `deep` to be of type `"TI_VAL_INT_S"` "
                    "but got type `%s` instead"DOC_FUTURE,
                    ti_val_str(deep_val));
            return e->nr;
        }

        deepi = VINT(deep_val);

        if (deepi < 0 || deepi > TI_MAX_DEEP_HINT)
        {
            ex_set(e, EX_VALUE_ERROR,
                    "expecting a `deep` value between 0 and %d "
                    "but got %"PRId64" instead",
                    TI_MAX_DEEP_HINT, deepi);
            return e->nr;
        }

        *deep = (uint8_t) deepi;
    }
    else
        *deep = 2;

    *load = load_val ? ti_val_as_bool(load_val) : false;

    return 0;
}

int ti_module_call(
        ti_module_t * module,
        ti_query_t * query,
        cleri_node_t * nd,
        ex_t * e)
{
    assert (!query->rval);

    const int nargs = fn_get_nargs(nd);
    _Bool load;
    uint8_t deep;
    cleri_children_t * child = nd->children;
    ti_future_t * future;

    if (module->scope_id && *module->scope_id != ti_query_scope_id(query))
    {
        ex_set(e, EX_FORBIDDEN,
                "module `%s` is restricted to scope `%s`",
                module->name->str,
                ti_scope_name_from_id(*module->scope_id));
        return e->nr;
    }

    if (nargs < 1)
    {
        ex_set(e, EX_NUM_ARGUMENTS,
                "modules must be called using at least 1 argument "
                "but 0 were given"DOC_MODULES);
        return e->nr;
    }

    ti_incref(module);  /* take a reference to module */

    if (ti_do_statement(query, child->node, e))
        goto fail0;

    if (!ti_val_is_thing(query->rval))
    {
        ex_set(e, EX_TYPE_ERROR,
                "expecting the first module argument to be of "
                "type `"TI_VAL_THING_S"` but got type `%s` instead"DOC_MODULES,
                ti_val_str(query->rval));
        goto fail0;
    }

    if (ti_module_read_args((ti_thing_t *) query->rval, &load, &deep, e))
        goto fail0;

    future = ti_future_create(query, module, nargs, deep, load);
    if (!future)
    {
        ex_set_mem(e);
        goto fail0;
    }

    VEC_push(future->args, query->rval);
    query->rval = NULL;

    while ((child = child->next) && (child = child->next))
    {
        if (ti_do_statement(query, child->node, e))
            goto fail1;

        VEC_push(future->args, query->rval);
        query->rval = NULL;
    }

    if (ti_future_register(future))
    {
        ex_set_mem(e);
        goto fail1;
    }

    query->rval = (ti_val_t *) future;
    ti_decref(module);  /* the future is guaranteed to have at least
                           one reference */
    return e->nr;

fail1:
    ti_val_unsafe_drop((ti_val_t *) future);
fail0:
    ti_module_drop(module);
    return e->nr;
}
