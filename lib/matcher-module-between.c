#ifdef WITH_MODULES

#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"
#include "matcher-module-between.h"


bool
matcher_module_between_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool high = false;
    bool low=false;
    bool result;
    matcher_container_between_t *bt;
    bt = (matcher_container_between_t*) bson_malloc0(sizeof *bt);
    while (bson_iter_next(config)) {
        const char * key = bson_iter_key (config);
        if (BSON_ITER_HOLDS_NUMBER(config)){
            if (strcmp(key, "high") == 0){
                high = _mongoc_matcher_op_near_cast_number_to_double(config, &bt->high);
            } else if (strcmp(key, "low") == 0) {
                low = _mongoc_matcher_op_near_cast_number_to_double(config, &bt->low);
            }
        }
    }
    result = high && low;
    if (!result){
        bson_free(bt);
    } else {
        op->module.config.container.module_data = (void*)bt;
    }
    return result;
}

mongoc_matcher_module_callback_t
matcher_module_between_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    if (iter){
        double number_to_check = 0;
        matcher_container_between_t *bt;
        bt = (matcher_container_between_t *) op->module.config.container.module_data;
        if (BSON_ITER_HOLDS_NUMBER(iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(iter, &number_to_check)){
            if (bt->high >= number_to_check && number_to_check >= bt->low){
                return MATCHER_MODULE_CALLBACK_FOUND;
            }
        }
    }
    return MATCHER_MODULE_CALLBACK_CONTINUE;
}

bool
matcher_module_between_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_container_between_t *bt;
    bt = (matcher_container_between_t *) op->module.config.container.module_data;
    bson_free(bt);
    return result;
}

#endif