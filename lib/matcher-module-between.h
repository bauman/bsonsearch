#ifdef WITH_MODULES
#ifndef MATCHER_MODULE_BETWEEN_H
#define MATCHER_MODULE_BETWEEN_H

#define MODULE_BETWEEN_COMMAND "between"

#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"
bool
matcher_module_between_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_between_destroy(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_between_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);

typedef struct _matcher_container_between_t matcher_container_between_t;
struct _matcher_container_between_t {
    double high;
    double low;
};

#endif /* MATCHER_MODULE_BETWEEN_H */
#endif /* WITH_MODULES */