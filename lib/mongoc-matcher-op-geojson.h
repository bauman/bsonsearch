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

#ifndef MONGOC_MATCHER_OP_GEOJSON_H
#define MONGOC_MATCHER_OP_GEOJSON_H

#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"

BSON_BEGIN_DECLS

#define MONGOC_EARTH_RADIUS_M 6371000
#define RADIAN_MAGIC_NUMBER 0.01745329251 //pi/180
#define DEGREE_MAGIC_NUMBER 57.29578 
#define HALF_PI  1.5707963267949
#define MONGOC_MAX_POLYGON_POINTS 2048

#define MONGOC_GEOJSON_POINT "Point"
#define MONGOC_GEOJSON_POINT_L 5

#define MONGOC_GEOJSON_LINESTRING "LineString"
#define MONGOC_GEOJSON_LINESTRING_L 10

mongoc_matcher_op_t *
_mongoc_matcher_op_geonear_new     ( const char      *path,   /* IN */
                                     bson_iter_t     *child);   /* IN */
mongoc_matcher_op_t *
_mongoc_matcher_op_geowithin_new     ( const char      *path,   /* IN */
                                       bson_iter_t     *child);   /* IN */

bool
_mongoc_matcher_op_geowithin_box_iter_values     ( bson_iter_t           box_iter,  /* IN */
                                                   mongoc_matcher_op_t   *op) ; /*OUT*/
bool  _mongoc_matcher_op_geonear_iter_values     ( bson_iter_t           near_iter,  /* IN */
                                                   mongoc_matcher_op_t   *op) ; /*OUT*/
bool _mongoc_matcher_op_geonear_parse_geometry     ( bson_iter_t           near_iter,  /* IN */
                                                     mongoc_matcher_op_t   *op) ; /*OUT*/

bool
_mongoc_matcher_op_geonear_parse_coordinates     ( bson_iter_t           *coordinate_iter,  /* IN */
                                                   mongoc_matcher_op_t   *op);  /*IN/OUT*/

bool
_mongoc_matcher_op_geowithin_polygon_iter_values     ( bson_iter_t           within_iter,  /* IN */
                                                       mongoc_matcher_op_t   *op) ; /*OUT*/
bool
_mongoc_matcher_op_geowithin_polygon_iter_point     ( bson_iter_t           within_iter,  /* IN */
                                                      mongoc_matcher_op_t   *op) ; /*OUT*/

bool
_mongoc_matcher_op_geowithinpoly (mongoc_matcher_op_t    *op, /* IN */
                                  const bson_t           *bson); /* IN */


bool haversine_distance(double lon1,  /* IN */
                        double lat1,  /* IN */
                        double lon2,  /* IN */
                        double lat2,  /* IN */
                        double *distance); /* OUT */
bool
point_in_poly(double nvert,
              mongoc_matcher_op_t *op,
              double testx, double testy);

bool _mongoc_matcher_op_geonear (mongoc_matcher_op_near_t    *near, /* IN */
                                 const bson_t                *bson) ;/* IN */
bool
_mongoc_matcher_op_near_boundary (mongoc_matcher_op_t         *op, /* IN */
                                      const bson_t                *bson); /* IN */
bool _mongoc_matcher_op_geowithin (mongoc_matcher_op_near_t    *near, /* IN */
                                   const bson_t                *bson); /* IN */

bool
bc_get_bearing(double lona, double lata, /* in */
               double lonb, double latb, /* in */
               double * output);         /* out */

bool
bc_crossarc( double lat1, double lon1, /* in */
             double lat2, double lon2, /* in */
             double lat3, double lon3, /* in */
             double *output);           /* out */
BSON_END_DECLS


#endif /* MONGOC_MATCHER_OP_GEOJSON_H */
