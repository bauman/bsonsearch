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
#ifndef MONGOC_MATCHER_OP_IP_H
#define MONGOC_MATCHER_OP_IP_H

#include "mongoc-matcher-op-private.h"
BSON_BEGIN_DECLS
#define BSON_SUBTYPE_IPV4 0x84
#define BSON_SUBTYPE_IPV6 0x86
#define MONGOC_MATCHER_OP_IP_HAVE_LOW 1
#define MONGOC_MATCHER_OP_IP_HAVE_HIGH 2
#define MONGOC_MATCHER_OP_IP_BYTES 16
BSON_END_DECLS

#define MONGOC_MATCHER_OP_IP_CRITERIA(i, o, i1, i2){for(i = 0; i < MONGOC_MATCHER_OP_IP_BYTES; ++i){o[i] = i1[i] & i2[i];}}

mongoc_matcher_op_t *
_mongoc_matcher_op_ip_new    (mongoc_matcher_opcode_t opcode,
                              const char              *path,
                              const bson_iter_t       *iter);
bool
_mongoc_matcher_op_iniprange_build_ip(mongoc_matcher_op_t *op,
                                      const bson_iter_t   *iter);
bool
_mongoc_matcher_op_iniprange_match(mongoc_matcher_op_t *op,
                                   const bson_t         *bson);

bool
_mongoc_matcher_op_iniprange_match_iter (mongoc_matcher_op_ip_t *ip, /* IN */
                                         bson_iter_t               *iter);    /* IN */

bool
_mongoc_matcher_op_iniprangeset_match_iter (mongoc_matcher_op_ip_t *ip, /* IN */
                                            bson_iter_t               *iter);    /* IN */

bool
_mongoc_matcher_op_iniprangeset_build_ip(mongoc_matcher_op_t *op,
                                         const bson_iter_t   *iter);

#endif //MONGOC_MATCHER_OP_IP_H
#endif /*WITH_IP*/


