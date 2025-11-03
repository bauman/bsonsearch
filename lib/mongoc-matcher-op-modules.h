#ifdef WITH_MODULES


#ifndef MONGOC_MATCHER_OP_MODULES_H
#define MONGOC_MATCHER_OP_MODULES_H


#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-modules-private.h"


BSON_BEGIN_DECLS
#define MODULE_HAS_NONE 0
#define MODULE_HAS_NAME 1
#define MODULE_HAS_CONFIG 2
#define MODULE_HAS_ALL 3
BSON_END_DECLS


mongoc_matcher_op_t *
_mongoc_matcher_op_module_new     ( const char              *path,   /* IN */
                                    bson_iter_t             *child);   /* IN */

void
_mongoc_matcher_op_module_destroy (mongoc_matcher_op_t *op);

bool
_mongoc_matcher_op_module_config(mongoc_matcher_op_t *op,
                                 bson_iter_t         *config);
bool
_mongoc_matcher_op_module_match(mongoc_matcher_op_t *op, const bson_t * bson);

uint8_t
_mongoc_matcher_op_module_startup ();

uint8_t
_mongoc_matcher_op_module_shutdown ();


typedef bool (*MATCHER_NEW_CALLBACK)(void *op, void *config);
typedef int (*MATCHER_SEARCH_CALLBACK)(void *op, void *iter, void* usrmem);
typedef int (*MATCHER_DESTROY_CALLBACK)(mongoc_matcher_op_t *op);
typedef int (*MATCHER_CLEANUP_CALLBACK)(mongoc_matcher_op_t *op, void* usrmem);
typedef int* (*MATCHER_PREPARE_CALLBACK)(mongoc_matcher_op_t *op);

typedef struct _module_storage module_storage;
struct _module_storage {
    char * name;
    mongoc_matcher_module_opcode_t opcode;
    MATCHER_NEW_CALLBACK m_new; //called once when the matcher is generated
    MATCHER_PREPARE_CALLBACK m_prep; //called once to start each BSON object
    MATCHER_SEARCH_CALLBACK m_search; //called for each namespace found
    MATCHER_CLEANUP_CALLBACK m_cleanup; //called once to end each BSON object
    MATCHER_DESTROY_CALLBACK m_destroy; //called once when matcher is destroyed
    UT_hash_handle hh;
};

extern module_storage *global_module_storage_NAME;


#endif /* MONGOC_MATCHER_OP_MODULES_H */
#endif /* WITH_MODULES */