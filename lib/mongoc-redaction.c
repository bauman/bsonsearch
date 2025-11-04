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
#ifdef WITH_PROJECTION

#include <stddef.h>
#include <uthash.h>
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-projection.h"
#include "mongoc-bson-descendants.h"
#include "mongoc-redaction.h"


mongoc_matcher_op_t *
_mongoc_matcher_parse_redaction (mongoc_matcher_opcode_t  opcode,  /* IN */
                                  bson_iter_t             *iter,    /* IN */
                                  bool                     is_root, /* IN */
                                  bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *op = NULL, *tmp=NULL;
    mongoc_matcher_op_str_hashtable_t *s = NULL;
    mongoc_matcher_op_str_hashtable_t *found_in_set = NULL;
    BSON_ASSERT (opcode == MONGOC_MATCHER_OPCODE_REDACTION );
    BSON_ASSERT (iter);

    bson_iter_t child;
    if (bson_iter_recurse(iter, &child)) {
        op = _mongoc_matcher_parse_projection_loop(opcode, &child, error);
    }
    tmp = op;
    found_in_set = op->projection.pathlist;
    while (tmp != NULL) {
        char *matcher_hash_key_persist = bson_strdup(tmp->projection.path);
        s = ( mongoc_matcher_op_str_hashtable_t *)malloc(sizeof( mongoc_matcher_op_str_hashtable_t ));
        s->matcher_hash_key = matcher_hash_key_persist;
        HASH_ADD_KEYPTR(hh, found_in_set, s->matcher_hash_key, strlen(s->matcher_hash_key), s);
        tmp = tmp->projection.next;
    } ;
    op->projection.pathlist = found_in_set;
    return op;
}

bool
mongoc_matcher_redaction_execute(mongoc_matcher_op_t *op,        /*in */
                                  bson_t              *bson,      /*in */
                                  bson_t              *projected) /*in/out */
{
    bool result = false;
    BSON_ASSERT (op->base.opcode == MONGOC_MATCHER_OPCODE_REDACTION );
    bson_iter_t iter;
    bson_t arrlist;
    uint32_t packed = 0;

    if (bson_iter_init(&iter, bson)){
        while (bson_iter_next(&iter)) {
            packed = 0;
            const char * key = bson_iter_key(&iter);
            mongoc_matcher_op_str_hashtable_t *check=NULL;
            HASH_FIND_STR(op->projection.pathlist, key, check);
            if (check == NULL)
            {
                bson_append_array_begin (projected, key, -1, &arrlist);
                packed += mongoc_matcher_projection_value_into_array(&iter, &arrlist, packed);
                bson_append_array_end (projected, &arrlist);
            }
        }
    }
    return result;
}

#endif //WITH_PROJECTION