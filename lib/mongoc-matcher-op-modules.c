#ifdef WITH_MODULES
#include <bson/bson.h>
#include <uthash.h>
#include "mongoc-matcher-op-modules.h"
#include "mongoc-matcher-op-private.h"
#include "matcher-module-store.h"
#include "mongoc-bson-descendants.h"

module_storage *global_module_storage_NAME = NULL;

mongoc_matcher_op_t *
_mongoc_matcher_op_module_new     ( const char              *path,   /* IN */
                                    bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_t *op = NULL;
    uint8_t proper_format = MODULE_HAS_NONE;
    module_storage * s = NULL;
    bool good_config = false;
    bson_iter_t module_internal_config;


    op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *op);
    op->base.opcode = MONGOC_MATCHER_OPCODE_MODULE;
    op->module.path = bson_strdup(path);
    op->module.config.base.opcode = MATCHER_MODULE_OPCODE_UNDEFINED;

    switch (bson_iter_type ((child))) {
        case BSON_TYPE_DOCUMENT: {
            bson_iter_t module_config_iter;
            if (bson_iter_recurse (child, &module_config_iter)) {
                while (bson_iter_next(&module_config_iter)) {
                    const char * key = bson_iter_key (&module_config_iter);
                    if (BSON_ITER_HOLDS_UTF8(&module_config_iter) &&
                            strcmp(key, "name") ==0) {
                        uint32_t name_len = 0;
                        const char * name = bson_iter_utf8(&module_config_iter, &name_len);
                        proper_format |= MODULE_HAS_NAME;
                        op->module.config.base.name = bson_strdup(name);
                        HASH_FIND_STR(global_module_storage_NAME, (char*)name, s);
                        if (s){
                            op->module.config.base.opcode = s->opcode;
                        }
                    } else if (BSON_ITER_HOLDS_DOCUMENT(&module_config_iter) && strcmp(key, "config") == 0 ) {
                        if (bson_iter_recurse(&module_config_iter, &module_internal_config)) {
                            proper_format |= MODULE_HAS_CONFIG;
                        }
                    }
                }
            }
            break;
        }
        default:
         break;
    }
    if(proper_format == MODULE_HAS_ALL){
        good_config = _mongoc_matcher_op_module_config(op, &module_internal_config);
        if (!good_config){
            op->module.config.base.opcode = MATCHER_MODULE_OPCODE_UNKNOWN;
        }
    }
    return op;
}
bool
_mongoc_matcher_op_module_config(mongoc_matcher_op_t *op,
                                 bson_iter_t         *config) {
    bool result = false;

    module_storage * s = NULL;
    HASH_FIND_STR(global_module_storage_NAME, op->module.config.base.name, s);
    if (s){
        if (s->m_new){
            result = s->m_new(op, config);
        }
    }
    return result;
}

bool
_mongoc_matcher_op_module_match(mongoc_matcher_op_t *op, const bson_t * bson){
    BSON_ASSERT (op);
    BSON_ASSERT (bson);
    bool found_one = false;
    void* usermem = NULL;
    bson_iter_t tmp;
    bson_iter_t iter;
    int checked = 0, skip=0;
    mongoc_matcher_module_callback_t module_callback_response = MATCHER_MODULE_CALLBACK_CONTINUE;
    module_storage * s = NULL;
    HASH_FIND_STR(global_module_storage_NAME, (char*)op->module.config.base.name, s);
    if (s) {
        if (s->m_prep){
            usermem = (void *)s->m_prep(op);
        }
        if (s->m_search) {
            if (strchr(op->module.path, '.')) {
                if (!bson_iter_init(&tmp, bson) ||
                    !bson_iter_find_descendant(&tmp, op->module.path, &iter)) { //try this way first
                    while (!found_one &&
                           module_callback_response == MATCHER_MODULE_CALLBACK_CONTINUE &&
                           bson_iter_init(&tmp, bson) &&
                           bson_iter_find_descendants(&tmp, op->module.path, &skip, &iter)) {
                        if (module_callback_response == MATCHER_MODULE_CALLBACK_CONTINUE){
                            module_callback_response = s->m_search(op, &iter, usermem);
                        }
                        skip = ++checked;
                    }
                    if (module_callback_response == MATCHER_MODULE_CALLBACK_CONTINUE){
                        module_callback_response = s->m_search(op, NULL, usermem);
                    }
                    if (module_callback_response == MATCHER_MODULE_CALLBACK_FOUND) { found_one = true; }
                    if (s->m_cleanup){ s->m_cleanup(op, usermem); }
                    return ((checked > 0) && found_one);
                }
            } else if (!bson_iter_init_find(&iter, bson, op->module.path)) {
                if (s->m_cleanup){ s->m_cleanup(op, usermem); }
                return false;
            }
            module_callback_response = s->m_search(op, &iter, usermem);
            if (module_callback_response == MATCHER_MODULE_CALLBACK_CONTINUE){
                module_callback_response = s->m_search(op, NULL, usermem);
            }
            if (module_callback_response == MATCHER_MODULE_CALLBACK_FOUND) { found_one = true; }
        }
        if (s->m_cleanup){
            s->m_cleanup(op, usermem);
        }
    }
    return found_one;
}



void
_mongoc_matcher_op_module_destroy (mongoc_matcher_op_t *op){
    module_storage * s = NULL;
    HASH_FIND_STR(global_module_storage_NAME, op->module.config.base.name, s);
    if (s){
        if (s->m_destroy){
            s->m_destroy(op);
        }
    }
    bson_free(op->module.path);
    bson_free(op->module.config.base.name);

}
uint8_t
_mongoc_matcher_op_module_shutdown (){
    //may need some other shutdown things in this context later.
    return _matcher_module_store_shutdown();
}

uint8_t
_mongoc_matcher_op_module_startup (){
    bool result = true;
    result &= _matcher_module_store_startup();
    return result;
}

#endif /* WITH_MODULES */