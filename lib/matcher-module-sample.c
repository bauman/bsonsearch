#ifdef WITH_MODULES

#include <time.h>
#include <stdlib.h>
#include <bson/bson.h>
#include "matcher-module-sample.h"


bool
matcher_module_sample_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool result = false;
    matcher_container_sample_holder_t *md;
    md = (matcher_container_sample_holder_t*) bson_malloc0(sizeof *md);
    srand(time(NULL));
    while (bson_iter_next(config)) {
        const char *key = bson_iter_key(config);
        if (strcmp(key, "$ratio") == 0 && BSON_ITER_HOLDS_DOUBLE(config)) {
            md->sample = bson_iter_double(config);
        }  else {
            // some other key, who knows... skip it
        }
    }
    op->module.config.container.module_data = (void*)md;
    return result;
}

mongoc_matcher_module_callback_t
matcher_module_sample_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    matcher_container_sample_holder_t *md;
    md = (matcher_container_sample_holder_t*) op->module.config.container.module_data;
    if (iter){
        if ((float)rand()/(float)(RAND_MAX) < md->sample){
            cb = MATCHER_MODULE_CALLBACK_FOUND;
        } else {
            cb = MATCHER_MODULE_CALLBACK_STOP;
        }
    } else {
        // API will always provide a null iter at the end if module always calls back continue
        // Null iter means this is the last descendant in this document,
        // last chance to decide if it matches
    }
    return cb;
}

bool
matcher_module_sample_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_container_sample_holder_t *md;
    md = (matcher_container_sample_holder_t *) op->module.config.container.module_data;
    bson_free(md);
    return result;
}


#endif /* WITH_MODULES */