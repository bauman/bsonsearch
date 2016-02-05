#include <stdio.h>
#include <bson.h>
#include "mongoc-matcher.h"
#include "bsoncompare.h"
#include <uthash.h>
// gcc -I/usr/include/libbson-1.0 -lbson-1.0 -lpcre -shared -o libbsoncompare.so -fPIC bsoncompare.c mongoc-matcher.c mongoc-matcher-op.c


struct pattern_to_regex *global_compiled_regexes = NULL;

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
  if (matcher != NULL)
  {
      mongoc_matcher_destroy (matcher);
  }

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
regex_destroy()
{
    struct pattern_to_regex *s, *tmp;
    HASH_ITER(hh, global_compiled_regexes, s, tmp) {
        HASH_DEL(global_compiled_regexes, s);
        pcre_free(s->re);
        free(s);
    }
    //TODO: slim chance s->re is Null?  Decided to let segfault for now to raise alarm
    //      in future, return number of regexes had to free, or number of errors.  I don't know.
    return 0;
}
int
regex_print()
{
    struct pattern_to_regex *s, *tmp;
    HASH_ITER(hh, global_compiled_regexes, s, tmp) {
        printf("The Pattern:(%s) \n", s->pattern);
    }
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
  mongoc_matcher_destroy (matcher); //TODO: This will segfault on a spec that doesnt create a matcher
                                    //      DESIRED! I want this thing to fail noisy for now, until I have a plan.
  bson_destroy(spec);
  bson_destroy(bson);
  return result;
}


int
get_array_len(bson_t        *b,
              const uint8_t *namespace,
              uint32_t      len_namespace)
{
    //TODO: libbson uses const char * dotkey.  I use const uint8_t * namespace.
    //      I believe libbson will be changing char*'s to uint8_t at some point.
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


