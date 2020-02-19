/*
 * doc.h
 */
#ifndef DOC_H_
#define DOC_H_

#include <ti/version.h>

#define DOC_DOCS(__uri) \
    "https://docs.thingsdb.net/"TI_VERSION_SYNTAX_STR"/"__uri
#define DOC_SEE(__uri) \
    "; see "DOC_DOCS(__uri)

/* Collection API */
#define DOC_ASSERT                  DOC_SEE("collection-api/assert")
#define DOC_BASE64_ENCODE           DOC_SEE("collection-api/base64_encode")
#define DOC_BASE64_DECODE           DOC_SEE("collection-api/base64_decode")
#define DOC_BOOL                    DOC_SEE("collection-api/bool")
#define DOC_BYTES                   DOC_SEE("collection-api/bytes")
#define DOC_DEEP                    DOC_SEE("collection-api/deep")
#define DOC_DEL_TYPE                DOC_SEE("collection-api/del_type")
#define DOC_ERR                     DOC_SEE("collection-api/err")
#define DOC_FLOAT                   DOC_SEE("collection-api/float")
#define DOC_HAS_TYPE                DOC_SEE("collection-api/has_type")
#define DOC_IF                      DOC_SEE("collection-api/if")
#define DOC_INT                     DOC_SEE("collection-api/int")
#define DOC_ISARRAY                 DOC_SEE("collection-api/isarray")
#define DOC_ISASCII                 DOC_SEE("collection-api/isascii")
#define DOC_ISBOOL                  DOC_SEE("collection-api/isbool")
#define DOC_ISBYTES                 DOC_SEE("collection-api/isbytes")
#define DOC_ISERR                   DOC_SEE("collection-api/iserr")
#define DOC_ISFLOAT                 DOC_SEE("collection-api/isfloat")
#define DOC_ISINF                   DOC_SEE("collection-api/isinf")
#define DOC_ISINT                   DOC_SEE("collection-api/isint")
#define DOC_ISLIST                  DOC_SEE("collection-api/islist")
#define DOC_ISNAN                   DOC_SEE("collection-api/isnan")
#define DOC_ISNIL                   DOC_SEE("collection-api/isnil")
#define DOC_ISRAW                   DOC_SEE("collection-api/israw")
#define DOC_ISSET                   DOC_SEE("collection-api/isset")
#define DOC_ISSTR                   DOC_SEE("collection-api/isstr")
#define DOC_ISTHING                 DOC_SEE("collection-api/isthing")
#define DOC_ISTUPLE                 DOC_SEE("collection-api/istuple")
#define DOC_ISUTF8                  DOC_SEE("collection-api/isutf8")
#define DOC_LIST                    DOC_SEE("collection-api/list")
#define DOC_MOD_TYPE                DOC_SEE("collection-api/mod_type")
#define DOC_MOD_TYPE_ADD            DOC_SEE("collection-api/mod_type/add")
#define DOC_MOD_TYPE_DEL            DOC_SEE("collection-api/mod_type/del")
#define DOC_MOD_TYPE_MOD            DOC_SEE("collection-api/mod_type/mod")
#define DOC_NEW                     DOC_SEE("collection-api/new")
#define DOC_NEW_TYPE                DOC_SEE("collection-api/new_type")
#define DOC_NOW                     DOC_SEE("collection-api/now")
#define DOC_RAISE                   DOC_SEE("collection-api/raise")
#define DOC_RAND                    DOC_SEE("collection-api/rand")
#define DOC_RANDINT                 DOC_SEE("collection-api/randint")
#define DOC_REFS                    DOC_SEE("collection-api/refs")
#define DOC_RETURN                  DOC_SEE("collection-api/return")
#define DOC_SET                     DOC_SEE("collection-api/set")
#define DOC_SET_TYPE                DOC_SEE("collection-api/set_type")
#define DOC_STR                     DOC_SEE("collection-api/str")
#define DOC_THING                   DOC_SEE("collection-api/thing")
#define DOC_TRY                     DOC_SEE("collection-api/try")
#define DOC_TYPE                    DOC_SEE("collection-api/type")
#define DOC_TYPE_COUNT              DOC_SEE("collection-api/type_count")
#define DOC_TYPE_INFO               DOC_SEE("collection-api/type_info")
#define DOC_TYPES_INFO              DOC_SEE("collection-api/types_info")
#define DOC_WSE                     DOC_SEE("collection-api/wse")

/* Node API */
#define DOC_BACKUP_INFO             DOC_SEE("node-api/backup_info")
#define DOC_BACKUPS_INFO            DOC_SEE("node-api/backups_info")
#define DOC_COUNTERS                DOC_SEE("node-api/counters")
#define DOC_DEL_BACKUP              DOC_SEE("node-api/del_backup")
#define DOC_HAS_BACKUP              DOC_SEE("node-api/has_backup")
#define DOC_NEW_BACKUP              DOC_SEE("node-api/new_backup")
#define DOC_NODE_INFO               DOC_SEE("node-api/node_info")
#define DOC_NODES_INFO              DOC_SEE("node-api/nodes_info")
#define DOC_RESET_COUNTERS          DOC_SEE("node-api/reset_counters")
#define DOC_SET_LOG_LEVEL           DOC_SEE("node-api/set_log_level")
#define DOC_SHUTDOWN                DOC_SEE("node-api/shutdown")

/* ThingsDB API */
#define DOC_COLLECTION_INFO         DOC_SEE("thingsdb-api/collection_info")
#define DOC_COLLECTIONS_INFO        DOC_SEE("thingsdb-api/collections_info")
#define DOC_DEL_COLLECTION          DOC_SEE("thingsdb-api/del_collection")
#define DOC_DEL_EXPIRED             DOC_SEE("thingsdb-api/del_expired")
#define DOC_DEL_NODE                DOC_SEE("thingsdb-api/del_node")
#define DOC_DEL_TOKEN               DOC_SEE("thingsdb-api/del_token")
#define DOC_DEL_USER                DOC_SEE("thingsdb-api/del_user")
#define DOC_GRANT                   DOC_SEE("thingsdb-api/grant")
#define DOC_HAS_COLLECTION          DOC_SEE("thingsdb-api/has_collection")
#define DOC_HAS_NODE                DOC_SEE("thingsdb-api/has_node")
#define DOC_HAS_TOKEN               DOC_SEE("thingsdb-api/has_token")
#define DOC_HAS_USER                DOC_SEE("thingsdb-api/has_user")
#define DOC_NEW_COLLECTION          DOC_SEE("thingsdb-api/new_collection")
#define DOC_NEW_NODE                DOC_SEE("thingsdb-api/new_node")
#define DOC_NEW_TOKEN               DOC_SEE("thingsdb-api/new_token")
#define DOC_NEW_USER                DOC_SEE("thingsdb-api/new_user")
#define DOC_RENAME_COLLECTION       DOC_SEE("thingsdb-api/rename_collection")
#define DOC_RENAME_USER             DOC_SEE("thingsdb-api/rename_user")
#define DOC_REVOKE                  DOC_SEE("thingsdb-api/revoke")
#define DOC_SET_PASSWORD            DOC_SEE("thingsdb-api/set_password/")
#define DOC_USER_INFO               DOC_SEE("thingsdb-api/user_info")
#define DOC_USERS_INFO              DOC_SEE("thingsdb-api/users_info")

/* Procedures API */
#define DOC_DEL_PROCEDURE           DOC_SEE("procedures-api/del_procedure")
#define DOC_HAS_PROCEDURE           DOC_SEE("procedures-api/has_procedure")
#define DOC_NEW_PROCEDURE           DOC_SEE("procedures-api/new_procedure")
#define DOC_PROCEDURE_DOC           DOC_SEE("procedures-api/procedure_doc")
#define DOC_PROCEDURE_INFO          DOC_SEE("procedures-api/procedure_info")
#define DOC_PROCEDURES_INFO         DOC_SEE("procedures-api/procedures_info")
#define DOC_RUN                     DOC_SEE("procedures-api/run")

/* Data Types */
#define DOC_BYTES_LEN               DOC_SEE("data-types/bytes/len")
#define DOC_CLOSURE_CALL            DOC_SEE("data-types/closure/call")
#define DOC_CLOSURE_DOC             DOC_SEE("data-types/closure/doc")
#define DOC_ERROR_CODE              DOC_SEE("data-types/error/code")
#define DOC_ERROR_MSG               DOC_SEE("data-types/error/msg")
#define DOC_LIST_CHOICE             DOC_SEE("data-types/list/choice")
#define DOC_LIST_EVERY              DOC_SEE("data-types/list/every")
#define DOC_LIST_EXTEND             DOC_SEE("data-types/list/extend")
#define DOC_LIST_FILTER             DOC_SEE("data-types/list/filter")
#define DOC_LIST_FIND               DOC_SEE("data-types/list/find")
#define DOC_LIST_FINDINDEX          DOC_SEE("data-types/list/findindex")
#define DOC_LIST_INDEXOF            DOC_SEE("data-types/list/indexof")
#define DOC_LIST_LEN                DOC_SEE("data-types/list/len")
#define DOC_LIST_MAP                DOC_SEE("data-types/list/map")
#define DOC_LIST_POP                DOC_SEE("data-types/list/pop")
#define DOC_LIST_PUSH               DOC_SEE("data-types/list/push")
#define DOC_LIST_REDUCE             DOC_SEE("data-types/list/reduce")
#define DOC_LIST_REMOVE             DOC_SEE("data-types/list/remove")
#define DOC_LIST_SOME               DOC_SEE("data-types/list/some")
#define DOC_LIST_SORT               DOC_SEE("data-types/list/sort")
#define DOC_LIST_SPLICE             DOC_SEE("data-types/list/splice")
#define DOC_SET_ADD                 DOC_SEE("data-types/set/add/")
#define DOC_SET_FILTER              DOC_SEE("data-types/set/filter")
#define DOC_SET_FIND                DOC_SEE("data-types/set/find")
#define DOC_SET_HAS                 DOC_SEE("data-types/set/has")
#define DOC_SET_LEN                 DOC_SEE("data-types/set/len")
#define DOC_SET_MAP                 DOC_SEE("data-types/set/map")
#define DOC_SET_REMOVE              DOC_SEE("data-types/set/remove")
#define DOC_STR_CONTAINS            DOC_SEE("data-types/str/contains")
#define DOC_STR_ENDSWITH            DOC_SEE("data-types/str/endswith")
#define DOC_STR_LEN                 DOC_SEE("data-types/str/len")
#define DOC_STR_LOWER               DOC_SEE("data-types/str/lower")
#define DOC_STR_STARTSWITH          DOC_SEE("data-types/str/startswith")
#define DOC_STR_TEST                DOC_SEE("data-types/str/test")
#define DOC_STR_UPPER               DOC_SEE("data-types/str/upper")
#define DOC_THING_DEL               DOC_SEE("data-types/thing/del")
#define DOC_THING_FILTER            DOC_SEE("data-types/thing/filter")
#define DOC_THING_GET               DOC_SEE("data-types/thing/get")
#define DOC_THING_HAS               DOC_SEE("data-types/thing/has")
#define DOC_THING_ID                DOC_SEE("data-types/thing/id")
#define DOC_THING_KEYS              DOC_SEE("data-types/thing/keys")
#define DOC_THING_LEN               DOC_SEE("data-types/thing/len")
#define DOC_THING_MAP               DOC_SEE("data-types/thing/map")
#define DOC_THING_SET               DOC_SEE("data-types/thing/set")
#define DOC_THING_UNWATCH           DOC_SEE("data-types/thing/unwatch")
#define DOC_THING_VALUES            DOC_SEE("data-types/thing/values")
#define DOC_THING_WATCH             DOC_SEE("data-types/thing/watch")
#define DOC_THING_WRAP              DOC_SEE("data-types/thing/wrap")
#define DOC_WTYPE_UNWRAP            DOC_SEE("data-types/wtype/unwrap")

/* Errors */
#define DOC_ASSERT_ERR              DOC_SEE("errors/assert_err")
#define DOC_AUTH_ERR                DOC_SEE("errors/auth_err")
#define DOC_BAD_DATA_ERR            DOC_SEE("errors/bad_data_err")
#define DOC_FORBIDDEN_ERR           DOC_SEE("errors/forbidden_err")
#define DOC_LOOKUP_ERR              DOC_SEE("errors/lookup_err")
#define DOC_MAX_QUOTA_ERR           DOC_SEE("errors/max_quota_err")
#define DOC_NODE_ERR                DOC_SEE("errors/node_err")
#define DOC_NUM_ARGUMENTS_ERR       DOC_SEE("errors/num_arguments_err")
#define DOC_OPERATION_ERR           DOC_SEE("errors/operation_err")
#define DOC_OVERFLOW_ERR            DOC_SEE("errors/overflow_err")
#define DOC_SYNTAX_ERR              DOC_SEE("errors/syntax_err")
#define DOC_TYPE_ERR                DOC_SEE("errors/type_err")
#define DOC_VALUE_ERR               DOC_SEE("errors/value_err")
#define DOC_ZERO_DIV_ERR            DOC_SEE("errors/zero_div_err")

/* Other */
#define DOC_NAMES                   DOC_SEE("overview/names")
#define DOC_SOCKET_QUERY            DOC_SEE("connect/socket/query")
#define DOC_SCOPES                  DOC_SEE("overview/scopes")
#define DOC_SLICES                  DOC_SEE("overview/slices")
#define DOC_SPEC                    DOC_SEE("data-types/type")
#define DOC_WATCHING                DOC_SEE("watching")

/* No functions */
#define DOC_PROCEDURES_API          DOC_SEE("procedures-api")
#define DOC_CLOSURE                 DOC_SEE("data-types/closure")
#define DOC_HTTP_API                DOC_SEE("connect/http-api")

#endif  /* TI_DOC_H_ */
