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
_mongoc_matcher_op_ip_new    (mongoc_matcher_opcode_t opcode,
                              const char              *path,
                              const bson_iter_t       *iter)
{
    mongoc_matcher_op_t *op = NULL;
    op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *op);
    op->base.opcode = opcode;
    op->ip.path = bson_strdup(path);
    bool success = false;
    switch (opcode){
        case MONGOC_MATCHER_OPCODE_INIPRANGE:{
            success = _mongoc_matcher_op_iniprange_build_ip(op, iter);
            break;
        }
        case MONGOC_MATCHER_OPCODE_INIPRANGESET:{
            success = _mongoc_matcher_op_iniprangeset_build_ip(op, iter);
            break;
        }
        default:
            break;
    }
    if (!success){
        _mongoc_matcher_op_destroy(op);
        op=NULL;
    }
    return op;
}
bool
_mongoc_matcher_op_iniprangeset_build_ip(mongoc_matcher_op_t *op,
                                         const bson_iter_t   *iter) {
    bool result = true;
    mongoc_matcher_op_t *current_next = op;
    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_ARRAY: {
            bson_iter_t right_outer_array;
            if (bson_iter_recurse(iter, &right_outer_array)) {
                while (bson_iter_next(&right_outer_array)){
                    if (BSON_ITER_HOLDS_ARRAY(&right_outer_array)){
                        mongoc_matcher_op_t *next = (mongoc_matcher_op_t*) bson_malloc0(sizeof *next);
                        if (_mongoc_matcher_op_iniprange_build_ip(next, &right_outer_array)){
                            next->base.opcode = current_next->base.opcode;
                            current_next->ip.next = next;
                            current_next = next;
                        } else {
                            _mongoc_matcher_op_destroy(next);
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return result;
}

bool
_mongoc_matcher_op_iniprange_build_ip(mongoc_matcher_op_t *op,
                                      const bson_iter_t   *iter)
{
    bool result = false;
    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_ARRAY: {
            uint8_t have_all = 0;
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array)) {
                while (bson_iter_next(&right_array) && result < MONGOC_MATCHER_OP_IP_HAVE_HIGH) {
                    BSON_ASSERT(BSON_ITER_HOLDS_BINARY(&right_array));
                    bson_subtype_t subtype;
                    uint32_t binary_len=0;
                    const uint8_t * binary;
                    bson_iter_binary(&right_array, &subtype, &binary_len, &binary);
                    op->ip.length = binary_len;
                    if (binary_len != MONGOC_MATCHER_OP_IP_BYTES){
                        break;
                    }
                    if (have_all < MONGOC_MATCHER_OP_IP_HAVE_LOW){
                        memcpy(op->ip.base_addr, binary, binary_len);
                        op->ip.subtype = subtype;
                        have_all |= MONGOC_MATCHER_OP_IP_HAVE_LOW;
                    } else {
                        BSON_ASSERT(op->ip.subtype == subtype);
                        memcpy(op->ip.netmask, binary, binary_len);
                        have_all |= MONGOC_MATCHER_OP_IP_HAVE_HIGH;
                    }
                }
            }
            if (have_all > MONGOC_MATCHER_OP_IP_HAVE_HIGH){
                int i = 0;
                MONGOC_MATCHER_OP_IP_CRITERIA(i, op->ip.criteria, op->ip.base_addr, op->ip.netmask);
                result = true;
            }
            break;
        }
        default:
            break;
    }
    return result;
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
                switch (op->base.opcode){
                    case MONGOC_MATCHER_OPCODE_INIPRANGE:
                    {
                        found_one |= _mongoc_matcher_op_iniprange_match_iter(&op->ip, &iter);
                        break;
                    }
                    case MONGOC_MATCHER_OPCODE_INIPRANGESET:
                    {
                        break;
                    }
                    default:
                        break;
                }

                skip = ++checked;
            }
            return ((checked>0) && found_one);
        }
    } else if (!bson_iter_init_find (&iter, bson, op->ip.path)) {
        return false;
    }
    switch (op->base.opcode){
        case MONGOC_MATCHER_OPCODE_INIPRANGE:
        {
            return _mongoc_matcher_op_iniprange_match_iter(&op->ip, &iter);
        }
        case MONGOC_MATCHER_OPCODE_INIPRANGESET:
        {
            return  _mongoc_matcher_op_iniprangeset_match_iter(&op->ip, &iter);
        }
        default:
            return false;
    }
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
                if (memcmp(masked_ip, ip->criteria, MONGOC_MATCHER_OP_IP_BYTES) == 0){
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
bool
_mongoc_matcher_op_iniprangeset_match_iter (mongoc_matcher_op_ip_t *ip, /* IN */
                                            bson_iter_t               *iter)    /* IN */
{
    bool result = false;
    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_BINARY: {
            mongoc_matcher_op_t *next_op = ip->next;
            bson_subtype_t subtype;
            uint32_t binary_len=0;
            const uint8_t * binary;
            bson_iter_binary(iter, &subtype, &binary_len, &binary);
            if (binary_len == 16 && subtype == next_op->ip.subtype)
            {
                while (next_op){
                    mongoc_matcher_op_ip_t *next_ip = &next_op->ip;
                    uint8_t masked_ip[MONGOC_MATCHER_OP_IP_BYTES];
                    int i = 0;
                    MONGOC_MATCHER_OP_IP_CRITERIA(i, masked_ip, binary, next_ip->netmask);
                    if (memcmp(masked_ip, next_ip->criteria, MONGOC_MATCHER_OP_IP_BYTES) == 0){
                        result = true;
                        break;
                    }
                    next_op = next_op->ip.next;
                }
            }
            break;
        }
        case BSON_TYPE_ARRAY:{
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array)) {
                while (bson_iter_next(&right_array)){
                    result |=  _mongoc_matcher_op_iniprangeset_match_iter(ip, &right_array);
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
