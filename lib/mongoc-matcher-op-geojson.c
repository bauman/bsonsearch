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
#include <math.h>
#include "mongoc-error.h"
#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-geojson.h"
#include "mongoc-matcher-op-private.h"



/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geonear_new --
 *
 *       Create a new op for checking {$near: [x,y], $maxDistance: n}.
 *
 * Returns:
 *       A newly allocated mongoc_matcher_op_t that should be freed with
 *       _mongoc_matcher_op_destroy().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_matcher_op_t *
_mongoc_matcher_op_geonear_new     ( const char              *path,   /* IN */
                                     bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_t *op;
    bson_iter_t near_iter;

    op = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *op);
    op->near.base.opcode = MONGOC_MATCHER_OPCODE_GEONEAR;
    op->near.near_type = MONGOC_MATCHER_NEAR_UNDEFINED;
    op->near.path = bson_strdup (path);
    if (!bson_iter_recurse (child, &near_iter) ){
        return NULL;
    }
    _mongoc_matcher_op_geonear_iter_values(near_iter, op);
    return op;
}

bool
_mongoc_matcher_op_geonear (mongoc_matcher_op_near_t    *near, /* IN */
                            const bson_t                *bson) /* IN */
{
    bool retval = false;
    bson_iter_t iter;
    bson_iter_t desc;
    mongoc_matcher_op_t *right_op;
    double distance;
    BSON_ASSERT (near);
    BSON_ASSERT (bson);

    if (bson_iter_init (&iter, bson) &&
            bson_iter_find_descendant (&iter, near->path, &desc) &&
            BSON_ITER_HOLDS_DOCUMENT(&iter))
    {
        right_op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *right_op);
        if (_mongoc_matcher_op_geonear_parse_geometry(desc, right_op) &&
                haversine_distance(near->x, near->y, right_op->near.x, right_op->near.y, &distance) &&
            distance <= near->maxd)
            _mongoc_matcher_op_destroy(right_op);
        retval = true;
    }
    return retval;
}

bool
_mongoc_matcher_op_geonear_iter_values     ( bson_iter_t           near_iter,  /* IN */
                                              mongoc_matcher_op_t   *op)  /*OUT*/
{
    while (bson_iter_next (&near_iter)) {
        const char * key = bson_iter_key (&near_iter);
        if (strcmp(key, "$minDistance")==0){
            if (!_mongoc_matcher_op_near_cast_number_to_double(&near_iter, &op->near.mind))
                return false;
        } else if (strcmp(key, "$maxDistance")==0){
            if (!_mongoc_matcher_op_near_cast_number_to_double(&near_iter, &op->near.maxd))
                return false;
        } else if (strcmp(key, "$geometry")==0){
            if (!_mongoc_matcher_op_geonear_parse_geometry(near_iter, op))
                return false;
        }
    }
    return true;
}
bool
_mongoc_matcher_op_geonear_parse_geometry     ( bson_iter_t           near_iter,  /* IN */
                                                mongoc_matcher_op_t   *op)  /*OUT*/
{
    bson_iter_t geometry_iter;
    if (!bson_iter_recurse (&near_iter, &geometry_iter)) {
        return false;
    }
    while (bson_iter_next (&geometry_iter)) {
        const char * key = bson_iter_key (&geometry_iter);
        if (strcmp(key, "coordinates")==0){
            //better be longitude/latitude
            bson_iter_t coordinate_iter;
            if (!bson_iter_recurse (&geometry_iter, &coordinate_iter) ||
                    !bson_iter_next(&coordinate_iter)||
                    !_mongoc_matcher_op_near_cast_number_to_double(&coordinate_iter, &op->near.x))
                return false;
            op->near.x *= RADIAN_MAGIC_NUMBER;
            if (!bson_iter_next(&coordinate_iter) ||
                    !_mongoc_matcher_op_near_cast_number_to_double(&coordinate_iter, &op->near.y))
                return false;
            else {op->near.near_type = MONGOC_MATCHER_NEAR_2D;}
            op->near.y *= RADIAN_MAGIC_NUMBER;
        } else if (strcmp(key, "type")==0){ //compiler should nuke this if until it's filled out.
            //TODO: THIS LIBRARY ONLY HANDLES POINTS.  More Types need added.
        }
    }
    return true;
}

//https://en.wikipedia.org/wiki/Haversine_formula
// 0.5*(1-cos(2*x)) = sin^2(x) <-trig identity to replace sin^2 from formula
bool
haversine_distance(double lon1,      /* IN  (radians)*/
                   double lat1,      /* IN  (radians)*/
                   double lon2,      /* IN  (radians)*/
                   double lat2,      /* IN  (radians)*/
                   double *distance) /* OUT */
{
    double lon_diff = lon2 - lon1;
    double lat_diff = lat2 - lat1;
    double under_the_root = 0.5*(1-cos(2*(lat_diff/2))) + cos(lat1) * cos(lat2) * 0.5*(1-cos(2*(lon_diff/2))) ;
    double over_the_root  = sqrt(under_the_root);
    if (over_the_root > 1.0)
        over_the_root = 1.0;
    double pre_radius_unit = 2 * asin(over_the_root);
    (*distance) = pre_radius_unit * MONGOC_EARTH_RADIUS_M;
    return true;
}