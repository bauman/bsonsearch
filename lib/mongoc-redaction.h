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

#ifdef WITH_PROJECTION
#ifndef MONGOC_MATCHER_REDACTION
#define MONGOC_MATCHER_REDACTION
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-projection.h"


mongoc_matcher_op_t *
_mongoc_matcher_parse_redaction (mongoc_matcher_opcode_t  opcode,  /* IN */
                                 bson_iter_t             *iter,    /* IN */
                                 bool                     is_root, /* IN */
                                 bson_error_t            *error);   /* OUT */
bool
mongoc_matcher_redaction_execute(mongoc_matcher_op_t *op,        /*in */
                                    bson_t              *bson,      /*in */
                                    bson_t              *projected); /*in/out */


#endif //MONGOC_MATCHER_REDACTION
#endif //WITH_PROJECTION