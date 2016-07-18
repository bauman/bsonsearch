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

#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-conditional.h"


mongoc_matcher_op_t *
_mongoc_matcher_parse_conditional (mongoc_matcher_opcode_t  opcode,  /* IN */
                              bson_iter_t             *iter,    /* IN */
                              bool                     is_root, /* IN */
                              bson_error_t            *error)   /* OUT */
{
    mongoc_matcher_op_t *op = NULL;
    BSON_ASSERT (opcode == MONGOC_MATCHER_OPCODE_CONDITIONAL);
    BSON_ASSERT (iter);

    bson_iter_t child;
    if (bson_iter_recurse(iter, &child)) {
        op = _mongoc_matcher_parse_conditional_loop(&child, error);
    }
    return op;
}
mongoc_matcher_op_t *
_mongoc_matcher_parse_conditional_loop (bson_iter_t             *iter,    /* IN */
                                        bson_error_t            *error)
{
    mongoc_matcher_op_t *condition = NULL;
    mongoc_matcher_op_t *iftrue    = NULL;
    mongoc_matcher_op_t *iffalse   = NULL;
    mongoc_matcher_op_t *op        = NULL;
    uint8_t conditional_ok = CONDITIONAL_HAS_NONE;
    while (bson_iter_next(iter)) {
        const char * key = bson_iter_key(iter);
        bson_iter_t child;
        if (strcmp(key, "if")==0){
            if (bson_iter_recurse(iter, &child)&& bson_iter_next(&child)) {
                condition = _mongoc_matcher_parse (&child, error);
            }
            if (condition != NULL){
                conditional_ok |= CONDITIONAL_HAS_CONDITION;
            }
        } else if (strcmp(key, "then")==0){
            if (bson_iter_recurse(iter, &child)&& bson_iter_next(&child)) {
                iftrue = _mongoc_matcher_parse (&child, error);
            }
            if (iftrue != NULL){
                conditional_ok |= CONDITIONAL_HAS_TRUE_PATH;
            }
        } else if (strcmp(key, "else")==0){

            if (bson_iter_recurse(iter, &child)&& bson_iter_next(&child)) {
                iffalse = _mongoc_matcher_parse (&child, error);
            }
            if (iffalse != NULL){
                conditional_ok |= CONDITIONAL_HAS_FALSE_PATH;
            }
        }
    }
    if ( conditional_ok == CONDITIONAL_HAS_ALL )  {
        op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *op);
        op->base.opcode = MONGOC_MATCHER_OPCODE_CONDITIONAL;
        op->conditional.condition = condition;
        op->conditional.iftrue = iftrue;
        op->conditional.iffalse = iffalse;
    } else {
        _mongoc_matcher_op_destroy(condition);
        _mongoc_matcher_op_destroy(iftrue);
        _mongoc_matcher_op_destroy(iffalse);
    }
    return op;
}
bool
_mongoc_matcher_op_conditional (mongoc_matcher_op_t  *op, /* IN */
                                const bson_t         *bson) /* IN */
{
    bool result;
    if (_mongoc_matcher_op_match(op->conditional.condition, bson)){
        result = _mongoc_matcher_op_match(op->conditional.iftrue, bson);
    } else {
        result = _mongoc_matcher_op_match(op->conditional.iffalse, bson);
    }
    return result;
}
#endif /*WITH_CONDITIONAL*/
