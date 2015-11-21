#include <stdio.h>
#include <bson.h>
//#include <mongoc.h>
#include "mongoc-matcher.h"
#include <stdint.h>

// gcc $(pkg-config --cflags --libs libbson-1.0 libmongoc-1.0) -shared -o libbsoncompare.so -fPIC bsoncompare.c

mongoc_matcher_t *
generate_matcher(const uint8_t *buf_spec,
                 uint32_t       len_spec)
{
  bson_t *spec;
  mongoc_matcher_t *matcher;
  spec = bson_new_from_data(buf_spec, (uint32_t)len_spec);
  matcher = mongoc_matcher_new (spec, NULL);
  return matcher;
}

int
matcher_destroy (mongoc_matcher_t       *matcher)
{
  mongoc_matcher_destroy (matcher);
  return 0;
}


bson_t *
generate_doc(const uint8_t *buf_doc,
             uint32_t       len_doc)
{
  bson_t *doc;
  doc = bson_new_from_data(buf_doc, (uint32_t)len_doc);
  return doc;
}



int
doc_destroy (bson_t *bson)
{
  bson_destroy(bson);
  return 0;
}

int
matcher_compare(mongoc_matcher_t   *matcher,
                const uint8_t      *buf_bson,
                uint32_t           len_bson)
{
  bson_t *bson;
  bool result = false;
  bson = bson_new_from_data(buf_bson, (uint32_t)len_bson);
  result = mongoc_matcher_match (matcher, bson);
  bson_destroy(bson);
  return result;
}


int
matcher_compare_doc(mongoc_matcher_t   *matcher,
                    bson_t             *bson)
{
  bool result = false;
  result = mongoc_matcher_match (matcher, bson);
  return result;
}


int
compare(const uint8_t *buf_spec,
        uint32_t       len_spec,
        const uint8_t *buf_bson,
        uint32_t       len_bson)
{
  bson_t *spec;
  bson_t *bson;
  bool result = false;
  mongoc_matcher_t *matcher;
  spec = bson_new_from_data(buf_spec, (uint32_t)len_spec);
  bson = bson_new_from_data(buf_bson, (uint32_t)len_bson);
  matcher = mongoc_matcher_new (spec, NULL);
  result = mongoc_matcher_match (matcher, bson);
  bson_destroy(spec);
  bson_destroy(bson);
  mongoc_matcher_destroy (matcher);
  return result;
}


int
get_array_len(bson_t        *b,
              const uint8_t *namespace,
              uint32_t      len_namespace)
{
    int result = 0;
    bson_iter_t iter;
    bson_iter_t baz;
    if (bson_iter_init (&iter, b) &&
        bson_iter_find_descendant (&iter, namespace, &baz) &&
        BSON_ITER_HOLDS_ARRAY (&baz)) {

        bson_iter_t right_array;
        bson_iter_recurse(&iter, &right_array);
        while (bson_iter_next(&right_array)) {
            result++;
        }
    }
  return result;
}


