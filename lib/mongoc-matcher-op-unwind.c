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
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-projection.h"
#include "mongoc-matcher-op-unwind.h"
#include "bsoncompare.h"

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_parse_unwind --
 *
 *       Parse an aggregation spec containing an operator
 *       $unwind.
 *
 *
 *       { "$unwind" : {<key>:<value> }}
 *             ^
 *       ------^
 *
 *
 * Returns:
 *       A newly allocated mongoc_matcher_op_t if successful; otherwise
 *       NULL and @error may be set.
 *
 * Side effects:
 *       @error may be set.
 *
 *--------------------------------------------------------------------------
 */
mongoc_matcher_op_t *
_mongoc_matcher_parse_unwind (mongoc_matcher_opcode_t  opcode,  /* IN */
                                  bson_iter_t             *iter,    /* IN */
                                  bool                     is_root, /* IN */
                                  bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *op = NULL;
    BSON_ASSERT (opcode == MONGOC_MATCHER_OPCODE_UNWIND);
    BSON_ASSERT (iter);

    bson_iter_t child;
    if (bson_iter_recurse(iter, &child)) {
        op = _mongoc_matcher_parse_unwind_loop(&child, error);
    }
    return op;
}

mongoc_matcher_op_t *
_mongoc_matcher_parse_unwind_loop (bson_iter_t             *iter,    /* IN */
                                   bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *projector = NULL;
    mongoc_matcher_op_t *queryer   = NULL;
    uint8_t unwind_ok = UNWIND_HAS_NONE;
    while (bson_iter_next(iter)){
        const bson_value_t * value = bson_iter_value(iter);
        switch (value->value_type){
            case BSON_TYPE_DOCUMENT:
            {
                const char * key = bson_iter_key (iter);
                if (strcmp (key, "$project") == 0) {
                    projector = _mongoc_matcher_parse_projection(MONGOC_MATCHER_OPCODE_UNWIND,
                                                            iter, false, error);
                    unwind_ok |= UNWIND_HAS_PROJECTION;
                } else if (strcmp (key, "$query") == 0 && BSON_ITER_HOLDS_DOCUMENT(iter)) {
                    bson_iter_t child;
                    if (bson_iter_recurse(iter, &child)&& bson_iter_next(&child)) {
                        queryer = _mongoc_matcher_parse (&child, error);
                    }
                    unwind_ok |= UNWIND_HAS_QUERY ;
                }
                break;
            }
            default:
                break;
        }
    }
    if (unwind_ok != UNWIND_HAS_ALL){
        _mongoc_matcher_op_destroy(projector);
        _mongoc_matcher_op_destroy(queryer);
        projector = NULL; //NULL RETURN VALUE
    } else {
        projector->projection.query = queryer;
    }
    return projector;
}

bool
_mongoc_matcher_op_unwind (mongoc_matcher_op_t *op, /* IN */
                            const bson_t                       *bson) /* IN */
{
    bool matched = false;
    char * key = NULL;
    size_t key_size = 0;
    if (op->projection.as){ //path and as may both be populated
        key = op->projection.as;
        key_size = strlen(op->projection.as);
    } else if (op->projection.path){
        key = op->projection.path;
        key_size = strlen(op->projection.path);
    }
    if (key){
        bson_t * projected = bson_new();
        mongoc_matcher_projection_execute(op, (bson_t *)bson, projected);
        bson_iter_t iter,  array_iter;
        if (bson_iter_init_find (&iter, projected, key) &&
            BSON_ITER_HOLDS_ARRAY(&iter) &&
            bson_iter_recurse(&iter, &array_iter)){

            while (bson_iter_next(&array_iter))
            {
                bson_t * item = bson_new();
                mongoc_matcher_projection_value_into_document(&array_iter, item, key, key_size);
                matched = _mongoc_matcher_op_match(op->projection.query, (const bson_t *)item);
                bson_destroy(item);
                if (matched){
                    break;
                }
            }
        }
        bson_destroy(projected);
    }
    return matched;
}

#endif /* WITH_PROJECTION */