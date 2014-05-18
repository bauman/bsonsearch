#include <bcon.h>
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>



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


bool
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


bool
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

