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


#include <bson.h>
#include "mongoc-bson-descendants.h"



/*
 *--------------------------------------------------------------------------
 *
 * bson_iter_find_descendant --
 *
 *       Locates a descendant using the "parent.child.key" notation. This
 *       operates similar to bson_iter_find() except that it can recurse
 *       into children documents using the dot notation.
 *
 * Returns:
 *       true if the descendant was found and @descendant was initialized.
 *
 * Side effects:
 *       @descendant may be initialized.
 *
 *--------------------------------------------------------------------------
 */

bool
bson_iter_find_descendants (bson_iter_t *iter,       /* INOUT */
                            const char  *dotkey,     /* IN */
                            int *skip,
                            bson_iter_t *descendant) /* OUT */
{
    bson_iter_t tmp, tmp2;
    const char *dot;
    bool found_descendant_doc = false;
    bool found_descendant_array = false;
    bool set_descendant = false;
    size_t sublen;

    BSON_ASSERT (iter);
    BSON_ASSERT (dotkey);
    BSON_ASSERT (descendant);

    if ((dot = strchr (dotkey, '.'))) {
        sublen = dot - dotkey;
    } else {
        sublen = strlen (dotkey);
    }

    if (_mongoc_bson_iter_find_with_len (iter, dotkey, (int)sublen)) {
        if (!dot) {
            set_descendant = true;
        } else if (BSON_ITER_HOLDS_DOCUMENT (iter)) {
            if (bson_iter_recurse (iter, &tmp)) {
                found_descendant_doc =  bson_iter_find_descendants (&tmp, dot + 1, skip, descendant);
                if (found_descendant_doc && (*skip > 0)){
                    found_descendant_doc = false;
                }
            }
        } else if (BSON_ITER_HOLDS_ARRAY(iter)){
            if (bson_iter_recurse (iter, &tmp)) {
                while (bson_iter_next(&tmp)){
                    if (bson_iter_recurse(&tmp, &tmp2)){
                        found_descendant_array = bson_iter_find_descendants (&tmp2, dot + 1, skip, descendant);
                        if (found_descendant_array && (*skip==0)){
                            break;
                        } else {
                            found_descendant_array = false;
                            (*skip)--;
                        }
                    }
                }
            }
        }
    }
    if (set_descendant)
        *descendant = *iter;

    return (found_descendant_doc || found_descendant_array || set_descendant);
}


/*
 * The code below this message is subject to the following copyright notice
 * License (ASL)
 *
 * https://github.com/mongodb/libbson/blob/50342ca0810910629709e7d86fefe5a7d5f7394a/src/bson/bson-iter.c
 *
 *
 * Copyright 2013-2014 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 *--------------------------------------------------------------------------
 *
 * _bson_iter_find_with_len --
 *
 *       Internal helper for finding an exact key.
 *
 * Returns:
 *       true if the field @key was found.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_bson_iter_find_with_len (bson_iter_t *iter,   /* INOUT */
                                const char  *key,    /* IN */
                                int          keylen) /* IN */
{
    const char *ikey;

    if (keylen == 0) {
        return false;
    }

    if (keylen < 0) {
        keylen = (int)strlen (key);
    }

    while (bson_iter_next (iter)) {
        ikey = bson_iter_key (iter);

        if ((0 == strncmp (key, ikey, keylen)) && (ikey [keylen] == '\0')) {
            return true;
        }
    }

    return false;
}