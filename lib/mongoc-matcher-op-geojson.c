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
 *       Create a new op for checking {$near: {<GeoJSON-dict>},
 *                                     $maxDistance: n,
 *                                     $minDistance: n }.
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

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geonear --
 *
 *      called by the mongoc-matcher to determine if 2 points are within
 *      maxDistance of eachother
 *
 * Returns:
 *      bool: true if the points are within maxDistance ofeachother
 *            false otherwise
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geonear_iter_values --
 *
 *       parses the geojson spec containing geoJSON
 *
 *       bson_iter_t should be pointing inside (recursed into) the document
 *                  but not yet pointed to the first key
 *
 *       example bson_iter_t pointer:
 *          {key: {  $geometry: {"coordinates": [...],"$maxDistance": number  }}
 *                 ^
 *          -------^
 * Returns:
 *       *op is updated with coordnates and min/max distances
 *       bool if op's coordinates & distances are to be trusted
 *
 * Notes:
 *      GeoJSON query container information can be found
 *      https://docs.mongodb.org/manual/reference/command/geoNear/
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
bool
_mongoc_matcher_op_geonear_iter_values     ( bson_iter_t           near_iter,  /* IN */
                                              mongoc_matcher_op_t   *op)  /*IN/OUT*/
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
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geonear_parse_geometry --
 *
 *       parses the geojson coordinates.
 *
 *
 *
 *       bson_iter_t should be pointing just before the outside list
 *          prior to this call.
 *
 *       example bson_iter_t pointer:
 *          {$geometry: {...,"coordinates": [[x1,y1],[x2,y2],...,[xN, yN]]}}
 *                     ^
 *          -----------^
 * Returns:
 *          *op is updated with coordnates
 *          bool if op's coordinates are to be trusted
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
bool
_mongoc_matcher_op_geonear_parse_geometry     ( bson_iter_t           near_iter,  /* IN */
                                                mongoc_matcher_op_t   *op)  /*IN/OUT*/
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
/*
 *--------------------------------------------------------------------------
 *
 * haversine_distance --
 *
 *       Compute the distance using the haversine formula given the
 *       latritude and longitude (IN RADIANS!!!)

 *
 * Returns:
 *       resultant distance placed in double *distance
 *       bool (true if could be computed and *distance should be trusted
 *              false if the distance could not be computed)
 *
 * Notes:
 *      This function is currently applicable ONLY to the EARTH sphere
 *      MONGOC_EARTH_RADIUS_M is defined in mongoc-matcher-op-geojson
 *      This variable cannot be overridden at runtime to work on other
 *      spherical bodies
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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
 *       Create a new op for checking {$near: <GeoJSON>, $maxDistance: n}.
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
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithin_polygon_iter_values --
 *
 *       iterates through the list of points that would make a polygon
 *       bson_iter_t should be pointint to a list of unlimited lists
 *          each inside list should contain exactly 2 numbers
 *
 *
 *       bson_iter_t should be pointing just before the outside list
 *          prior to this call.
 *
 *       example bson_iter_t pointer:
 *          {...,"coordinates": [[x1,y1],[x2,y2],...,[xN, yN]]}
 *                             ^
 *          -------------------^
 *
 * Returns:
 *       mongoc_matcher_op_t which has the box values populated in op->near
 *       bool:  true (op is correctly populated)
 *              false (the numbers in op should not be trusted)
 *
 * Notes:
 *      see http://geojson.org/geojson-spec.html#examples forr example
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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
            _mongoc_matcher_op_geowithin_polygon_iter_point(box_iter, next_point);//todo: should check this response
            (*current_op)->logical.left = next_point;
            current_op = &(*current_op)->logical.left;
        }
    }
    return true;
}
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithin_polygon_iter_point --
 *
 *       extracts the coordinate values and implicity casts to double
 *
 *       bson_iter_t should be pointing just before the list item prior
 *       to this call.
 *
 *       example bson_iter_t pointer:
 *          "coordinates": [102.0, 0.5]}
 *                        ^
 *          --------------^
 *
 * Returns:
 *       mongoc_matcher_op_t which has the box values populated in op->near
 *       bool:  true (op is correctly populated)
 *              false (the numbers in op should not be trusted)
 *
 * Notes:
 *      calls _mongoc_matcher_op_near_cast_number_to_double from
 *              mongoc-matcher-op.c for cast to double
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithin_box_iter_values --
 *
 *       function extracts the lower left coordinate
 *       and the upper right coordinate
 *       from bson_iter_t which should contain a list of exactly 2 lists
 *                        each should contain exactly 2 values.
 *
 *                        example: [[x,y],[x,y]]
 *
 *       implicitly casts these numbers into a (double) type and stores them
 *       in the mongoc_matcher_op_t output
 *
 * Returns:
 *       mongoc_matcher_op_t which has the box values populated in op->near
 *       bool:  true (op is correctly populated)
 *              false (the numbers in op should not be trusted)
 *
 * Notes:
 *      calls _mongoc_matcher_op_near_cast_number_to_double from
 *              mongoc-matcher-op.c for cast to double
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithin --
 *
 *       Compares the defined spec which is a polygon with exactly sides
 *       and is defined by the $box key containing the coordinates
 *       of the bottom left and upper right corners of the box
 *
 * Returns:
 *       A newly allocated mongoc_matcher_op_t that should be freed with
 *       _mongoc_matcher_op_destroy().
 *
 * Notes:
 *       This function is faster compared to the polygon spec
 *       because it does not need to call a recursive function based
 *       on the number of sides to the polygon for every document.
 *
 *       However,the $box query is not supported by GeoJSON spec and is
 *       MongoDB specific command.
 *
 *       More info found here:
 *       https://docs.mongodb.org/manual/reference/operator/query/box/
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_geowithinpoly --
 *
 *       Compares the defined spec which is a polygon with unlimited sides
 *           limited by systems max recursion depth (generally stack size)
 *           to the bson document.
 *
 *       This function must only be called if mongoc_matcher_op is a
 *           geojson compare type in op.base.type
 *
 * Returns:
 *       bool: true if the bson document matches the spec,
 *             false otherwise
 *
 * Notes:  False can mean the point is NOT within the spec,
 *                        the bson document did not contain a point
 *                        the bson doc was invalid.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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

/*
 *--------------------------------------------------------------------------
 *
 * get_near_poly --
 *
 *       pulls the requested point out of the mongoc_matcher_op_t tree
 *
 * Returns:
 *       double * output places the value found to this pointer's value
 *       bool (true if the requested point is found and output is populated
 *             false otherwise)
 *
 * Notes:
 *      Recursive Function
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
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
 *--------------------------------------------------------------------------
 *
 * point_in_poly --
 *
 *       Determine whether or not a point is within a given bounding box
 *
 * Returns:
 *       bool: True if point within box, false if point not within box.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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