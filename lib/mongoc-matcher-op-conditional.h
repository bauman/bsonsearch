/*
 * Copyright (c) 2016 Bauman
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

#ifdef WITH_CONDITIONAL
#ifndef MONGOC_MATCHER_OP_CONDITIONAL_H
#define MONGOC_MATCHER_OP_CONDITIONAL_H

#include "mongoc-matcher-op-private.h"
BSON_BEGIN_DECLS
#define CONDITIONAL_HAS_NONE 0
#define CONDITIONAL_HAS_CONDITION 1
#define CONDITIONAL_HAS_TRUE_PATH 2
#define CONDITIONAL_HAS_FALSE_PATH 4
#define CONDITIONAL_HAS_ALL 7

mongoc_matcher_op_t *
_mongoc_matcher_parse_conditional (mongoc_matcher_opcode_t  opcode,  /* IN */
                                   bson_iter_t             *iter,    /* IN */
                                   bool                     is_root, /* IN */
                                   bson_error_t            *error);   /* OUT */

mongoc_matcher_op_t *
_mongoc_matcher_parse_conditional_loop (bson_iter_t             *iter,    /* IN */
                                        bson_error_t            *error);
bool
_mongoc_matcher_op_conditional (mongoc_matcher_op_t  *op, /* IN */
                                const bson_t         *bson); /* IN */


BSON_END_DECLS

#endif //MONGOC_MATCHER_OP_CONDITIONAL_H
#endif /*WITH_CONDITIONAL*/

