/*
 * ti/proto.h
 */
#ifndef TI_PROTO_H_
#define TI_PROTO_H_

typedef enum
{
    /*
     * protocol definition for client connections
     */

    /*
     * 0..15 fire and forgets from client to node
     */

    /*
     * 16..32 fire and forgets from node to client
     */
    TI_PROTO_CLIENT_WATCH_INI   =16,    /* {event:x, thing: {#:x, ...}      */
    TI_PROTO_CLIENT_WATCH_UPD   =17,    /* {event:x. #:x, jobs:[] etc }     */
    TI_PROTO_CLIENT_WATCH_DEL   =18,    /* {#:x}                            */
    TI_PROTO_CLIENT_NODE_STATUS =19,    /* str_status                       */

    /*
     * 32..63 client requests
     */
    TI_PROTO_CLIENT_REQ_PING    =32,    /* empty                            */
    TI_PROTO_CLIENT_REQ_AUTH    =33,    /* [user, pass] or token            */
    TI_PROTO_CLIENT_REQ_QUERY_NODE          =34,    /* {query:...}   */
    TI_PROTO_CLIENT_REQ_QUERY_THINGSDB      =35,    /* {query:...}   */
    TI_PROTO_CLIENT_REQ_QUERY_COLLECTION    =36,    /* { collection:..
                                                         query:..
                                                         blobs: []
                                                         deep: 0..0x7f,
                                                         all: true/false
                                                       }
                                                     */
    TI_PROTO_CLIENT_REQ_WATCH   =48,    /* {collection:.. things: []}       */
    TI_PROTO_CLIENT_REQ_UNWATCH =49,    /* {collection:.. things: []}       */
    TI_PROTO_CLIENT_REQ_CALL    =50,    /* [target, procedure, arguments..] */
    /*
     * 64..95 client responses
     */
    TI_PROTO_CLIENT_RES_PING    =64,    /* empty */
    TI_PROTO_CLIENT_RES_AUTH    =65,    /* empty */
    TI_PROTO_CLIENT_RES_QUERY   =66,    /* [{}, {}, ...] */
    TI_PROTO_CLIENT_RES_WATCH   =80,    /* empty */
    TI_PROTO_CLIENT_RES_UNWATCH =81,    /* empty */

    /*
     * 96..127 client errors
     */

    /* integer overflow error */
    TI_PROTO_CLIENT_ERR_OVERFLOW    =96,
    /* zero division or module error */
    TI_PROTO_CLIENT_ERR_ZERO_DIV    =97,
    /* max quota is reached */
    TI_PROTO_CLIENT_ERR_MAX_QUOTA   =98,
    /* authentication failed or request without authentication */
    TI_PROTO_CLIENT_ERR_AUTH        =99,
    /* no access for the requested task */
    TI_PROTO_CLIENT_ERR_FORBIDDEN   =100,
    /* some index is not found */
    TI_PROTO_CLIENT_ERR_INDEX       =101,
    /* invalid request, incorrect package type, invalid QPack data */
    TI_PROTO_CLIENT_ERR_BAD_REQUEST =102,
    /* syntax error in query */
    TI_PROTO_CLIENT_ERR_SYNTAX      =103,
    /* node is (currently) unable to respond to the request */
    TI_PROTO_CLIENT_ERR_NODE        =104,
    /* assertion statement has failed */
    TI_PROTO_CLIENT_ERR_ASSERTION   =105,
    /* internal server error, for example allocation error */
    TI_PROTO_CLIENT_ERR_INTERNAL    =127,

    /*
     * protocol definition for node connections
     */

    /*
     * 128..159 node fire and forgets
     */
    TI_PROTO_NODE_EVENT         =158,   /* event */
    TI_PROTO_NODE_INFO          =159,   /* [...] */

    /*
     * 160..191 node requests
     */

    /* expects a client response which will be forwarded back to the client */
    TI_PROTO_NODE_REQ_QUERY     =162,   /* [user_id, is_db, {query...}] */

    TI_PROTO_NODE_REQ_CONNECT   =176,   /* [...] */
    TI_PROTO_NODE_REQ_EVENT_ID  =177,   /* event id */
    TI_PROTO_NODE_REQ_AWAY      =178,   /* empty */
    TI_PROTO_NODE_REQ_SETUP     =180,   /* empty */
    TI_PROTO_NODE_REQ_SYNC      =181,   /* event_id */

    /* [target_id, file_id, offset, bytes, more]
     * more is a boolean which is set to true in case the file is not yet
     * complete.
     */
    TI_PROTO_NODE_REQ_SYNCFPART =182,   /* full sync part */
    TI_PROTO_NODE_REQ_SYNCFDONE =183,   /* full sync completed */
    TI_PROTO_NODE_REQ_SYNCAPART =184,   /* archive sync part */
    TI_PROTO_NODE_REQ_SYNCADONE =185,   /* archive sync completed */
    TI_PROTO_NODE_REQ_SYNCEPART =186,   /* event sync part */
    TI_PROTO_NODE_REQ_SYNCEDONE =187,   /* event sync completed */

    /*
     * 192..223 node responses
     */
    TI_PROTO_NODE_RES_CONNECT   =208,   /* [node_id, status] */
    TI_PROTO_NODE_RES_EVENT_ID  =209,   /* empty, event id accepted */
    TI_PROTO_NODE_RES_AWAY      =210,   /* empty, away id accepted */
    TI_PROTO_NODE_RES_SETUP     =212,   /* ti_data */
    TI_PROTO_NODE_RES_SYNC      =213,   /* empty */
    TI_PROTO_NODE_RES_SYNCFPART =214,   /* [target, file, offset]
                                           here offset is 0 in case no more
                                           data for the file is required
                                         */
    TI_PROTO_NODE_RES_SYNCFDONE =215,   /* empty, ack */
    TI_PROTO_NODE_RES_SYNCAPART =216,   /* [first, last, offset]
                                           here
                                           here offset is 0 in case no more
                                           data for the file is required
                                         */
    TI_PROTO_NODE_RES_SYNCADONE =217,   /* empty, ack */
    TI_PROTO_NODE_RES_SYNCEPART =218,   /* event */
    TI_PROTO_NODE_RES_SYNCEDONE =219,   /* empty, ack */


    /*
     * 224..255 node errors
     */
    TI_PROTO_NODE_ERR_RES           =240,   /* message */
    TI_PROTO_NODE_ERR_EVENT_ID      =241,   /* empty */
    TI_PROTO_NODE_ERR_AWAY          =242,   /* empty */

} ti_proto_e;

#define TI_PROTO_NODE_REQ_QUERY_TIMEOUT 120
#define TI_PROTO_NODE_REQ_CONNECT_TIMEOUT 5
#define TI_PROTO_NODE_REQ_EVENT_ID_TIMEOUT 60
#define TI_PROTO_NODE_REQ_AWAY_ID_TIMEOUT 5
#define TI_PROTO_NODE_REQ_SETUP_TIMEOUT 10
#define TI_PROTO_NODE_REQ_SYNC_TIMEOUT 10
#define TI_PROTO_NODE_REQ_SYNCFPART_TIMEOUT 10
#define TI_PROTO_NODE_REQ_SYNCFDONE_TIMEOUT 300
#define TI_PROTO_NODE_REQ_SYNCAPART_TIMEOUT 10
#define TI_PROTO_NODE_REQ_SYNCADONE_TIMEOUT 300
#define TI_PROTO_NODE_REQ_SYNCEPART_TIMEOUT 10
#define TI_PROTO_NODE_REQ_SYNCEDONE_TIMEOUT 300

const char * ti_proto_str(ti_proto_e tp);

#endif /* TI_PROTO_H_ */
