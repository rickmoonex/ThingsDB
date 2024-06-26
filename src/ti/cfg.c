/*
 * ti/cfg.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ti/cfg.h>
#include <ti/stream.h>
#include <util/cfgparser.h>
#include <util/logger.h>
#include <util/strx.h>
#include <ti/tcp.h>
#include <ti.h>
#include <util/fx.h>

static ti_cfg_t * cfg;
static const char * cfg__section = "thingsdb";

static void cfg__port(
        cfgparser_t * parser,
        const char * cfg_file,
        const char * option_name,
        uint16_t * port)
{
    const int min_ = 0;
    const int max_ = 65535;

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < min_ ||
            option->val->integer > max_)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting a value between %d and %d), "
                "using default value %u",
                option_name,
                cfg_file,
                min_,
                max_,
                *port);
        return;
    }

    *port = (uint16_t) option->val->integer;
}

static void cfg__zone(
        cfgparser_t * parser,
        const char * cfg_file,
        uint8_t * zone)
{
    const int min_ = 0;
    const int max_ = 255;

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, "zone");

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < min_ ||
            option->val->integer > max_)
    {
        log_warning(
                "error reading `zone` in `%s` "
                "(expecting a value between %d and %d), "
                "using default value %u",
                cfg_file,
                min_,
                max_,
                *zone);
        return;
    }

    *zone = (uint8_t) option->val->integer;
}

static void cfg__shutdown_period(
        cfgparser_t * parser,
        const char * cfg_file,
        uint8_t * shutdown_period)
{
    const int min_ = 0;
    const int max_ = 255;

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, "shutdown_period");

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < min_ ||
            option->val->integer > max_)
    {
        log_warning(
                "error reading `shutdown_period` in `%s` "
                "(expecting a value between %d and %d), "
                "using default value %u",
                cfg_file,
                min_,
                max_,
                *shutdown_period);
        return;
    }

    *shutdown_period = (uint8_t) option->val->integer;
}

static void cfg__ip_support(cfgparser_t * parser, const char * cfg_file)
{
    const char * option_name = "ip_support";
    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);
    if (rc != CFGPARSER_SUCCESS)
        return;

    if (option->tp != CFGPARSER_TP_STRING)
    {
        log_warning(
                "error reading `%s` in `%s` (%s), "
                "using default value `%s`",
                option_name,
                cfg_file,
                "expecting a string value",
                ti_tcp_ip_support_str(cfg->ip_support));
        return;
    }

    if (ti_tcp_ip_support_int(option->val->string, &cfg->ip_support) == 0)
        return;

    log_warning(
            "error reading `%s` in `%s` "
            "(expecting ALL, IPV4ONLY or IPV6ONLY but got `%s`), "
            "using default value `%s`",
            "ip_support",
            cfg_file,
            option->val->string,
            ti_tcp_ip_support_str(cfg->ip_support));
}

static int cfg__str(
        cfgparser_t * parser,
        const char * cfg_file,
        const char * option_name,
        char ** str)
{
    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);
    if (rc != CFGPARSER_SUCCESS)
        return 0;

    if (option->tp != CFGPARSER_TP_STRING)
    {
        log_warning(
                "error reading `%s` in `%s` (expecting a string value), "
                "Using default value `%s`",
                option_name,
                cfg_file,
                *str ? *str : "disabled");
        return 0;
    }
    if (*option->val->string == '\0')
    {
        if (*str)
            log_warning(
                    "missing `%s` in `%s` (%s), "
                    "using default value `%s`",
                    option_name,
                    cfg_file,
                    cfgparser_errmsg(rc),
                    *str);
        return 0;
    }

    free(*str);
    *str = strdup(option->val->string);
    return *str ? 0 : -1;
}

static void cfg__threshold_full_storage(
        cfgparser_t * parser,
        const char * cfg_file)
{
    const char * option_name = "threshold_full_storage";

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < 0)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting an integer value greater than, or equal to 0), "
                "using default value %zu",
                option_name,
                cfg_file,
                cfg->threshold_full_storage);
        return;
    }

    cfg->threshold_full_storage = (size_t) option->val->integer;
}

static void cfg__threshold_query_cache(
        cfgparser_t * parser,
        const char * cfg_file)
{
    const char * option_name = "threshold_query_cache";

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < 0)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting an integer value greater than, or equal to 0), "
                "using default value %zu",
                option_name,
                cfg_file,
                cfg->threshold_query_cache);
        return;
    }

    cfg->threshold_query_cache = (size_t) option->val->integer;
}

static void cfg__cache_expiration_time(
        cfgparser_t * parser,
        const char * cfg_file)
{
    const char * option_name = "cache_expiration_time";

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < 0)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting an integer value greater than, or equal to 0), "
                "using default value %zu",
                option_name,
                cfg_file,
                cfg->cache_expiration_time);
        return;
    }

    cfg->cache_expiration_time = (size_t) option->val->integer;
}


static void cfg__result_size_limit(cfgparser_t * parser, const char * cfg_file)
{
    const char * option_name = "result_size_limit";

    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    if (    option->tp != CFGPARSER_TP_INTEGER ||
            option->val->integer < 0)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting an integer value greater than, or equal to 0), "
                "using default value %zu",
                option_name,
                cfg_file,
                cfg->result_size_limit);
        return;
    }

    cfg->result_size_limit = (size_t) option->val->integer;
}

static void cfg__duration(
        cfgparser_t * parser,
        const char * cfg_file,
        const char * option_name,
        double * duration)
{
    double d;
    cfgparser_option_t * option;
    cfgparser_return_t rc;
    rc = cfgparser_get_option(&option, parser, cfg__section, option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    d = option->tp == CFGPARSER_TP_INTEGER
            ? option->val->integer
            : option->tp == CFGPARSER_TP_REAL
            ? option->val->real
            : -1.0;
    if (d < 0)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting an value greater than, or equal to 0), "
                "using default value %f",
                option_name,
                cfg_file,
                *duration);
        return;
    }

    *duration = d;
}

static void cfg__bool(
        cfgparser_t * parser,
        const char * option_name,
        const char * cfg_file,
        _Bool * cfgbool)
{
    cfgparser_option_t * option;
    cfgparser_return_t rc;
    int32_t i;
    rc = cfgparser_get_option(
            &option,
            parser,
            cfg__section,
            option_name);

    if (rc != CFGPARSER_SUCCESS)
        return;

    i = option->tp == CFGPARSER_TP_INTEGER ? option->val->integer : -1;

    if (i < 0 || i > 1)
    {
        log_warning(
                "error reading `%s` in `%s` "
                "(expecting either 0 (disabled) or 1 (enabled), "
                "using default value %d (%s)",
                option_name,
                cfg_file,
                *cfgbool,
                *cfgbool ? "enabled" : "disabled");
    }

    *cfgbool = i;
}

int ti_cfg_create(void)
{
    /* SUSv2 guarantees that "Host names are limited to 255 bytes,
     * excluding terminating null byte" */
    char hostname[256];
    char * sysuser = getenv("USER");
    char * homedir = getenv("HOME");
    if (gethostname(hostname, sizeof(hostname)))
        return -1;

    cfg = malloc(sizeof(ti_cfg_t));
    if (!cfg)
        return -1;

    /* set defaults */
    cfg->client_port = TI_DEFAULT_CLIENT_PORT;
    cfg->node_port = TI_DEFAULT_NODE_PORT;
    cfg->http_api_port = TI_DEFAULT_HTTP_API_PORT;
    cfg->ws_port = TI_DEFAULT_WS_PORT;
    cfg->http_status_port = TI_DEFAULT_HTTP_STATUS_PORT;
    cfg->threshold_full_storage = TI_DEFAULT_THRESHOLD_FULL_STORAGE;
    cfg->result_size_limit = TI_DEFAULT_RESULT_DATA_LIMIT;
    cfg->threshold_query_cache = TI_DEFAULT_THRESHOLD_QUERY_CACHE;
    cfg->cache_expiration_time = TI_DEFAULT_CACHE_EXPIRATION_TIME;
    cfg->ip_support = AF_UNSPEC;
    cfg->bind_client_addr = strdup("127.0.0.1");
    cfg->bind_node_addr = strdup("127.0.0.1");
    cfg->storage_path = !homedir || !sysuser || strcmp(sysuser, "root") == 0
            ? strdup("/var/lib/thingsdb/")
            : fx_path_join(homedir, ".thingsdb/");
    cfg->modules_path = !homedir || !sysuser || strcmp(sysuser, "root") == 0
            ? strdup("/usr/lib/thingsdb-modules")
            : fx_path_join(homedir, ".thingsdb-modules/");
    cfg->wait_for_modules = 0;
    cfg->python_interpreter = strdup("python");
    cfg->gcloud_key_file = NULL;
    cfg->pipe_client_name = NULL;
    cfg->ws_cert_file = NULL;
    cfg->ws_key_file = NULL;
    cfg->zone = 0;
    cfg->shutdown_period = 6;
    cfg->query_duration_warn = 0;
    cfg->query_duration_error = 0;
    cfg->node_name = strdup(hostname);

    if (!cfg->bind_client_addr ||
        !cfg->bind_node_addr ||
        !cfg->storage_path ||
        !cfg->modules_path ||
        !cfg->python_interpreter ||
        !cfg->node_name)
        ti_cfg_destroy();

    ti.cfg = cfg;
    return 0;
}

void ti_cfg_destroy(void)
{
    if (!cfg)
        return;
    free(cfg->node_name);
    free(cfg->bind_client_addr);
    free(cfg->bind_node_addr);
    free(cfg->pipe_client_name);
    free(cfg->storage_path);
    free(cfg->modules_path);
    free(cfg->python_interpreter);
    free(cfg->gcloud_key_file);
    free(cfg->ws_cert_file);
    free(cfg->ws_key_file);
    free(cfg);
    cfg = ti.cfg = NULL;
}

int ti_cfg_parse(const char * cfg_file)
{
    int rc;
    cfgparser_t * parser = cfgparser_create();
    if (!parser)
    {
        printf("allocation error\n");
        return -1;
    }

    rc = cfgparser_read(parser, cfg_file);
    if (rc != CFGPARSER_SUCCESS)
    {
        /* we could choose to continue with defaults but this is probably
         * not what users want so lets quit.
         */
        printf("cannot not read `%s` (%s)\n", cfg_file, cfgparser_errmsg(rc));
        rc = -1;
        goto exit_parse;
    }

    if (    (rc = cfg__str(
                    parser,
                    cfg_file,
                    "node_name",
                    &cfg->node_name)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "storage_path",
                    &cfg->storage_path)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "bind_client_addr",
                    &cfg->bind_client_addr)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "bind_node_addr",
                    &cfg->bind_node_addr)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "pipe_client_name",
                    &cfg->pipe_client_name)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "modules_path",
                    &cfg->modules_path)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "python_interpreter",
                    &cfg->python_interpreter)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "gcloud_key_file",
                    &cfg->gcloud_key_file)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "ws_cert_file",
                    &cfg->ws_cert_file)) ||
            (rc = cfg__str(
                    parser,
                    cfg_file,
                    "ws_key_file",
                    &cfg->ws_key_file)))
        goto exit_parse;

    cfg__bool(parser, cfg_file, "wait_for_modules", &cfg->wait_for_modules);
    cfg__port(parser, cfg_file, "listen_client_port", &cfg->client_port);
    cfg__port(parser, cfg_file, "listen_node_port", &cfg->node_port);
    cfg__port(parser, cfg_file, "http_status_port", &cfg->http_status_port);
    cfg__port(parser, cfg_file, "http_api_port", &cfg->http_api_port);
    cfg__port(parser, cfg_file, "ws_port", &cfg->ws_port);
    cfg__zone(parser, cfg_file, &cfg->zone);
    cfg__shutdown_period(parser, cfg_file, &cfg->shutdown_period);
    cfg__ip_support(parser, cfg_file);
    cfg__threshold_full_storage(parser, cfg_file);
    cfg__result_size_limit(parser, cfg_file);
    cfg__threshold_query_cache(parser, cfg_file);
    cfg__cache_expiration_time(parser, cfg_file);
    cfg__duration(
            parser,
            cfg_file,
            "query_duration_warn",
            &cfg->query_duration_warn);
    cfg__duration(
            parser,
            cfg_file,
            "query_duration_error",
            &cfg->query_duration_error);

exit_parse:
    cfgparser_destroy(parser);
    return rc;
}

int ti_cfg_ensure_storage_path(void)
{
    size_t len;
    char * storage_path;

    if (!fx_is_dir(cfg->storage_path) &&
        mkdir(cfg->storage_path, FX_DEFAULT_DIR_ACCESS))
    {
        /* still one thread so `strerror` is fine to use here */
        printf(
            "cannot create directory `%s` (%s)",
            cfg->storage_path, strerror(errno)
        );
        return -1;
    }

    storage_path = realpath(cfg->storage_path, NULL);

    if (!storage_path)
    {
        printf("cannot find storage path `%s`\n", cfg->storage_path);
        return -1;
    }

    free(cfg->storage_path);
    cfg->storage_path = storage_path;

    /* add trailing slash (/) if its not already there */
    len = strlen(cfg->storage_path);

    if (!len || cfg->storage_path[len - 1] != '/')
    {
        char * tmp = malloc(len + 2);

        if (!tmp)
        {
            printf("allocation error\n");
            return -1;
        }

        memcpy(tmp, cfg->storage_path, len);
        tmp[len] = '/';
        tmp[len + 1] = '\0';

        free(cfg->storage_path);
        cfg->storage_path = tmp;
    }

    return 0;
}
