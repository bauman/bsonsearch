#ifdef WITH_MODULES
#ifdef WITH_DUKJS
#ifndef MATCHER_MODULE_DUK_H
#define MATCHER_MODULE_DUK_H
#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"
#include "duktape.h"

#define MODULE_DUKJS_COMMAND  "dukjs"
#define MODULE_DUKJS_HAS_NONE  0x00
#define MODULE_DUKJS_HAS_CODE  0x01
#define MODULE_DUKJS_HAS_ENTRY 0x02
#define MODULE_DUKJS_G2G       0x03

bool
matcher_module_duk_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_duk_destroy(mongoc_matcher_op_t *op);

void *
matcher_module_duk_prep(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_duk_cleanup(mongoc_matcher_op_t *op, void * usermem);

mongoc_matcher_module_callback_t
matcher_module_duk_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);


typedef enum {
    MATCHER_MODULE_DUK_UNDEFINED,
    MATCHER_MODULE_DUK_ACCEPTED,
    MATCHER_MODULE_DUK_UNKNOWN,
    MATCHER_MODULE_DUK_INVALID,
} mongoc_matcher_module_duk_compare_t;


typedef struct _matcher_container_duk_holder_t matcher_container_duk_holder_t;
struct _matcher_container_duk_holder_t {
    mongoc_matcher_module_duk_compare_t compare;
    duk_context *ctx;
    const char * entrypoint;
    uint32_t entrypointlen;
    uint8_t * code;
    uint32_t codelen;
    uint8_t state;
};

typedef struct _matcher_container_duk_usermem_t matcher_container_duk_usermem_t;
struct _matcher_container_duk_usermem_t {
    bson_type_t type;
    bson_t * bson_data;
    char * json_data;
};



#endif /* MATCHER_MODULE_DISCO_H */
#endif /* WITH_DUKJS */
#endif /* WITH_MODULES */