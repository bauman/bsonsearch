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
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-projection.h"
#include "mongoc-bson-descendants.h"


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_parse_projection --
 *
 *       Parse an aggregation spec containing a projection operator
 *       $project.
 *
 *       See the following link for more information.
 *
 *       https://docs.mongodb.org/manual/tutorial/project-fields-from-query-results/
 *
 *       Differences, this does not support the 0 value to omit a single field.
 *       If the key is present, it will do it's best to include it in the output
 *
 *
 * Requires:
 *       iter MUST Be a document type, otherwise, outcome undefined.
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
_mongoc_matcher_parse_projection (mongoc_matcher_opcode_t  opcode,  /* IN */
                                  bson_iter_t             *iter,    /* IN */
                                  bool                     is_root, /* IN */
                                  bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *op = NULL;
    mongoc_matcher_op_t *left_op = NULL;
    BSON_ASSERT (opcode == MONGOC_MATCHER_OPCODE_PROJECTION);
    BSON_ASSERT (iter);

    bson_iter_t child;
    if (bson_iter_recurse(iter, &child)) {
        op = _mongoc_matcher_parse_projection_loop(&child, error);
    }
    return op;
}
mongoc_matcher_op_t *
_mongoc_matcher_parse_projection_loop (bson_iter_t             *iter,    /* IN */
                                       bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *on_the_left=NULL;
    mongoc_matcher_op_t *next_left = NULL;
    if (bson_iter_next(iter)){
        const bson_value_t * value = bson_iter_value(iter);
        switch (value->value_type){
            case BSON_TYPE_BINARY:
            case BSON_TYPE_INT32:
            {
                const char * key = bson_iter_key (iter);
                next_left = _mongoc_matcher_parse_projection_loop(iter, error);
                on_the_left = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *on_the_left);
                on_the_left->base.opcode = MONGOC_MATCHER_OPCODE_PROJECTION;
                on_the_left->projection.path = bson_strdup(key);
                on_the_left->projection.next = next_left;
                break;
            }
            default:
                break;
        }
    }
    return on_the_left;

}

bool
mongoc_matcher_projection_execute(mongoc_matcher_op_t *op,     //in
                                  bson_t              *bson,        //in
                                  bson_t              *projected)   //out
{
    assert(op->base.opcode == MONGOC_MATCHER_OPCODE_PROJECTION);



    bson_iter_t iter, tmp;
    int checked = 0, skip=0;
    bool result = false;
    bson_t parent, arrlist;
    char *str;

    bson_init (&parent);
    bson_append_array_begin (&parent, op->projection.path, -1, &arrlist);
    //bson_append_int32 (&child, "0", 1, 1);
    //bson_append_int32 (&child, "1", 1, 3);

    uint32_t packed = 0;
    if (strchr (op->projection.path, '.')) {
        if (bson_iter_init (&tmp, bson) )
        {
            if (bson_iter_find_descendant (&tmp, op->projection.path, &iter))
            {
                mongoc_matcher_projection_value_into_array(iter, arrlist, checked);
            } else {
                while (bson_iter_init (&tmp, bson) &&
                       bson_iter_find_descendants (&tmp, op->projection.path, &skip, &iter)){
                    packed += mongoc_matcher_projection_value_into_array(iter, arrlist, packed);
                    skip = ++checked;
                }
            }
        }
    } else if (bson_iter_init_find (&iter, bson, op->projection.path)) {
        mongoc_matcher_projection_value_into_array(iter, arrlist, checked);
    }
    bson_append_array_end (&parent, &arrlist);
    bson_error_t *err;
    str = bson_as_json (&parent, NULL);
    //printf ("%s\n", str);
    bson_free (str);
    return result;
}

uint32_t
mongoc_matcher_projection_value_into_array(bson_iter_t  iter, bson_t arrlist, uint32_t i)
{
    char STR_BUFFER[16];
    const char *key;
    size_t st = bson_uint32_to_string (i, &key, STR_BUFFER, sizeof STR_BUFFER);
    switch (bson_iter_type ((&iter))) {
        case BSON_TYPE_DOCUMENT:
            i=0;
            break;
        case BSON_TYPE_ARRAY:
        {
            bson_iter_t right_array;
            bson_iter_recurse(&iter, &right_array);
            while (bson_iter_next(&right_array)) {
                i += mongoc_matcher_projection_value_into_array( right_array, arrlist, i);
            }
            break;
        }
        case BSON_TYPE_UTF8:
        {
            uint32_t vlen=-1;
            const char * value = bson_iter_utf8(&iter, &vlen);
            bson_append_utf8(&arrlist, key, st, bson_strdup(value), vlen);
            break;
        }
        case BSON_TYPE_BOOL:
        case BSON_TYPE_BINARY:
        case BSON_TYPE_DATE_TIME:
        case BSON_TYPE_INT32:
        case BSON_TYPE_INT64:
        case BSON_TYPE_REGEX:
        case BSON_TYPE_OID:
        case BSON_TYPE_DOUBLE:
        case BSON_TYPE_TIMESTAMP:
        default:
            i=0;
            break;
    }
    return i;
}


#endif //WITH_PROJECTION