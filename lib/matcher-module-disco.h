#ifdef WITH_MODULES
#ifdef WITH_DISCO
#ifndef MATCHER_MODULE_DISCO_H
#define MATCHER_MODULE_DISCO_H
#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"
#include "discodb.h"

#define MODULE_DISCO_COMMAND "disco"
#define MODULE_DISCO_HAS_NONE 0x00
#define MODULE_DISCO_HAS_ACTION 0x01
#define MODULE_DISCO_HAS_DDB 0x02
#define MODULE_DISCO_G2G 0x03

bool
matcher_module_disco_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_disco_destroy(mongoc_matcher_op_t *op);

void *
matcher_module_disco_prep(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_disco_cleanup(mongoc_matcher_op_t *op, void * usermem);

mongoc_matcher_module_callback_t
matcher_module_disco_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);


typedef enum {
    MATCHER_MODULE_DISCO_UNDEFINED,
    MATCHER_MODULE_DISCO_KEY_EXISTS,
    MATCHER_MODULE_DISCO_VALUE_IS,
    MATCHER_MODULE_DISCO_VALUE_ONLY,
    MATCHER_MODULE_DISCO_VALUE_MATCHES_Q,
    MATCHER_MODULE_DISCO_UNKNOWN,
    MATCHER_MODULE_DISCO_INVALID,
} mongoc_matcher_module_disco_compare_t;


typedef struct _matcher_container_disco_holder_t matcher_container_disco_holder_t;
struct _matcher_container_disco_holder_t {
    uint8_t state;
    struct ddb * db;
    struct ddb_query_clause *clauses;
    const uint8_t * search;
    uint32_t search_len;
    uint32_t clauses_len;
    mongoc_matcher_module_disco_compare_t compare;
#ifdef ALLOW_FILESYSTEM
    int db_fd;
#endif
    char * precache_data;
    uint64_t precache_data_len;
    bool precache;
    bson_type_t type;
};

typedef struct _matcher_container_disco_usermem_t matcher_container_disco_usermem_t;
struct _matcher_container_disco_usermem_t {
    struct ddb_entry kentry;
    const struct ddb_entry * ventry;
    bson_type_t type;
};



#endif /* MATCHER_MODULE_DISCO_H */
#endif /* WITH_DISCO */
#endif /* WITH_MODULES */