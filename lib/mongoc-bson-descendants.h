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

#ifndef MONGOC_BSON_DESCENDANTS_H
#define MONGOC_BSON_DESCENDANTS_H

#include <bson/bson.h>


BSON_BEGIN_DECLS


bool
bson_iter_find_descendants (bson_iter_t *iter,       /* INOUT */
                            const char  *dotkey,     /* IN */
                            int         *skip,
                            bson_iter_t *descendant) /* OUT */;
bool
_mongoc_bson_iter_move_to_key(bson_iter_t *iter,   /* INOUT */
                              const char  *key,    /* IN */
                              int          keylen);  /* IN */
bool
_mongoc_bson_iter_find_with_len (bson_iter_t *iter,   /* INOUT */
                          const char  *key,    /* IN */
                          int          keylen) /* IN */;

BSON_END_DECLS


#endif /* MONGOC_BSON_DESCENDANTS_H */