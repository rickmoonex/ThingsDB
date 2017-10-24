/*
 * auth.h
 *
 *  Created on: Oct 5, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifndef RQL_AUTH_H_
#define RQL_AUTH_H_

#define RQL_AUTH_MASK_FULL ULLONG_MAX

typedef struct rql_auth_s rql_auth_t;

#include <limits.h>
#include <inttypes.h>
#include <rql/user.h>

rql_auth_t * rql_auth_new(rql_user_t * user, uint64_t mask);

struct rql_auth_s
{
    rql_user_t * user;  /* does not take a reference */
    uint64_t mask;
};

#endif /* RQL_AUTH_H_ */