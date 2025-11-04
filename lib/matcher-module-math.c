#ifdef WITH_MODULES
#ifdef WITH_MATH

#include <bson/bson.h>
#include "matcher-module-math.h"





bool
matcher_module_math_sum_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool result = false;
    matcher_container_math_sum_t *md;
    md = (matcher_container_math_sum_t*) bson_malloc0(sizeof *md);
    md->compare = MATCHER_MODULE_MATH_UNDEFINED;
    while (bson_iter_next(config)) {
        const char * key = bson_iter_key (config);
        if (strcmp(key, "$gte") == 0) {
            md->compare = MATCHER_MODULE_MATH_GTE;
        } else if (strcmp(key, "$eq") == 0) {
            md->compare = MATCHER_MODULE_MATH_EQ;
        } else if (strcmp(key, "$lte") == 0) {
            md->compare = MATCHER_MODULE_MATH_LTE;
        } else if (strcmp(key, "$gt") == 0) {
            md->compare = MATCHER_MODULE_MATH_GT;
        } else if (strcmp(key, "$lt") == 0) {
            md->compare = MATCHER_MODULE_MATH_LT;
        } else if (strcmp(key, "$not") == 0) {
            md->compare = MATCHER_MODULE_MATH_NE;
        } else {
            md->compare = MATCHER_MODULE_MATH_UNKNOWN;
        }

        if (md->compare != MATCHER_MODULE_MATH_UNKNOWN && BSON_ITER_HOLDS_NUMBER(config)){
            result = true;
            bson_type_t btype = bson_iter_type(config);
            md->type = btype;
            switch (btype){
                case BSON_TYPE_INT32:{
                    md->target.i64 = (int64_t)bson_iter_int32(config);
                    break;
                }
                case BSON_TYPE_INT64:{
                    md->target.i64 = bson_iter_int64(config);
                    break;
                }
                case BSON_TYPE_DOUBLE:{
                    md->target.dbl = bson_iter_double(config);
                    break;
                }
                case BSON_TYPE_DECIMAL128:{
                    md->type = BSON_TYPE_UNDEFINED;
                    break;
                }
                default: {
                    md->type = BSON_TYPE_UNDEFINED;
                    break;
                }
            }
        }
    }
    op->module.config.container.module_data = (void*)md;
    return result;
}

void *
matcher_module_math_sum_prep(mongoc_matcher_op_t *op){
    matcher_container_math_sum_t *ud;
    ud = (matcher_container_math_sum_t*) bson_malloc0(sizeof *ud);
    return (void*)ud;
}

static bool matcher_load_int64(bson_iter_t * iter, bson_type_t expectation, int64_t *i){
    bool result = true;
    bson_type_t btype = bson_iter_type(iter);
    switch (btype){
        case BSON_TYPE_INT32:{
            *i += (int64_t)bson_iter_int32(iter);
            break;
        }
        case BSON_TYPE_INT64:{
            *i += bson_iter_int64(iter);
            break;
        }
        case BSON_TYPE_ARRAY:{
            bson_iter_t right_array;
            bson_iter_recurse(iter, &right_array);
            while (bson_iter_next(&right_array)) {
                matcher_load_int64(&right_array, expectation, i);
            }
            break;
        }
        default:
            break;
    }


    return result;
}

mongoc_matcher_module_callback_t
matcher_module_math_sum_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    matcher_container_math_sum_t *md, *ud;
    md = (matcher_container_math_sum_t*) op->module.config.container.module_data;
    ud = (matcher_container_math_sum_t*) usermem;
    if (iter){
        bson_type_t btype = bson_iter_type(iter);
        switch (btype){
            case BSON_TYPE_ARRAY:
            case BSON_TYPE_INT32:
            case BSON_TYPE_INT64:{
                matcher_load_int64(iter, md->type, &ud->target.i64);
                break;
            }
            default:
                break;
        }
    } else {
        bool matches = false;
        switch (md->compare){
            case MATCHER_MODULE_MATH_EQ:{
                matches = (ud->target.i64 == md->target.i64);
                break;
            }
            case MATCHER_MODULE_MATH_GTE:{
                matches = (ud->target.i64 >= md->target.i64);
                break;
            }
            case MATCHER_MODULE_MATH_LTE:{
                matches = (ud->target.i64 <= md->target.i64);
                break;
            }
            case MATCHER_MODULE_MATH_GT:{
                matches = (ud->target.i64 > md->target.i64);
                break;
            }
            case MATCHER_MODULE_MATH_LT:{
                matches = (ud->target.i64 < md->target.i64);
                break;
            }
            case MATCHER_MODULE_MATH_NE:{
                matches = (ud->target.i64 != md->target.i64);
                break;
            }
            default:
                break;
        }
        if (matches){
            cb = MATCHER_MODULE_CALLBACK_FOUND;
        }
    }
    return cb;
}
mongoc_matcher_module_callback_t
matcher_module_math_sum_cleanup(mongoc_matcher_op_t *op, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    bson_free(usermem);
    return cb;
}
bool
matcher_module_math_sum_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_container_math_sum_t *md;
    md = (matcher_container_math_sum_t *) op->module.config.container.module_data;
    bson_free(md);
    return result;
}


#endif /* WITH_MATH */
#endif /* WITH_MODULES */