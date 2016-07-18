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

#ifdef WITH_TEXT
#ifndef MONGOC_MATCHER_OP_TEXT_H
#define MONGOC_MATCHER_OP_TEXT_H
#include "mongoc-matcher-op-private.h"
BSON_BEGIN_DECLS
#define DEFAULT_TEXT_STOPCHARS " ,.-?;:()&@#%$^"
#define DEFAULT_TEXT_STEMMING_LANG "english"
#define DEFAULT_TEXT_DICTIONARY "en_US"
#define DEFAULT_TEXT_CASE_SENSITIVE false

mongoc_matcher_op_t *
_mongoc_matcher_text_new (const char              *path,   /* IN */
                            bson_iter_t             *child);   /* OUT */


mongoc_matcher_op_t *
_mongoc_matcher_parse_text_loop (const char              * path,
                                 bson_iter_t             *iter);

bool
_mongoc_matcher_op_text         (mongoc_matcher_op_t  *op, /* IN */
                                const bson_t         *bson); /* IN */
bool
_mongoc_matcher_op_text_match_iter (mongoc_matcher_op_text_t *compare, /* IN */
                                    bson_iter_t                 *iter);    /* IN */

bool
_mongoc_matcher_op_text_match  (mongoc_matcher_op_text_t     *compare,
                                const bson_t                 *bson);


BSON_END_DECLS
#endif /* MONGOC_MATCHER_OP_TEXT_H */
#endif /* WITH_TEXT */