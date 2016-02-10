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
    double distance = -1; //distance must be > 0 to pass the compare.
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
            retval = true;
        _mongoc_matcher_op_destroy(right_op);
    }
    return retval;
}

bool
_mongoc_matcher_op_geonear_iter_values     ( bson_iter_t           near_iter,  /* IN */
                                              mongoc_matcher_op_t   *op)  /*OUT*/
{
    while (bson_iter_next (&near_iter)) {
        const char * key = bson_iter_key (&near_iter);
        if (strcmp(key, "$geometry")==0){
            if (!_mongoc_matcher_op_geonear_parse_geometry(near_iter, op))
                return false;
        } else if (strcmp(key, "$maxDistance")==0){
            if (!_mongoc_matcher_op_near_cast_number_to_double(&near_iter, &op->near.maxd))
                return false;
        } else if (strcmp(key, "$minDistance")==0){
            if (!_mongoc_matcher_op_near_cast_number_to_double(&near_iter, &op->near.mind))
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


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithin_new --
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
_mongoc_matcher_op_geowithin_new     ( const char              *path,   /* IN */
                                       bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_t *op;
    bson_iter_t within_iter;
    op = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *op);
    op->near.base.opcode = MONGOC_MATCHER_OPCODE_GEOUNDEFINED;
    op->near.near_type = MONGOC_MATCHER_NEAR_UNDEFINED;
    op->near.path = bson_strdup (path);

    if (!bson_iter_recurse (child, &within_iter)){
        _mongoc_matcher_op_destroy(op);
        return NULL;
    }
    while (bson_iter_next (&within_iter)) {
        const char * key = bson_iter_key (&within_iter);
        if (strcmp(key, "$box")==0 &&
                _mongoc_matcher_op_geowithin_box_iter_values(within_iter, op)) {
            op->near.near_type = MONGOC_MATCHER_NEAR_2D;
            op->base.opcode = MONGOC_MATCHER_OPCODE_GEOWITHIN;
            return op;
        } else if (strcmp(key, "$polygon")==0 &&
                BSON_ITER_HOLDS_ARRAY(&within_iter) &&
                _mongoc_matcher_op_geowithin_polygon_iter_values(within_iter, op)) {
            op->near.near_type = MONGOC_MATCHER_NEAR_2D;
            op->base.opcode = MONGOC_MATCHER_OPCODE_GEOWITHINPOLY;
            return op;
        }
    }
    return NULL;
}

//iterate through the list of lists
bool
_mongoc_matcher_op_geowithin_polygon_iter_values     ( bson_iter_t           within_iter,  /* IN */
                                                       mongoc_matcher_op_t   *op)  /*OUT*/
{
    bson_iter_t box_iter;
    mongoc_matcher_op_t **current_op;
    current_op = &op;
    op->near.maxd = 0;
    if (bson_iter_recurse(&within_iter, &box_iter)){
        while (bson_iter_next(&box_iter)){
            op->near.maxd++;
            mongoc_matcher_op_t *next_point;
            next_point = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *next_point);
            next_point->near.base.opcode = MONGOC_MATCHER_OPCODE_GEOWITHINPOLY;
            next_point->near.near_type = MONGOC_MATCHER_NEAR_UNDEFINED;
            bool suc = _mongoc_matcher_op_geowithin_polygon_iter_point(box_iter, next_point);
            (*current_op)->logical.left = next_point;
            current_op = &(*current_op)->logical.left;
        }
    }
    return true;
}

bool
_mongoc_matcher_op_geowithin_polygon_iter_point    ( bson_iter_t           within_iter,  /* IN */
                                                     mongoc_matcher_op_t   *op)  /*OUT*/
{
    bson_iter_t box_iter;
    if (BSON_ITER_HOLDS_ARRAY(&within_iter) &&
            bson_iter_recurse(&within_iter, &box_iter) &&
            bson_iter_next(&box_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&box_iter, &op->near.x) &&
            bson_iter_next(&box_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&box_iter, &op->near.y)){
        return true;
    }
    return false;
}
bool
_mongoc_matcher_op_geowithin_box_iter_values     ( bson_iter_t           within_iter,  /* IN */
                                                   mongoc_matcher_op_t   *op)  /*OUT*/
{
    bson_iter_t box_iter, top_left_iter, bottom_right_iter;
    if (BSON_ITER_HOLDS_ARRAY(&within_iter) &&
            bson_iter_recurse(&within_iter, &box_iter) &&
            bson_iter_next(&box_iter) &&
            BSON_ITER_HOLDS_ARRAY(&box_iter) &&
            bson_iter_recurse(&box_iter, &top_left_iter) &&
            bson_iter_next(&top_left_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&top_left_iter, &op->near.x) &&
            bson_iter_next(&top_left_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&top_left_iter, &op->near.y) &&
            bson_iter_next(&box_iter) &&
            BSON_ITER_HOLDS_ARRAY(&box_iter) &&
            bson_iter_recurse(&box_iter, &bottom_right_iter) &&
            bson_iter_next(&bottom_right_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&bottom_right_iter, &op->near.z) &&
            bson_iter_next(&bottom_right_iter) &&
            _mongoc_matcher_op_near_cast_number_to_double(&bottom_right_iter, &op->near.t)){
        return true;
    }
    return false;
}
bool
_mongoc_matcher_op_geowithin (mongoc_matcher_op_near_t    *near, /* IN */
                              const bson_t                *bson) /* IN */
{
    bson_iter_t iter;
    bson_iter_t desc;
    mongoc_matcher_op_t *right_op;
    bool return_val = false;
    BSON_ASSERT (near);
    BSON_ASSERT (bson);

    if (bson_iter_init (&iter, bson) &&
        bson_iter_find_descendant (&iter, near->path, &desc) &&
        BSON_ITER_HOLDS_ARRAY (&desc))
    {
        right_op = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *right_op);
        right_op->base.opcode = MONGOC_MATCHER_OPCODE_NEAR;
        if (_mongoc_matcher_op_array_to_op_t(&desc, right_op) &&
                (near->near_type == right_op->near.near_type))
        {
            switch (near->near_type){
                case MONGOC_MATCHER_NEAR_2D:
                    if (right_op->near.x >= near->x &&
                            right_op->near.y >= near->y &&
                            right_op->near.x <= near->z &&
                            right_op->near.y <= near->t){
                        return_val = true;
                    }
                    break;
                default:
                    break;
            }
        }
        _mongoc_matcher_op_destroy(right_op);
    }
    return return_val;
}

bool
_mongoc_matcher_op_geowithinpoly (mongoc_matcher_op_t    *op, /* IN */
                                  const bson_t           *bson) /* IN */
{
    bson_iter_t iter;
    bson_iter_t desc;
    mongoc_matcher_op_t *right_op;
    bool return_val = false;
    BSON_ASSERT (op);
    BSON_ASSERT (bson);

    if (bson_iter_init (&iter, bson) &&
        bson_iter_find_descendant (&iter, op->near.path, &desc) &&
        BSON_ITER_HOLDS_ARRAY (&desc))
    {
        right_op = (mongoc_matcher_op_t *)bson_malloc0 (sizeof *right_op);
        right_op->base.opcode = MONGOC_MATCHER_OPCODE_GEOWITHINPOLY;
        if (_mongoc_matcher_op_array_to_op_t(&desc, right_op) &&
                (op->near.near_type == right_op->near.near_type)){
            return_val = point_in_poly(op->near.maxd, op, right_op->near.x, right_op->near.y);
        }

        _mongoc_matcher_op_destroy(right_op);
    }
    return return_val;
}

/*
 * ---------------------------------------------------------------
 * The below section of code is in great need of optimization
 * ---------------------------------------------------------------
 */
static bool
get_near_poly(mongoc_matcher_op_t *op,      //in
              int depth,                    //in
              int dimension,                //in
              double * output)              //out
{
    switch (depth){
        case 0:
        {
            switch (dimension){
                case 1:
                {
                    (*output) = op->near.x;
                    return true;
                }
                case 2:
                {
                    (*output) = op->near.y;
                    return true;
                }
                default:
                    return false;
            }
        }
        default:
            if (op->logical.left != NULL)
                return get_near_poly(op->logical.left, --depth, dimension, output);
            else
                return false;
    }
}
/*
 * The code below this message is subject to the following copyright notice
 * License (BSD)
 *
 * Based on the algorithm Described here:
 * https://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
 *
 *
 * Copyright (c) 1970-2003, Wm. Randolph Franklin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
 * 2. Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other materials provided with the distribution.
 * 3. The name of W. Randolph Franklin may not be used to endorse or promote products derived from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
bool
point_in_poly(double nvert,
              mongoc_matcher_op_t *op,
              double testx, double testy)
{
    uint8_t i, j;
    bool c = false;
    if (nvert <= MONGOC_MAX_POLYGON_POINTS)
    {
        for (i = 0, j = nvert-1; i < nvert; j = i++) {
            double verty_i, verty_j, vertx_i, vertx_j;
            if ( get_near_poly(op->logical.left, i, 2, &verty_i)&&
                 get_near_poly(op->logical.left, j, 2, &verty_j) &&
                 get_near_poly(op->logical.left, i, 1, &vertx_i) &&
                 get_near_poly(op->logical.left, j, 1, &vertx_j) &&
                 ((verty_i>testy) != (verty_j>testy)) &&
                 (testx < (vertx_j-vertx_i) * (testy-verty_i) / (verty_j-verty_i) + vertx_i) )
                c = !c;
        }
    }

    return c;
}