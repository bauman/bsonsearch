#ifdef WITH_MODULES

#include <uthash.h>
#include "matcher-module-store.h"
#include "mongoc-matcher-op-modules.h"
#include "mongoc-matcher-op-modules-private.h"

uint32_t
_matcher_module_store_startup()
{
    uint32_t num_modules = 0;
    module_storage * s = NULL;
    //ADD EACH MODULE
    //--between-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_BETWEEN_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_BETWEEN;
    s->m_new = (void*)&matcher_module_between_startup;
    s->m_prep = NULL;
    s->m_search = (void*)&matcher_module_between_search;
    s->m_cleanup = NULL;
    s->m_destroy = (void*)&matcher_module_between_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
    //--sample-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_SAMPLE_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_SAMPLE;
    s->m_new = (void*)&matcher_module_sample_startup;
    s->m_prep = NULL;
    s->m_search = (void*)&matcher_module_sample_search;
    s->m_cleanup = NULL;
    s->m_destroy = (void*)&matcher_module_sample_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#ifdef WITH_MATH
    //--sum-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_MATH_SUM_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_SUM;
    s->m_new = (void*)&matcher_module_math_sum_startup;
    s->m_prep = (void*)&matcher_module_math_sum_prep;
    s->m_search = (void*)&matcher_module_math_sum_search;
    s->m_cleanup = (void*)&matcher_module_math_sum_cleanup;
    s->m_destroy = (void*)&matcher_module_math_sum_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#endif /*WITH_MATH */
#ifdef WITH_ETHER
    //--ether-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_ETHER_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_ETHER;
    s->m_new = (void*)&matcher_module_ether_startup;
    s->m_prep = NULL;
    s->m_search = (void*)&matcher_module_ether_search;
    s->m_cleanup = NULL;
    s->m_destroy = (void*)&matcher_module_ether_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#endif/* WITH_ETHER */
#ifdef WITH_IP
    //--IP-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_IP_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_IP;
    s->m_new = (void*)&matcher_module_ip_startup;
    s->m_prep = NULL;
    s->m_search = (void*)&matcher_module_ip_search;
    s->m_cleanup = NULL;
    s->m_destroy = (void*)&matcher_module_ip_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#endif/* WITH_IP */
#ifdef WITH_DISCO
    //--sum-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_DISCO_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_DISCO;
    s->m_new = (void*)&matcher_module_disco_startup;
    s->m_prep = (void*)&matcher_module_disco_prep;
    s->m_search = (void*)&matcher_module_disco_search;
    s->m_cleanup = (void*)&matcher_module_disco_cleanup;
    s->m_destroy = (void*)&matcher_module_disco_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#endif /*WITH_DISCO */
#ifdef WITH_DUKJS
    //--sum-----------------------------------------------
    s =  (module_storage*)bson_malloc0(sizeof(*s));
    s->name = bson_strdup(MODULE_DUKJS_COMMAND);
    s->opcode = MATCHER_MODULE_OPCODE_DUKJS;
    s->m_new = (void*)&matcher_module_duk_startup;
    s->m_prep = (void*)&matcher_module_duk_prep;
    s->m_search = (void*)&matcher_module_duk_search;
    s->m_cleanup = (void*)&matcher_module_duk_cleanup;
    s->m_destroy = (void*)&matcher_module_duk_destroy;
    HASH_ADD_KEYPTR(hh, global_module_storage_NAME, s->name, strlen(s->name), s);
    num_modules +=1;
    //--------------------------------------------------------
#endif /*WITH_DUKJS */

    return num_modules;
}

uint32_t
_matcher_module_store_shutdown(){
    uint8_t freed = 0;
    module_storage *s, *tmp;
    HASH_ITER(hh, global_module_storage_NAME, s, tmp) {
        HASH_DEL(global_module_storage_NAME, s);
        bson_free(s->name);
        free(s);
        freed++;
    }
    return freed;
}
uint32_t
_matcher_module_store_count(){
    uint8_t num_modules = 0;
    module_storage *s, *tmp;
    HASH_ITER(hh, global_module_storage_NAME, s, tmp) {
        num_modules++;
    }
    return num_modules;
}


#endif
