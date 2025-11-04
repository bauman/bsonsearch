#ifdef WITH_MODULES
#ifdef WITH_ETHER
#include <bson/bson.h>
#include "matcher-module-ether.h"
#include "mongoc-matcher-op-private.h"
#include "uthash.h"



bool
matcher_module_ether_startup(mongoc_matcher_op_t * op, bson_iter_t * config)
{
    bool result = false;
    bool have_type = false;
    bool have_query = false;
    bson_iter_t query;
    matcher_container_ether_t *md;
    md = (matcher_container_ether_t*) bson_malloc0(sizeof *md);
    op->module.config.container.module_data = (void *) md;
    while (bson_iter_next(config)) {
        const char * key = bson_iter_key (config);
        if (BSON_ITER_HOLDS_UTF8(config) && strcmp(key, "type") == 0){
            uint32_t len;
            const char * type = bson_iter_utf8(config, &len);
            md->opcode = matcher_module_ether_get_opcode(type, len);
            have_type = true;
        } else if (BSON_ITER_HOLDS_ARRAY(config) && strcmp(key, "query") == 0){
            have_query = bson_iter_recurse (config, &query);
        }
    }
    result = have_type && have_query;
    if (have_type && have_query){
        result = matcher_module_ether_get_query(md, &query);
    }
    return result;
}

bool
matcher_module_ether_get_query(matcher_container_ether_t *md, bson_iter_t *query)
{
    bool result = true;
    switch (md->opcode){
        case MATCHER_ETHER_SOURCE: {
            md->i4 = bson_malloc0(sizeof (struct iphdr));
            struct in_addr target, mask;
            result &= matcher_module_ether_get_query_data(md, query, &target, &mask);
            md->i4->saddr = target.s_addr & mask.s_addr;
            md->smask = mask.s_addr;
            break;
        }
        case MATCHER_ETHER_DEST_LIST:
        case MATCHER_ETHER_SOURCE_LIST:{
            struct in_addr target;
            while (bson_iter_next(query)){
                module_ether_list_ip4 *s = NULL;
                s =  (module_ether_list_ip4*)bson_malloc0(sizeof(*s));
                BSON_ASSERT(BSON_ITER_HOLDS_UTF8(query));
                uint32_t len;
                const char * tstr = bson_iter_utf8(query, &len);
                result &= inet_aton(tstr, &target);
                s->addr = target.s_addr;
                HASH_ADD_INT(md->addrset, addr, s);
            }
            break;
        }
        case MATCHER_ETHER_DEST:{
            md->i4 = bson_malloc0(sizeof (struct iphdr));
            struct in_addr target, mask;
            result &= matcher_module_ether_get_query_data(md, query, &target, &mask);
            md->i4->daddr = target.s_addr & mask.s_addr;
            md->dmask = mask.s_addr;
            break;
        }
        case MATCHER_ETHER_SOURCEANDDEST:{
            md->i4 = bson_malloc0(sizeof (struct iphdr));
            struct in_addr starget, smask, dtarget, dmask;
            bson_iter_t source_iter, dest_iter;
            if (bson_iter_next(query) && BSON_ITER_HOLDS_ARRAY(query) &&
                    bson_iter_recurse(query, &source_iter) &&
                    matcher_module_ether_get_query_data(md, &source_iter, &starget, &smask) &&
                    bson_iter_next(query) &&
                    BSON_ITER_HOLDS_ARRAY(query) &&
                    bson_iter_recurse(query, &dest_iter) &&
                    matcher_module_ether_get_query_data(md, &dest_iter, &dtarget, &dmask)
                    ){
                md->i4->saddr = starget.s_addr & smask.s_addr;
                md->i4->daddr = dtarget.s_addr & dmask.s_addr;
                md->smask = smask.s_addr;

                md->dmask = dmask.s_addr;
            } else {
                result = false;
            }
            break;
        }
        default:
            break;
    }
    return result;
}
bool
matcher_module_ether_get_query_data(matcher_container_ether_t *md, bson_iter_t *query,
                                    struct in_addr *target, struct in_addr *mask){
    bool result = true;
    bool goodip, goodmask;
    uint32_t len;
    bson_iter_next(query);
    mask->s_addr = (uint32_t)MODULE_ETHER_IPV4_255_MASK ;
    BSON_ASSERT(BSON_ITER_HOLDS_UTF8(query));
    const char * tstr = bson_iter_utf8(query, &len);
    goodip = inet_aton(tstr, target);
    BSON_ASSERT(goodip);
    if (bson_iter_next(query) && BSON_ITER_HOLDS_UTF8(query)){
        const char * nmstr = bson_iter_utf8(query, &len);
        goodmask = inet_aton(nmstr, mask);
        BSON_ASSERT(goodmask);
    }
    return result;
}
matcher_ether_opcode_t
matcher_module_ether_get_opcode(const char *type, uint32_t len)
{
    matcher_ether_opcode_t result = MATCHER_ETHER_UNDEFINED;
    if (len >= 5 && len < 20){
        if (strcmp(type, "srcip") == 0 ){
            result = MATCHER_ETHER_SOURCE;
        } else if (strcmp(type, "dstip") == 0){
            result = MATCHER_ETHER_DEST;
        } else if (strcmp(type, "srciplist") == 0){
            result = MATCHER_ETHER_SOURCE_LIST;
        } else if (strcmp(type, "dstiplist") == 0){
            result = MATCHER_ETHER_DEST_LIST;
        } else if (strcmp(type, "dstip&dstport") == 0){
            result = MATCHER_ETHER_DESTPORT;
        } else if (strcmp(type, "srcip&dstip") == 0){
            result = MATCHER_ETHER_SOURCEANDDEST;
        } else if (strcmp(type, "srcip|dstip") == 0){
            result = MATCHER_ETHER_SOURCEORDEST;
        } else if (strcmp(type, "srcip&srcport") == 0){
            result = MATCHER_ETHER_SOURCEPORT;
        } else if (strcmp(type, "srcip&dstip&dstport") == 0){
            result = MATCHER_ETHER_SOURCEDESTPORT;
        } else {
            result = MATCHER_ETHER_UNKNOWN;
        }
    }
    return result;
}

bool
matcher_module_ether_destroy(mongoc_matcher_op_t *op)
{
    bool result = false;
    matcher_container_ether_t *md;
    md = (matcher_container_ether_t*) op->module.config.container.module_data;
    if (md->i4){bson_free(md->i4);}
    if (md->addrset){
        module_ether_list_ip4 *s = NULL, *tmp = NULL;
        HASH_ITER(hh, md->addrset, s, tmp) {
            HASH_DEL(md->addrset, s);
            free(s);
        }
    }


    bson_free(op->module.config.container.module_data);
    return result;
}

mongoc_matcher_module_callback_t
matcher_module_ether_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    if (iter){
        matcher_container_ether_t *md;
        md = (matcher_container_ether_t*) op->module.config.container.module_data;
        switch (md->opcode){
            case MATCHER_ETHER_SOURCE:
            case MATCHER_ETHER_DEST:
            case MATCHER_ETHER_SOURCEORDEST:
            case MATCHER_ETHER_SOURCEANDDEST:
                cb = matcher_module_ether_search_i4(op, iter, usermem, md);
                break;
            case MATCHER_ETHER_SOURCE_LIST:
            case MATCHER_ETHER_DEST_LIST:{
                cb = matcher_module_ether_search_i4_list(op, iter, usermem, md);
                break;
            }
            default:
                break;
        }

    }
    return cb;
}
mongoc_matcher_module_callback_t
matcher_module_ether_search_i4_list(mongoc_matcher_op_t * op, bson_iter_t * iter,
                                    void * usermem, matcher_container_ether_t * md){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    if (BSON_ITER_HOLDS_BINARY(iter)) {
        bson_subtype_t subtype;
        uint32_t binary_len = 0;
        const uint8_t *binary;
        bson_iter_binary(iter, &subtype, &binary_len, &binary);
        if (binary_len >= sizeof(struct iphdr) + sizeof(struct ethhdr)) { //SIZEOF hopefully compiler optimized
            struct iphdr *iph = (struct iphdr *) (binary + sizeof(struct ethhdr));
            module_ether_list_ip4 *s = NULL;
            switch (md->opcode) {
                case MATCHER_ETHER_SOURCE_LIST: {
                    HASH_FIND_INT(md->addrset, &iph->saddr, s);
                    if (s) {cb = MATCHER_MODULE_CALLBACK_FOUND;}
                    break;
                }
                case MATCHER_ETHER_DEST_LIST:{
                    HASH_FIND_INT(md->addrset, &iph->daddr, s);
                    //struct in_addr dst;
                    //dst.s_addr = iph->daddr;
                    //char * ds = bson_strdup(inet_ntoa(dst));
                    if (s) {cb = MATCHER_MODULE_CALLBACK_FOUND;}
                    break;
                }
                default:
                    break;
            }
        }
    }
    return cb;

}
mongoc_matcher_module_callback_t
matcher_module_ether_search_i4(mongoc_matcher_op_t * op, bson_iter_t * iter,
                               void * usermem, matcher_container_ether_t * md){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    if (BSON_ITER_HOLDS_BINARY(iter)){
        bson_subtype_t subtype;
        uint32_t binary_len=0;
        const uint8_t * binary;
        bson_iter_binary(iter, &subtype, &binary_len, &binary);
        if ( binary_len >= sizeof (struct iphdr) + sizeof (struct ethhdr) ){ //SIZEOF hopefully compiler optimized
            struct iphdr *iph = (struct iphdr*)(binary + sizeof(struct ethhdr));
            struct iphdr *cmp = bson_malloc0(sizeof (struct iphdr));
            switch (md->opcode){
                case MATCHER_ETHER_SOURCE:
                    cmp->saddr = iph->saddr & md->smask;
                    break;
                case MATCHER_ETHER_DEST:
                    cmp->daddr = iph->daddr & md->dmask;
                    break;
                case MATCHER_ETHER_SOURCEANDDEST:
                    cmp->saddr = iph->saddr & md->smask;
                    cmp->daddr = iph->daddr & md->dmask;
                    break;
                default:
                    break;
            }
            if (memcmp(md->i4, cmp, sizeof (struct iphdr)) == 0) {
                cb = MATCHER_MODULE_CALLBACK_FOUND;
            }
            bson_free(cmp);
        }
    }
    return cb;
}

#endif /* WITH_ETHER */
#endif /* WITH_MODULES */