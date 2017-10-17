/*
 * sock.c
 *
 *  Created on: Oct 5, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <rql/sock.h>
#include <util/logger.h>

static void rql__sock_stop(uv_tcp_t * tcp);
static const char * rql__sock_addr(rql_sock_t * sock);
static const char * rql__unresolved = "unresolved";


rql_sock_t * rql_sock_create(rql_sock_e tp, rql_t * rql)
{
    rql_sock_t * sock = (rql_sock_t *) malloc(sizeof(rql_sock_t));
    if (!sock) return NULL;

    sock->ref = 1;
    sock->n = 0;
    sock->sz = 0;
    sock->tp = tp;
    sock->flags = 0;
    sock->rql = rql;
    sock->via.user = NULL;
    sock->cb = NULL;
    sock->buf = NULL;
    sock->addr_ = NULL;

    sock->tcp.data = sock;

    return sock;
}

rql_sock_t * rql_sock_grab(rql_sock_t * sock)
{
    sock->ref++;
    return sock;
}

void rql_sock_drop(rql_sock_t * sock)
{
    if (sock && !--sock->ref)
    {
        assert (~sock->flags & RQL_SOCK_FLAG_INIT);
        free(sock->buf);
        free(sock->addr_);
        free(sock);
    }
}

int rql_sock_init(rql_sock_t * sock)
{
    int rc = uv_tcp_init(&sock->rql->loop, &sock->tcp);
    if (!rc)
    {
        sock->flags |= RQL_SOCK_FLAG_INIT;
    }
    return rc;
}

/*
 * Call rqlsock_closr
 */
void rql_sock_close(rql_sock_t * sock)
{
    assert (sock->flags & RQL_SOCK_FLAG_INIT);
    sock->n = 0; /* prevents quick looping allocation function */
    sock->flags &= ~RQL_SOCK_FLAG_INIT;
    uv_close(
            (uv_handle_t *) &sock->tcp,
            (uv_close_cb) rql__sock_stop);
}

void rql_sock_alloc_buf(uv_handle_t * handle, size_t sugsz, uv_buf_t * buf)
{
    rql_sock_t * sock  = (rql_sock_t *) handle->data;

    if (sock->n == 0 && sock->sz != sugsz)
    {
        free(sock->buf);
        sock->buf = (char *) malloc(sugsz);
        if (sock->buf == NULL)
        {
            log_error(EX_ALLOC);
            buf->base = NULL;
            buf->len = 0;
            return;
        }
        sock->sz = sugsz;
        sock->n = 0;
    }

    buf->base = sock->buf + sock->n;
    buf->len = sock->sz - sock->n;
}

void rql_sock_on_data(uv_stream_t * clnt, ssize_t n, const uv_buf_t * buf)
{
    rql_sock_t * sock  = (rql_sock_t *) clnt->data;
    rql_pkg_t * pkg;
    size_t total_sz;

    if (~sock->flags & RQL_SOCK_FLAG_INIT) return;

    if (n < 0)
    {
        if (n != UV_EOF) log_error(uv_strerror(n));
        rql_sock_close(sock);
        return;
    }

    sock->n += n;
    if (sock->n < sizeof(rql_pkg_t)) return;

    pkg = (rql_pkg_t *) sock->buf;
    if (!rql_pkg_check(pkg))
    {
        log_error("invalid package from '%s'; closing connection",
                rql_sock_addr(sock));
        rql_sock_close(sock);
        return;
    }

    total_sz = sizeof(rql_pkg_t) + pkg->n;

    if (sock->n < total_sz)
    {
        if (sock->sz < total_sz)
        {
            char * tmp = realloc(sock->buf, total_sz);
            if (!tmp)
            {
                log_error(EX_ALLOC);
                rql_sock_close(sock);
                return;
            }
            sock->buf = tmp;
            sock->sz = total_sz;
        }
        return;
    }

    sock->cb(sock, pkg);

    sock->n -= total_sz;
    if (sock->n > 0)
    {
        /* move data and call rql_sock_on_data() again */
        memmove(sock->buf, sock->buf + total_sz, sock->n);
        rql_sock_on_data(clnt, 0, buf);
    }
}

const char * rql_sock_ip_support_str(uint8_t ip_support)
{
    switch (ip_support)
    {
    case AF_UNSPEC: return "ALL";
    case AF_INET: return "IPV4ONLY";
    case AF_INET6: return "IPV6ONLY";
    default: return "UNKNOWN";
    }
}

const char * rql_sock_addr(rql_sock_t * sock)
{
    return (sock->addr_) ? sock->addr_ : rql__sock_addr(sock);
}

static void rql__sock_stop(uv_tcp_t * tcp)
{
    rql_sock_t * sock = (rql_sock_t * ) tcp->data;
    switch (sock->tp)
    {
    case RQL_SOCK_BACK:
    case RQL_SOCK_FRONT:
        break;
    case RQL_SOCK_NODE:
        log_info("node disconnected: %s", rql_sock_addr(sock));
        rql_node_drop(sock->via.node);
        break;
    case RQL_SOCK_CLIENT:
        log_info("client disconnected: %s", rql_sock_addr(sock));
        rql_user_drop(sock->via.user);
        break;
    }
    rql_sock_drop(sock);
}

static const char * rql__sock_addr(rql_sock_t * sock)
{
    const size_t sz = 54;
    sock->addr_ = (char *) malloc(sz);
    if (!sock->addr_) return rql__unresolved;

    struct sockaddr_storage name;
    int n = sizeof(name);

    if (uv_tcp_getpeername(&sock->tcp, (struct sockaddr *) &name, &n))
    {
        return rql__unresolved;
    }

    switch (name.ss_family)
    {
    case AF_INET:
    {
        char addr[INET_ADDRSTRLEN];
        uv_inet_ntop(
                AF_INET,
                &((struct sockaddr_in *) &name)->sin_addr,
                addr,
                sizeof(addr));
        snprintf(
                sock->addr_,
                sz,
                "%s:%d",
                addr,
                ntohs(((struct sockaddr_in *) &name)->sin_port));
    } break;
    case AF_INET6:
    {
        char addr[INET6_ADDRSTRLEN];
        uv_inet_ntop(
                AF_INET6,
                &((struct sockaddr_in6 *) &name)->sin6_addr,
                addr,
                sizeof(addr));
        snprintf(
                sock->addr_,
                sz,
                "[%s]:%d",
                addr,
                ntohs(((struct sockaddr_in6 *) &name)->sin6_port));
    } break;
    default:
        return rql__unresolved;
    }

    return sock->addr_;
}

