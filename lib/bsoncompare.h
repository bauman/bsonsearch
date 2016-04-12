#ifndef BSONCOMPARE_H
#define BSONCOMPARE_H

#include <bson.h>
#include <uthash.h>
#include <pcre.h>
#include "mongoc-matcher.h"

BSON_BEGIN_DECLS

mongoc_matcher_t * generate_matcher(const uint8_t *buf_spec, uint32_t  len_spec);
int                matcher_destroy (mongoc_matcher_t   *matcher);
int                doc_destroy  (bson_t *bson);
bson_t *           generate_doc(const uint8_t *buf_doc, uint32_t       len_doc);
int                matcher_compare(mongoc_matcher_t   *matcher, const uint8_t  *buf_bson,  uint32_t   len_bson);
int                matcher_compare_doc(mongoc_matcher_t  *matcher, bson_t  *bson);
int                compare(const uint8_t *buf_spec, uint32_t  len_spec, const uint8_t *buf_bson, uint32_t len_bson);
int                get_array_len(bson_t *b, const char *namespace, uint32_t      len_namespace);
int                regex_destroy();
int                regex_print();
int                bsonsearch_startup();
int                bsonsearch_shutdown();
#ifdef WITH_PROJECTION
bool project_bson(mongoc_matcher_t *matcher, bson_t *bson, bson_t *projected);
#endif //  WITH_PROJECTION

struct pattern_to_regex {
    char* pattern;            /* we'll use this field as the key */
    pcre * re;
    UT_hash_handle hh; /* makes this structure hashable */
};
extern struct pattern_to_regex *global_compiled_regexes;
BSON_END_DECLS


#endif /* BSONCOMPARE_H */
