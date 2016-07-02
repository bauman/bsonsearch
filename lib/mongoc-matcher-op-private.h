/*
 * Copyright 2014 MongoDB, Inc.
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

#ifndef MONGOC_MATCHER_OP_PRIVATE_H
#define MONGOC_MATCHER_OP_PRIVATE_H

#include <bson.h>
#include <uthash.h>

#ifdef WITH_YARA
#include <yara.h>
#endif //WITH_YARA


BSON_BEGIN_DECLS


typedef union  _mongoc_matcher_op_t         mongoc_matcher_op_t;
typedef struct _mongoc_matcher_op_base_t    mongoc_matcher_op_base_t;
typedef struct _mongoc_matcher_op_logical_t mongoc_matcher_op_logical_t;
typedef struct _mongoc_matcher_op_compare_t mongoc_matcher_op_compare_t;
typedef struct _mongoc_matcher_op_exists_t  mongoc_matcher_op_exists_t;
typedef struct _mongoc_matcher_op_type_t    mongoc_matcher_op_type_t;
typedef struct _mongoc_matcher_op_size_t    mongoc_matcher_op_size_t;
typedef struct _mongoc_matcher_op_not_t     mongoc_matcher_op_not_t;
typedef struct _mongoc_matcher_op_near_t    mongoc_matcher_op_near_t;
typedef struct _mongoc_matcher_op_str_hashtable_t mongoc_matcher_op_str_hashtable_t;
typedef enum
{
   MONGOC_MATCHER_OPCODE_EQ,
   MONGOC_MATCHER_OPCODE_GT,
   MONGOC_MATCHER_OPCODE_GTE,
   MONGOC_MATCHER_OPCODE_IN,
   MONGOC_MATCHER_OPCODE_INSET,
#ifdef WITH_YARA
    MONGOC_MATCHER_OPCODE_YARA,
#endif //WITH_YARA
   MONGOC_MATCHER_OPCODE_LT,
   MONGOC_MATCHER_OPCODE_LTE,
   MONGOC_MATCHER_OPCODE_NE,
   MONGOC_MATCHER_OPCODE_NIN,
   MONGOC_MATCHER_OPCODE_OR,
   MONGOC_MATCHER_OPCODE_AND,
   MONGOC_MATCHER_OPCODE_NOT,
   MONGOC_MATCHER_OPCODE_NOR,
   MONGOC_MATCHER_OPCODE_EXISTS,
   MONGOC_MATCHER_OPCODE_TYPE,
   MONGOC_MATCHER_OPCODE_SIZE,
   MONGOC_MATCHER_OPCODE_STRLEN,
   MONGOC_MATCHER_OPCODE_NEAR,
   MONGOC_MATCHER_OPCODE_GEONEAR,
   MONGOC_MATCHER_OPCODE_GEOWITHIN,
   MONGOC_MATCHER_OPCODE_GEOWITHINPOLY,
   MONGOC_MATCHER_OPCODE_GEOUNDEFINED,
#ifdef WITH_PROJECTION
   MONGOC_MATCHER_OPCODE_PROJECTION,
   MONGOC_MATCHER_OPCODE_UNWIND,
#endif //WITH_PROJECTION
   MONGOC_MATCHER_OPCODE_UNDEFINED,
} mongoc_matcher_opcode_t;



typedef enum
{
    MONGOC_MATCHER_NEAR_UNDEFINED,
    MONGOC_MATCHER_NEAR_2D,
    MONGOC_MATCHER_NEAR_3D,
    MONGOC_MATCHER_NEAR_4D, //3D + Time
} mongoc_matcher_near_t;


struct _mongoc_matcher_op_base_t
{
   mongoc_matcher_opcode_t opcode;
};


struct _mongoc_matcher_op_logical_t
{
   mongoc_matcher_op_base_t base;
   mongoc_matcher_op_t *left;
   mongoc_matcher_op_t *right;
};


struct _mongoc_matcher_op_compare_t
{
   mongoc_matcher_op_base_t base;
   char *path;
   bson_iter_t iter;
   mongoc_matcher_op_str_hashtable_t *inset;
#ifdef WITH_YARA
   YR_RULES *rules;
   uint32_t timout;
   bool fast_mode;
#endif //WITH_YARA
};

#ifdef WITH_PROJECTION

typedef struct _mongoc_matcher_op_projection_t mongoc_matcher_op_projection_t;
struct _mongoc_matcher_op_projection_t
{
    mongoc_matcher_op_base_t base;
    char *path;
    mongoc_matcher_op_str_hashtable_t *pathlist;
    char* as;
    mongoc_matcher_op_t *next;
    mongoc_matcher_op_t *query;
};
#endif //WITH_PROJECTION

struct _mongoc_matcher_op_exists_t
{
   mongoc_matcher_op_base_t base;
   char *path;
   bool exists;
   mongoc_matcher_op_t *query;
};

struct _mongoc_matcher_op_size_t
{
    mongoc_matcher_op_base_t base;
    mongoc_matcher_opcode_t compare_type;
    char *path;
    u_int32_t size;
};

struct _mongoc_matcher_op_type_t
{
   mongoc_matcher_op_base_t base;
   bson_type_t type;
   char *path;
};


struct _mongoc_matcher_op_not_t
{
   mongoc_matcher_op_base_t base;
   mongoc_matcher_op_t *child;
   char *path;
};

struct _mongoc_matcher_op_near_t
{
    mongoc_matcher_op_base_t base;
    bson_iter_t iter;
    char *path;
    mongoc_matcher_near_t near_type;
    double x;
    double y;
    double z;
    double t; //normally for time, hijacked by some geo functions
    double maxd; //distance
    double mind; //distance
};

union _mongoc_matcher_op_t
{
   mongoc_matcher_op_base_t base;
   mongoc_matcher_op_logical_t logical;
   mongoc_matcher_op_compare_t compare;
   mongoc_matcher_op_exists_t exists;
   mongoc_matcher_op_type_t type;
   mongoc_matcher_op_size_t size;
   mongoc_matcher_op_near_t near;
   mongoc_matcher_op_not_t not_;
#ifdef WITH_PROJECTION
   mongoc_matcher_op_projection_t projection;
#endif //WITH_PROJECTION
};


struct _mongoc_matcher_op_str_hashtable_t {
    char* matcher_hash_key;
    UT_hash_handle hh;
};

mongoc_matcher_op_t * _mongoc_matcher_op_inset_new (const char              *path,   /* IN */
                                                    const bson_iter_t       *iter);
mongoc_matcher_op_t *_mongoc_matcher_op_logical_new (mongoc_matcher_opcode_t  opcode,
                                                     mongoc_matcher_op_t     *left,
                                                     mongoc_matcher_op_t     *right);
mongoc_matcher_op_t *_mongoc_matcher_op_compare_new (mongoc_matcher_opcode_t  opcode,
                                                     const char              *path,
                                                     const bson_iter_t       *iter);
mongoc_matcher_op_t *_mongoc_matcher_op_exists_new  (const char              *path,
                                                     bson_iter_t             *iter);
mongoc_matcher_op_t *_mongoc_matcher_op_type_new    (const char              *path,
                                                     bson_iter_t             *iter);

mongoc_matcher_op_t *_mongoc_matcher_op_size_new    (mongoc_matcher_opcode_t opcode,
                                                     const char              *path,
                                                     const bson_iter_t       *iter);
mongoc_matcher_op_t *_mongoc_matcher_op_not_new     (const char              *path,
                                                     mongoc_matcher_op_t     *child);
mongoc_matcher_op_t *_mongoc_matcher_op_near_new    (mongoc_matcher_opcode_t  opcode,
                                                     const char              *path,
                                                     const bson_iter_t       *iter,
                                                     double                  maxDistance);
bool                 _mongoc_matcher_op_match       (mongoc_matcher_op_t     *op,
                                                     const bson_t            *bson);
bool _mongoc_matcher_op_near_cast_number_to_double  (const bson_iter_t       *right_array,   /* IN */
                                                     double                  *maxDistance);   /* OUT*/
bool _mongoc_matcher_op_array_to_op_t               (const bson_iter_t       *iter,
                                                     mongoc_matcher_op_t     *op);
void                 _mongoc_matcher_op_destroy     (mongoc_matcher_op_t     *op);
void                 _mongoc_matcher_op_to_bson     (mongoc_matcher_op_t     *op,
                                                     bson_t                  *bson);
uint32_t _mongoc_matcher_op_size_get_iter_len       (bson_iter_t  *iter);


BSON_END_DECLS


#endif /* MONGOC_MATCHER_OP_PRIVATE_H */
