/*
 * back.h
 *
 *  Created on: Oct 5, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifndef RQL_BACK_H_
#define RQL_BACK_H_

typedef struct rql_back_s  rql_back_t;

#include <rql/rql.h>
#include <rql/sock.h>

struct rql_back_s
{
    rql_sock_t * sock;
};

rql_back_t * rql_back_create(rql_t * rql);
void rql_back_destroy(rql_back_t * back);
int rql_back_listen(rql_back_t * back);

#endif /* RQL_BACK_H_ */

