



#ifdef WITH_YARA
#ifndef MONGOC_MATCHER_OP_YARA_H
#define MONGOC_MATCHER_OP_YARA_H

#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"

BSON_BEGIN_DECLS
#define MONGOC_MATCHER_YARA_TIMEOUT_DEFAULT 300
#define MONGOC_MATCHER_YARA_FAST_MODE_DEFAULT true

typedef struct _mongoc_matcher_op_binary_flo         mongoc_matcher_op_binary_flo;
typedef struct _mongoc_matcher_op_yara_callback_data mongoc_matcher_op_yara_callback_data;
typedef struct _mongoc_matcher_op_yara_callback_hits mongoc_matcher_op_yara_callback_hits;
typedef struct _mongoc_matcher_op_yara_compiler_data mongoc_matcher_op_yara_compiler_data;

struct _mongoc_matcher_op_binary_flo
{
    const uint8_t * binary;
    uint32_t binary_len;
    uint32_t cursor_pos;
};
struct _mongoc_matcher_op_yara_callback_data
{
    int matches;
    mongoc_matcher_op_yara_callback_hits *next_hit;

};
struct _mongoc_matcher_op_yara_callback_hits
{
    char *hit_name;
    mongoc_matcher_op_yara_callback_hits *next_hit;
};

struct _mongoc_matcher_op_yara_compiler_data
{
    int errors;
    int warnings;
};

bool
_mongoc_matcher_op_yara_match (mongoc_matcher_op_compare_t *compare, /* IN */
                               bson_iter_t                 *iter)  ;  /* IN */

mongoc_matcher_op_t *
_mongoc_matcher_op_yara_new     ( const char              *path,   /* IN */
                                  bson_iter_t             *child);   /* IN */
bool
_mongoc_matcher_op_yara_compare(mongoc_matcher_op_compare_t *compare,
                                mongoc_matcher_op_binary_flo *bin_flo);

mongoc_matcher_op_t *
_mongoc_matcher_op_yara_new_op_from_bin     ( const char              *path,
                                              bson_iter_t             *child);

mongoc_matcher_op_t *
_mongoc_matcher_op_yara_new_op_from_string  ( const char              *path,
                                              bson_iter_t             *child);
        size_t
binary_read(void* ptr,
            size_t size,
            size_t count,
            void* user_data);

BSON_END_DECLS

#endif //MONGOC_MATCHER_OP_YARA_H
#endif //YARA

