#ifdef WITH_MODULES
#ifdef WITH_MATH
#ifndef MATCHER_MODULE_MATH_H
#define MATCHER_MODULE_MATH_H
#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"

#define MODULE_MATH_SUM_COMMAND "sum"


bool
matcher_module_math_sum_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_math_sum_destroy(mongoc_matcher_op_t *op);

void *
matcher_module_math_sum_prep(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_math_sum_cleanup(mongoc_matcher_op_t *op, void * usermem);

mongoc_matcher_module_callback_t
matcher_module_math_sum_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);

typedef union _matcher_container_math_numbers_t matcher_container_math_numbers_t;
typedef struct _matcher_container_math_sum_t matcher_container_math_sum_t;
union _matcher_container_math_numbers_t {
    int64_t i64;
    double dbl;
    bson_decimal128_t d128;
};

typedef enum {
    MATCHER_MODULE_MATH_UNDEFINED,
    MATCHER_MODULE_MATH_EQ,
    MATCHER_MODULE_MATH_GTE,
    MATCHER_MODULE_MATH_GT,
    MATCHER_MODULE_MATH_LTE,
    MATCHER_MODULE_MATH_LT,
    MATCHER_MODULE_MATH_NE,
    MATCHER_MODULE_MATH_UNKNOWN,
} mongoc_matcher_module_math_compare_t;


struct _matcher_container_math_sum_t {
    matcher_container_math_numbers_t target;
    mongoc_matcher_module_math_compare_t compare;
    bson_type_t type;
};

#endif /* MATCHER_MODULE_MATH_H */
#endif /* WITH_MATH */
#endif /* WITH_MODULES */