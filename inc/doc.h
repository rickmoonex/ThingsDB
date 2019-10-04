/*
 * doc.h
 */
#ifndef DOC_H_
#define DOC_H_

/* TODO: create doc exist tests */

#include <tiinc.h>

#define DOC_DOCS(__hash) "https://thingsdb.github.io/ThingsDocs/"__hash
#define DOC_SEE(__hash) "; see "DOC_DOCS(__hash)

#define DOC_ADD                     DOC_SEE("#add")
#define DOC_ARRAY                   DOC_SEE("#array")
#define DOC_ASSERT                  DOC_SEE("#assert")
#define DOC_ASSERT_ERR              DOC_SEE("#assert_err")
#define DOC_AUTH_ERR                DOC_SEE("#auth_err")
#define DOC_BAD_DATA_ERR            DOC_SEE("#bad_data_err")
#define DOC_BOOL                    DOC_SEE("#bool")
#define DOC_CALL                    DOC_SEE("#call")
#define DOC_CLOSURE                 DOC_SEE("#closure")
#define DOC_COLLECTION_INFO         DOC_SEE("#collection_info")
#define DOC_COLLECTIONS_INFO        DOC_SEE("#collections_info")
#define DOC_CONTAINS                DOC_SEE("#contains")
#define DOC_COUNTERS                DOC_SEE("#counters")
#define DOC_DEEP                    DOC_SEE("#deep")
#define DOC_DEL                     DOC_SEE("#del")
#define DOC_DEL_COLLECTION          DOC_SEE("#del_collection")
#define DOC_DEL_EXPIRED             DOC_SEE("#del_expired")
#define DOC_DEL_NODE                DOC_SEE("#del_node")
#define DOC_DEL_PROCEDURE           DOC_SEE("#del_procedure")
#define DOC_DEL_TOKEN               DOC_SEE("#del_token")
#define DOC_DEL_TYPE                DOC_SEE("#del_type")
#define DOC_DEL_USER                DOC_SEE("#del_user")
#define DOC_ENDSWITH                DOC_SEE("#endswith")
#define DOC_ERR                     DOC_SEE("#err")
#define DOC_EXTEND                  DOC_SEE("#extend")
#define DOC_FILTER                  DOC_SEE("#filter")
#define DOC_FIND                    DOC_SEE("#find")
#define DOC_FINDINDEX               DOC_SEE("#findindex")
#define DOC_FLOAT                   DOC_SEE("#float")
#define DOC_FORBIDDEN_ERR           DOC_SEE("#forbidden_err")
#define DOC_GET                     DOC_SEE("#get")
#define DOC_GRANT                   DOC_SEE("#grant")
#define DOC_HAS_SET                 DOC_SEE("#has-set")
#define DOC_HAS_THING               DOC_SEE("#has-thing")
#define DOC_ID                      DOC_SEE("#id")
#define DOC_INDEXOF                 DOC_SEE("#indexof")
#define DOC_INT                     DOC_SEE("#int")
#define DOC_ISARRAY                 DOC_SEE("#isarray")
#define DOC_ISASCII                 DOC_SEE("#isascii")
#define DOC_ISBOOL                  DOC_SEE("#isbool")
#define DOC_ISERR                   DOC_SEE("#iserr")
#define DOC_ISFLOAT                 DOC_SEE("#isfloat")
#define DOC_ISINF                   DOC_SEE("#isinf")
#define DOC_ISINT                   DOC_SEE("#isint")
#define DOC_ISLIST                  DOC_SEE("#islist")
#define DOC_ISNAN                   DOC_SEE("#isnan")
#define DOC_ISNIL                   DOC_SEE("#isnil")
#define DOC_ISRAW                   DOC_SEE("#israw")
#define DOC_ISSET                   DOC_SEE("#isset")
#define DOC_ISTHING                 DOC_SEE("#isthing")
#define DOC_ISTUPLE                 DOC_SEE("#istuple")
#define DOC_ISUTF8                  DOC_SEE("#isutf8")
#define DOC_KEYS                    DOC_SEE("#keys")
#define DOC_LEN                     DOC_SEE("#len")
#define DOC_LOOKUP_ERR              DOC_SEE("#lookup_err")
#define DOC_LOWER                   DOC_SEE("#lower")
#define DOC_MAP                     DOC_SEE("#map")
#define DOC_MAX_QUOTA_ERR           DOC_SEE("#max_quota_err")
#define DOC_MOD_TYPE                DOC_SEE("#mod_type")
#define DOC_MOD_TYPE_ADD            DOC_SEE("#mod_type_add")
#define DOC_MOD_TYPE_DEL            DOC_SEE("#mod_typr_del")
#define DOC_MOD_TYPE_MOD            DOC_SEE("#type_mod_mod")
#define DOC_NAMES                   DOC_SEE("#names")
#define DOC_NEW                     DOC_SEE("#new")
#define DOC_NEW_COLLECTION          DOC_SEE("#new_collection")
#define DOC_NEW_NODE                DOC_SEE("#new_node")
#define DOC_NEW_PROCEDURE           DOC_SEE("#new_procedure")
#define DOC_NEW_TOKEN               DOC_SEE("#new_token")
#define DOC_NEW_TYPE                DOC_SEE("#new_type")
#define DOC_NEW_USER                DOC_SEE("#new_user")
#define DOC_NODE_ERR                DOC_SEE("#node_err")
#define DOC_NODE_INFO               DOC_SEE("#node_info")
#define DOC_NODES_INFO              DOC_SEE("#nodes_info")
#define DOC_NOW                     DOC_SEE("#now")
#define DOC_NUM_ARGUMENTS_ERR       DOC_SEE("#num_arguments_err")
#define DOC_OPERATION_ERR           DOC_SEE("#operation_err")
#define DOC_OVERFLOW_ERR            DOC_SEE("#overflow_err")
#define DOC_POP                     DOC_SEE("#pop")
#define DOC_PROCEDURE_DOC           DOC_SEE("#procedure_doc")
#define DOC_PROCEDURE_INFO          DOC_SEE("#procedure_info")
#define DOC_PROCEDURES_API          DOC_SEE("#procedures-api")
#define DOC_PROCEDURES_INFO         DOC_SEE("#procedures_info")
#define DOC_PUSH                    DOC_SEE("#push")
#define DOC_QUERY                   DOC_SEE("#query")
#define DOC_QUOTAS                  DOC_SEE("#quotas")
#define DOC_RAISE                   DOC_SEE("#raise")
#define DOC_REFS                    DOC_SEE("#refs")
#define DOC_REMOVE_LIST             DOC_SEE("#remove-list")
#define DOC_REMOVE_SET              DOC_SEE("#remove-set")
#define DOC_RENAME_COLLECTION       DOC_SEE("#rename_collection")
#define DOC_RENAME_USER             DOC_SEE("#rename_user")
#define DOC_RESET_COUNTERS          DOC_SEE("#reset_counters")
#define DOC_RETURN                  DOC_SEE("#return")
#define DOC_REVOKE                  DOC_SEE("#revoke")
#define DOC_RUN                     DOC_SEE("#run")
#define DOC_SCOPES                  DOC_SEE("#scopes")
#define DOC_SET_LOG_LEVEL           DOC_SEE("#set_log_level")
#define DOC_SET_NEW_TYPE            DOC_SEE("#set-new-type")
#define DOC_SET_PASSWORD            DOC_SEE("#set_password")
#define DOC_SET_PROPERTY            DOC_SEE("#set-property")
#define DOC_SET_QUOTA               DOC_SEE("#set_quota")
#define DOC_SHUTDOWN                DOC_SEE("#shutdown")
#define DOC_SLICES                  DOC_SEE("#slices")
#define DOC_SPEC                    DOC_SEE("#type-declaration")
#define DOC_SPLICE                  DOC_SEE("#splice")
#define DOC_STARTSWITH              DOC_SEE("#startswith")
#define DOC_STR                     DOC_SEE("#str")
#define DOC_SYNTAX_ERR              DOC_SEE("#syntax_err")
#define DOC_TEST                    DOC_SEE("#test")
#define DOC_THING                   DOC_SEE("#thing")
#define DOC_TRY                     DOC_SEE("#try")
#define DOC_TYPE                    DOC_SEE("#type")
#define DOC_TYPE_ERR                DOC_SEE("#type_err")
#define DOC_TYPE_COUNT              DOC_SEE("#type_count")
#define DOC_TYPE_INFO               DOC_SEE("#type_info")
#define DOC_TYPES                   DOC_SEE("#types")
#define DOC_TYPES_INFO              DOC_SEE("#types_info")
#define DOC_UNWRAP                  DOC_SEE("#unwrap")
#define DOC_UPPER                   DOC_SEE("#upper")
#define DOC_USER_INFO               DOC_SEE("#user_info")
#define DOC_USERS_INFO              DOC_SEE("#users_info")
#define DOC_VALUE_ERR               DOC_SEE("#value_err")
#define DOC_VALUES                  DOC_SEE("#values")
#define DOC_WATCH                   DOC_SEE("#watch")
#define DOC_WRAP                    DOC_SEE("#wrap")
#define DOC_WSE                     DOC_SEE("#wse")
#define DOC_ZERO_DIV_ERR            DOC_SEE("#zero_div_err")

#endif  /* TI_DOC_H_ */
