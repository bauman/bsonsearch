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


#ifdef WITH_IP
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-ip.h"
#include "mongoc-bson-descendants.h"

mongoc_matcher_op_t *
_mongoc_matcher_op_ip_new    (const char              *path,
                              const bson_iter_t       *iter)
{
    mongoc_matcher_op_t *op = NULL;
    op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *op);
    op->base.opcode = MONGOC_MATCHER_OPCODE_INIPRANGE;
    op->ip.path = bson_strdup(path);
    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_ARRAY: {
            uint8_t result = 0;
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array)) {
                while (bson_iter_next(&right_array) && result < MONGOC_MATCHER_OP_IP_HAVE_HIGH) {
                    BSON_ASSERT(BSON_ITER_HOLDS_BINARY(&right_array));
                    bson_subtype_t subtype;
                    uint32_t binary_len=0;
                    const uint8_t * binary;
                    bson_iter_binary(&right_array, &subtype, &binary_len, &binary);
                    if (binary_len != MONGOC_MATCHER_OP_IP_BYTES){
                        break;
                    }
                    if (result < MONGOC_MATCHER_OP_IP_HAVE_LOW){
                        memcpy(op->ip.base_addr, binary, binary_len);
                        op->ip.subtype = subtype;
                        result |= MONGOC_MATCHER_OP_IP_HAVE_LOW;
                    } else {
                        BSON_ASSERT(op->ip.subtype == subtype);
                        memcpy(op->ip.netmask, binary, binary_len);
                        result |= MONGOC_MATCHER_OP_IP_HAVE_HIGH;
                    }
                }
            }
            if (result > MONGOC_MATCHER_OP_IP_HAVE_HIGH){
                int i = 0;
                MONGOC_MATCHER_OP_IP_CRITERIA(i, op->ip.criteria, op->ip.base_addr, op->ip.netmask);
            }
            break;
        }
        default:
            break;
    }
    return op;
}

bool
_mongoc_matcher_op_iniprange_match(mongoc_matcher_op_t *op,
                                   const bson_t         *bson)
{
    bson_iter_t tmp;
    bson_iter_t iter;
    bool found_one = false;
    int checked = 0, skip=0;
    BSON_ASSERT (op);
    BSON_ASSERT (bson);
    if (strchr (op->ip.path, '.')) {
        if (!bson_iter_init (&tmp, bson) ||
            !bson_iter_find_descendant (&tmp, op->ip.path, &iter)) { //try this way first
            while (!found_one &&
                   bson_iter_init (&tmp, bson) &&
                   bson_iter_find_descendants (&tmp, op->ip.path, &skip, &iter)){
                found_one |= _mongoc_matcher_op_iniprange_match_iter(&op->ip, &iter);
                skip = ++checked;
            }
            return ((checked>0) && found_one);
        }
    } else if (!bson_iter_init_find (&iter, bson, op->ip.path)) {
        return false;
    }
    return _mongoc_matcher_op_iniprange_match_iter(&op->ip, &iter);
}
bool
_mongoc_matcher_op_iniprange_match_iter (mongoc_matcher_op_ip_t *ip, /* IN */
                                         bson_iter_t               *iter)    /* IN */
{
    bool result = false;
    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_BINARY: {
            bson_subtype_t subtype;
            uint32_t binary_len=0;
            const uint8_t * binary;
            bson_iter_binary(iter, &subtype, &binary_len, &binary);
            if (binary_len == 16 && subtype == ip->subtype)
            {
                uint8_t masked_ip[MONGOC_MATCHER_OP_IP_BYTES];
                int i = 0;
                MONGOC_MATCHER_OP_IP_CRITERIA(i, masked_ip, binary, ip->netmask);
                if (memcmp(masked_ip, ip->criteria, sizeof(ip->criteria)) == 0){
                    result = true;
                }
            }
            break;
        }
        case BSON_TYPE_ARRAY:{
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array)) {
                while (bson_iter_next(&right_array)){
                    result |=  _mongoc_matcher_op_iniprange_match_iter(ip, &right_array);
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

#endif /*WITH_IP*/
