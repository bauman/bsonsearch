#ifdef WITH_MODULES
#ifndef MATCHER_MODULE_SAMPLE_H
#define MATCHER_MODULE_SAMPLE_H
#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"

#define MODULE_SAMPLE_COMMAND "sample"

bool
matcher_module_sample_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_sample_destroy(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_sample_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);


typedef struct _matcher_container_sample_holder_t matcher_container_sample_holder_t;
struct _matcher_container_sample_holder_t {
    double sample;
};




#endif /* MATCHER_MODULE_SAMPLE_H */
#endif /* WITH_MODULES */