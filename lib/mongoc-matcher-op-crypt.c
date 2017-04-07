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
#include <sodium.h>
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-crypt.h"
#include "mongoc-bson-descendants.h"


mongoc_matcher_op_t *
_mongoc_matcher_op_crypt_new    (mongoc_matcher_opcode_t opcode,
                                const char              *path,
                                const bson_iter_t       *iter)
{

    mongoc_matcher_op_t *op = NULL;
    if (sodium_init() >= 0) { //-1=fail, 0=success, 1=already init
        bool op_complete = false;
        BSON_ASSERT (opcode == MONGOC_MATCHER_OPCODE_SEALOPEN);
        BSON_ASSERT (iter);
        op = (mongoc_matcher_op_t *) sodium_malloc(sizeof *op);
        op->base.opcode = opcode;
        op->crypt.path = bson_strdup (path);
        bson_iter_t child;
        if (bson_iter_recurse(iter, &child)) {
            op_complete = _mongoc_matcher_parse_crypt_loop(op, &child);
        }
        if (!op_complete){
            _mongoc_matcher_op_destroy(op);
            op = NULL;
        }
    }
    return op;
}

bool
_mongoc_matcher_parse_crypt_loop (mongoc_matcher_op_t *op,
                                  bson_iter_t         *iter)
{
    bool result = false;
    mongoc_matcher_op_t * query = NULL;
    bson_error_t * error = NULL;
    uint8_t crypt_ok = CRYPT_HAS_NONE;
    while (bson_iter_next(iter)) {
        const char * key = bson_iter_key(iter);
        bson_iter_t child;
        if (strcmp(key, "$query")==0 && BSON_ITER_HOLDS_DOCUMENT(iter)){
            if (bson_iter_recurse(iter, &child)&& bson_iter_next(&child)) {
                query = _mongoc_matcher_parse (&child, error);
            }
            if (query != NULL){
                crypt_ok |= CRYPT_HAS_QUERY;
                op->crypt.query = query;
            }
        } else if (strcmp(key, "$keys")==0 && BSON_ITER_HOLDS_DOCUMENT(iter)){
            if (bson_iter_recurse(iter, &child)) {
                crypt_ok |= _mongoc_matcher_parse_crypt_keys(op, &child);
            }
        }
    }
    if (crypt_ok == CRYPT_HAS_ALL){
        result = true;
    }
    return result;
}
uint8_t 
_mongoc_matcher_parse_crypt_keys (mongoc_matcher_op_t *op,
                                  bson_iter_t         *iter)

{
    uint8_t result = 0;
    while (bson_iter_next(iter)) {
        const char * key = bson_iter_key(iter);
        if (strcmp(key, "pk")==0){
            bson_subtype_t subtype;
            uint32_t binary_len=0;
            const uint8_t * binary;
            bson_iter_binary(iter, &subtype, &binary_len, &binary);
            if (binary_len == crypto_box_PUBLICKEYBYTES){
                strncpy(op->crypt.pk, (const char *)binary, binary_len);
                result |= CRYPT_HAS_PK;
            }
        } else if (strcmp(key, "sk")==0){
            bson_subtype_t subtype;
            uint32_t binary_len=0;
            const uint8_t * binary;
            bson_iter_binary(iter, &subtype, &binary_len, &binary);
            if (binary_len == crypto_box_SECRETKEYBYTES){
                strncpy(op->crypt.sk, (const char *)binary, binary_len);
                result |= CRYPT_HAS_SK;
            }
        }
    }
    return result;
}

bool
_mongoc_matcher_op_sealopen_match(mongoc_matcher_op_t *op,
                                  const bson_t         *bson)
{
    bson_iter_t tmp;
    bson_iter_t iter;
    bool found_one = false;
    int checked = 0, skip=0;
    BSON_ASSERT (op);
    BSON_ASSERT (bson);
    if (strchr (op->crypt.path, '.')) {
        if (!bson_iter_init (&tmp, bson) ||
            !bson_iter_find_descendant (&tmp, op->crypt.path, &iter)) { //try this way first
            while (!found_one &&
                   bson_iter_init (&tmp, bson) &&
                   bson_iter_find_descendants (&tmp, op->crypt.path, &skip, &iter)){
                found_one |= _mongoc_matcher_op_sealopen_match_iter(&op->crypt, &iter);
                skip = ++checked;
            }
            return ((checked>0) && found_one);
        }
    } else if (!bson_iter_init_find (&iter, bson, op->crypt.path)) {
        return false;
    }
    return _mongoc_matcher_op_sealopen_match_iter(&op->crypt, &iter);
}


bool
_mongoc_matcher_op_sealopen_match_iter (mongoc_matcher_op_crypt_t *crypt, /* IN */
                                        bson_iter_t               *iter)    /* IN */
{
    bool result = false;
    unsigned char * decrypted;
    if (BSON_ITER_HOLDS_BINARY(iter))
    {
        bson_subtype_t subtype;
        uint32_t binary_len=0;
        const uint8_t * binary;
        bson_iter_binary(iter, &subtype, &binary_len, &binary);
        decrypted = (unsigned char *) bson_malloc0(binary_len - crypto_box_SEALBYTES);
        if (crypto_box_seal_open(decrypted, binary, binary_len, (unsigned char *)crypt->pk, (unsigned char *)crypt->sk) ==0)
        {
            bson_t * innerbson = bson_new_from_data(decrypted, binary_len - crypto_box_SEALBYTES);
            if (innerbson != NULL){
                result = _mongoc_matcher_op_match(crypt->query, innerbson);
                bson_destroy(innerbson);
            }
        }
        bson_free(decrypted);
    }
    return result;
}

#endif /*WITH_CRYPT*/
