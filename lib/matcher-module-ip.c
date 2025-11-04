/*
 * Copyright (c) 2017 Bauman
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef WITH_MODULES
#ifdef WITH_IP
#include <bson/bson.h>
#include "matcher-module-ip.h"


bool
matcher_module_ip_unload_ip(matcher_module_ip_t* md, bson_iter_t *ips)
{
    bool result = false, good_net=false, good_mask=false, type_matched=false;
    bson_subtype_t subtype_net, subtype_mask;
    uint32_t binary_len=0;
    const uint8_t * binary;
    bson_iter_binary(ips, &subtype_net, &binary_len, &binary);
    if (binary_len == MATCHER_MODULE_IP_BYTES){
        good_net = true;
        memcpy(md->base_addr, binary, binary_len);
    }
    if (bson_iter_next(ips) && BSON_ITER_HOLDS_BINARY(ips))
    {
        bson_iter_binary(ips, &subtype_mask, &binary_len, &binary);
        if (binary_len == MATCHER_MODULE_IP_BYTES){
            good_mask = true;
            memcpy(md->netmask, binary, binary_len);
            if (subtype_mask == subtype_net){md->subtype = subtype_net; type_matched = true;}
        }
    }
    result = good_mask && good_net && type_matched;
    if (result){
        int i = 0;
        MATCHER_MODULE_IP_CRITERIA(i, md->criteria, md->base_addr, md->netmask);
    }
    return result;
}

bool
matcher_module_ip_unload_ips(matcher_module_ip_t* md, bson_iter_t *ips){
    bool result = true;
    bson_iter_t each_ip;
    matcher_module_ip_t *head = md, *current=md, *parent=NULL;
    do {
        if (bson_iter_recurse(ips, &each_ip) && bson_iter_next(&each_ip) && BSON_ITER_HOLDS_BINARY(&each_ip)){
            if (!current){
                current = (matcher_module_ip_t*) bson_malloc0(sizeof *current);
                parent->next = current;
            }
            result &= matcher_module_ip_unload_ip(current, &each_ip);
            head->depth++;
            parent=current; current=NULL;
        }
    } while (bson_iter_next(ips));
#ifndef WITH_ACCELERATION
    //flatten the object and push it to device memory.

#endif
    return result;
}

bool
matcher_module_ip_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool result = false;
    bool high, low;
    bson_iter_t ips;
    matcher_module_ip_t *md;
    md = (matcher_module_ip_t*) bson_malloc0(sizeof *md);
    while (bson_iter_next(config)) {
        const char * key = bson_iter_key (config);
        if (BSON_ITER_HOLDS_ARRAY(config) && strcmp(key, "ips") == 0 &&
                bson_iter_recurse(config, &ips) && bson_iter_next(&ips) ){
            switch (bson_iter_type(&ips)) {
                case BSON_TYPE_BINARY: //SINGLE IP Range
                    result |= matcher_module_ip_unload_ip(md, &ips);
                    break;
                case BSON_TYPE_ARRAY: { //MULTIPLE IP Ranges
                    result |= matcher_module_ip_unload_ips(md, &ips);
                    break;
                }
                default: //may include Uint32 for compatibility with other tools
                    break;
            }
        }
    }
    if (!result){
        bson_free(md);
    } else {
        op->module.config.container.module_data = (void*)md;
    }
    return result;
}

mongoc_matcher_module_callback_t
matcher_module_ip_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    if (iter){
        matcher_module_ip_t *md;
        md = (matcher_module_ip_t *) op->module.config.container.module_data;
        if (matcher_module_ip_match_iter(md, iter)){
            return MATCHER_MODULE_CALLBACK_FOUND;
        }
    }
    return MATCHER_MODULE_CALLBACK_CONTINUE;
}






bool
matcher_module_ip_match_iter (matcher_module_ip_t *ip, /* IN */
                               bson_iter_t        *iter)    /* IN */
{
    bool result = false;
    switch (bson_iter_type (iter)) {
        case BSON_TYPE_BINARY: {
            bson_subtype_t subtype;
            uint32_t binary_len=0;
            const uint8_t * binary;
            bson_iter_binary(iter, &subtype, &binary_len, &binary);
            if (binary_len == MATCHER_MODULE_IP_BYTES && subtype == ip->subtype)
            {
                if (!ip->next){
                    uint8_t masked_ip[MATCHER_MODULE_IP_BYTES];
                    int i = 0;
                    MATCHER_MODULE_IP_CRITERIA(i, masked_ip, binary, ip->netmask);
                    if (memcmp(masked_ip, ip->criteria, MATCHER_MODULE_IP_BYTES) == 0){
                        result = true;
                    }
                } else {
#ifdef WITH_ACCELERATION

#else
                    matcher_module_ip_t *current=ip;
                    while (current){
                        uint8_t masked_ip[MATCHER_MODULE_IP_BYTES];
                        int i = 0;
                        MATCHER_MODULE_IP_CRITERIA(i, masked_ip, binary, current->netmask);
                        if (memcmp(masked_ip, current->criteria, MATCHER_MODULE_IP_BYTES) == 0){
                            result |= true;
                            break;
                        }
                        current = current->next;
                    }
#endif
                }
            }
            break;
        }
        case BSON_TYPE_ARRAY:{
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array)) {
                while (bson_iter_next(&right_array)){
                    result |=  matcher_module_ip_match_iter(ip, &right_array);
                    if (result){
                        break;
                    }
                }
            }
            break;
        }
        default:
            result = false;
            break;
    }
    return result;
}





bool
matcher_module_ip_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_module_ip_t *md, *current, *last;
    md = (matcher_module_ip_t *) op->module.config.container.module_data;
    current = md;
    last = NULL;
    while (current){
        bson_free(last);
        last = current;
        current = current->next;
    };
    bson_free(last);
    return result;
}

#endif /* WITH_IP */
#endif /* WITH_MODULES */