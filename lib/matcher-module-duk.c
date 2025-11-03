#ifdef WITH_MODULES
#ifdef WITH_DUKJS

#include <bson/bson.h>
#include "matcher-module-duk.h"


static bool
duk_extract_code(bson_iter_t * config, matcher_container_duk_holder_t *md){
    bool result = false;
    if (BSON_ITER_HOLDS_CODE(config)){
        md->code = (uint8_t *)bson_iter_code(config, &md->codelen);
        duk_push_lstring(md->ctx,
                         (const char *) md->code,
                         (duk_size_t) md->codelen);
        duk_int_t code_ready = duk_peval(md->ctx);
        if (code_ready  != 0) {
            md->compare = MATCHER_MODULE_DUK_INVALID;
            printf("Error: %s\n", duk_safe_to_string(md->ctx, -1));
        } else {
            md->compare = MATCHER_MODULE_DUK_ACCEPTED;
            duk_pop(md->ctx); /* ignore result */
        }
    }
    return result;
}

static mongoc_matcher_module_callback_t
duk_make_the_matches_call(matcher_container_duk_holder_t *md){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    if (duk_pcall(md->ctx, 1 /*nargs*/) != 0) {
        // I don't like introducing writing stderr.
        // no better ideas though :-(
        fprintf(stderr, "%s\n", duk_safe_to_string(md->ctx, -1));
        cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    } else {
        if (duk_get_boolean(md->ctx, -1)) {
            cb = MATCHER_MODULE_CALLBACK_FOUND;
        }
    }
    duk_pop(md->ctx);  /* pop result/error */
    return cb;
}


bool
matcher_module_duk_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool result = false;
    matcher_container_duk_holder_t *md;
    md = (matcher_container_duk_holder_t*) bson_malloc0(sizeof *md);
    BSON_ASSERT(md);
    md->compare = MATCHER_MODULE_DUK_UNDEFINED;
    md->ctx = duk_create_heap_default(); // This might still be null ... tf
    BSON_ASSERT(md->ctx);
    md->state = MODULE_DUKJS_HAS_NONE;
    while (bson_iter_next(config)) {
        const char *key = bson_iter_key(config);
        if (strcmp(key, "code") == 0) {
            result = duk_extract_code(config, md);
            md->state |= MODULE_DUKJS_HAS_CODE;
        } else if (strcmp(key, "entrypoint") == 0) {
            md->entrypoint = bson_iter_utf8(config, &md->entrypointlen);
            md->state |= MODULE_DUKJS_HAS_ENTRY;
        }
    }
    if (md->state != MODULE_DUKJS_G2G){
        md->compare = MATCHER_MODULE_DUK_INVALID;
    } else {
        result = true;
    }
    op->module.config.container.module_data = (void*)md;
    return result;
}

void *
matcher_module_duk_prep(mongoc_matcher_op_t *op){
    matcher_container_duk_usermem_t *ud; // user data
    ud = (matcher_container_duk_usermem_t*) bson_malloc0(sizeof *ud);
    BSON_ASSERT(ud);
    return (void*)ud;
}


static void
matcher_module_duk_free_son(matcher_container_duk_usermem_t* ud){
    if (ud->bson_data){
        bson_free(ud->bson_data);
        ud->bson_data = NULL;
    }
    if (ud->json_data){
        bson_free(ud->json_data);
        ud->json_data = NULL;
    }
}

mongoc_matcher_module_callback_t
matcher_module_duk_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    matcher_container_duk_holder_t *md;
    matcher_container_duk_usermem_t *ud;
    uint32_t binary_len;
    const uint8_t * binary = NULL;
    md = (matcher_container_duk_holder_t*) op->module.config.container.module_data;
    ud = (matcher_container_duk_usermem_t*) usermem;

    // DO THE SEARCH
    if (iter){
        bson_type_t btype = bson_iter_type(iter);
        duk_push_global_object(md->ctx);
        duk_get_prop_string(md->ctx, -1 /*index*/, md->entrypoint);

        //----------------------------------------------------------------------
        //  --Consider move to a "get_binary" static function?
        switch (btype){
            case BSON_TYPE_UTF8:{
                binary = (const uint8_t *) bson_iter_utf8(iter, &binary_len);
                duk_push_string(md->ctx, (const char*)binary);
                break;
            }
            case BSON_TYPE_DOCUMENT:{
                uint32_t          document_len;
                const uint8_t     *document;
                bson_iter_document(iter, &document_len, &document);
                ud->bson_data = bson_new_from_data(document, document_len);
                size_t json_len = 0;
                ud->json_data = bson_as_canonical_extended_json(ud->bson_data, &json_len);
                duk_push_string(md->ctx, ud->json_data);
                break;
            }
            default:
                break;
        }
        cb = duk_make_the_matches_call(md);
        matcher_module_duk_free_son(ud);
        //  --Consider move to a "value_only" static function?
        // ----------------------------------------------------------------------
    } else {
        // API will always provide a null iter at the end if module always calls back continue
        // Null iter means this is the last descendant in this document,
        // last chance to decide if it matches
    }
    return cb;
}


mongoc_matcher_module_callback_t
matcher_module_duk_cleanup(mongoc_matcher_op_t *op, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    matcher_container_duk_usermem_t *ud;
    ud = (matcher_container_duk_usermem_t*) usermem;
    matcher_module_duk_free_son(ud);
    bson_free(usermem);
    return cb;
}

bool
matcher_module_duk_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_container_duk_holder_t *md;
    md = (matcher_container_duk_holder_t *) op->module.config.container.module_data;
    if (md) {
        if (md->ctx){
            duk_destroy_heap(md->ctx);
            md->ctx = NULL;
        }
        bson_free(md);
        result = true;
    }
    return result;
}


#endif /* WITH_DUKJS */
#endif /* WITH_MODULES */