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


#ifdef WITH_CRYPT
#ifndef MONGOC_MATCHER_OP_CRYPT_H
#define MONGOC_MATCHER_OP_CRYPT_H

#include "mongoc-matcher-op-private.h"
BSON_BEGIN_DECLS
#define CRYPT_HAS_NONE 0
#define CRYPT_HAS_PK 1
#define CRYPT_HAS_SK 2
#define CRYPT_HAS_QUERY 4
#define CRYPT_HAS_ALL 7



BSON_END_DECLS


mongoc_matcher_op_t *
_mongoc_matcher_op_crypt_new    (mongoc_matcher_opcode_t opcode,
                                 const char              *path,
                                 const bson_iter_t       *iter);

bool
_mongoc_matcher_parse_crypt_loop (mongoc_matcher_op_t *op,
                                  bson_iter_t         *iter);

uint8_t
_mongoc_matcher_parse_crypt_keys (mongoc_matcher_op_t *op,
                                  bson_iter_t         *iter);

bool
_mongoc_matcher_op_sealopen_match (mongoc_matcher_op_t  *op, /* IN */
                                   const bson_t         *bson); /* IN */

bool
_mongoc_matcher_op_sealopen_match_iter (mongoc_matcher_op_crypt_t *crypt, /* IN */
                                        bson_iter_t               *iter);    /* IN */

#endif //MONGOC_MATCHER_OP_CRYPT_H
#endif /*WITH_CRYPT*/


