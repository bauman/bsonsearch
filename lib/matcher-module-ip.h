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

#ifndef MATCHER_MODULE_IP_H
#define MATCHER_MODULE_IP_H

#include "mongoc-matcher-op-private.h"
BSON_BEGIN_DECLS
#define BSON_SUBTYPE_IPV4 0x84
#define BSON_SUBTYPE_IPV6 0x86
#define MATCHER_MODULE_IP_HAVE_NET 1
#define MATCHER_MODULE_IP_HAVE_MASK 2
#define MATCHER_MODULE_IP_BYTES 16
#define MODULE_IP_COMMAND "ip"
BSON_END_DECLS

#define MATCHER_MODULE_IP_CRITERIA(i, o, i1, i2){for(i = 0; i < MATCHER_MODULE_IP_BYTES; ++i){o[i] = i1[i] & i2[i];}}

typedef struct _matcher_module_ip_criteria_hashtable_t _matcher_module_ip_criteria_hashtable_t;
struct _matcher_module_ip_criteria_hashtable_t {
    uint8_t criteria[MATCHER_MODULE_IP_BYTES]; //MONGOC_MATCHER_OP_IP_BYTES
    UT_hash_handle hh;
};
typedef struct _matcher_module_ip_t matcher_module_ip_t;
struct _matcher_module_ip_t
{
    uint32_t depth;
    bson_subtype_t subtype;
    uint8_t base_addr[MATCHER_MODULE_IP_BYTES];
    uint8_t netmask[MATCHER_MODULE_IP_BYTES];
    uint8_t criteria[MATCHER_MODULE_IP_BYTES];
    matcher_module_ip_t *next;
};

bool
matcher_module_ip_unload_ip(matcher_module_ip_t* md, bson_iter_t *ips);
bool
matcher_module_ip_unload_ips(matcher_module_ip_t* md, bson_iter_t *ips);
bool
matcher_module_ip_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_ip_destroy(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_ip_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);



bool
matcher_module_ip_match_iter (matcher_module_ip_t *ip, /* IN */
                              bson_iter_t        *iter);    /* IN */






#endif //MATCHER_MODULE_IP_H
#endif /* WITH_IP */
#endif /* WITH_MODULES */