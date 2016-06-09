#include <stdio.h>
#include <bson.h>
#include "mongoc-matcher.h"
#include "bsoncompare.h"
#include <uthash.h>
#ifdef WITH_YARA
#include <yara.h>
#endif //WITH_YARA
#ifdef WITH_PROJECTION
#include "mongoc-projection.h"
#endif //WITH_PROJECTION
#ifdef WITH_UTILS
#include "mongoc-matcher-op-geojson.h"
#include "mongoc-matcher-op-private.h"
#ifdef WITH_YARA //&& WITH_UTILS
#include <mongoc-matcher-op-yara.h>
#endif //WITH_YARA && WITH_UTILS
#endif //WITH_UTILS

// gcc -I/usr/include/libbson-1.0 -lbson-1.0 -lpcre -lyara -shared -o libbsoncompare.so -fPIC bsoncompare.c mongoc-matcher.c mongoc-matcher-op.c mongoc-matcher-op-geojson.c mongoc-matcher-op-yara.c


struct pattern_to_regex *global_compiled_regexes = NULL;

#ifdef WITH_PROJECTION
bool
project_bson(mongoc_matcher_t *matcher,     //in
             bson_t           *bson,        //in
             bson_t           *projected)   //out
{

    return mongoc_matcher_projection_execute(matcher->optree, bson, projected);
}
#endif //WITH_PROJECTION


#ifdef WITH_UTILS
#ifdef WITH_PROJECTION //&& UTILS
char *
bsonsearch_project_json(mongoc_matcher_t *matcher,     //in
                        bson_t           *bson)        //in
{
    bson_t * projected = bson_new();
    mongoc_matcher_projection_execute(matcher->optree, bson, projected);
    char * str;
    str = bson_as_json(projected, NULL);
    bson_destroy(projected);
    bson_free(projected);
    return str;
}
//call this to free the cstring from project_json
int
bsonsearch_free_project_str(void * ptr)
{
    bson_free(ptr);
    return 0;
}

bson_t *
bsonsearch_project_bson(mongoc_matcher_t *matcher,     //in
                        bson_t           *bson)        //in
{
    bson_t * projected = bson_new();
    mongoc_matcher_projection_execute(matcher->optree, bson, projected);
    return projected;
}




#endif //WITH_PROJECTION




double bsonsearch_haversine_distance(double lon1, double lat1, double lon2, double lat2)
{
    double result;
    if (!haversine_distance(lon1, lat1, lon2, lat2, &result)){
        result = (double)-1;
    }
    return result;
}

double bsonsearch_haversine_distance_degrees(double lon1, double lat1, double lon2, double lat2)
{
    double result;
    if (!haversine_distance(lon1*RADIAN_MAGIC_NUMBER, lat1*RADIAN_MAGIC_NUMBER, /*Defined in mongoc-matcher-op-geojson */
                            lon2*RADIAN_MAGIC_NUMBER, lat2*RADIAN_MAGIC_NUMBER,
                            &result)){
        result = (double)-1;
    }
    return result;
}
#ifdef WITH_YARA //&& WITH_UTILS
bool bsonsearch_yara_gte1_hit_raw(mongoc_matcher_t *matcher, char * line, ssize_t line_len)
{
    bool result;
    mongoc_matcher_op_binary_flo *bin_flo;
    bin_flo = (mongoc_matcher_op_binary_flo *)bson_malloc (sizeof *bin_flo);
    bin_flo->cursor_pos = 0;
    bin_flo->binary = (uint8_t*)line;
    bin_flo->binary_len = (uint32_t)line_len;
    result = _mongoc_matcher_op_yara_compare(&matcher->optree->compare, bin_flo);
    bson_free(bin_flo);
    return result;
}
#endif //WITH_YARA && WITH_UTILS
#endif //WITH_UTILS



int
bsonsearch_startup()
{
    int result = 0;
#ifdef WITH_YARA
    result += yr_initialize();
#endif //WITH_YARA
    return result;
}

int
bsonsearch_shutdown()
{
    int result = 0;
    result += regex_destroy();
#ifdef WITH_YARA
    result += yr_finalize();
#endif //WITH_YARA
    return result;
}


mongoc_matcher_t *
generate_matcher(const uint8_t *buf_spec,
                 uint32_t       len_spec)
{
  bson_t *spec;
  mongoc_matcher_t *matcher;
  spec = bson_new_from_data(buf_spec, (uint32_t)len_spec);
  matcher = mongoc_matcher_new (spec, NULL);
  bson_free(spec);
  return matcher;
}

mongoc_matcher_t *
generate_matcher_from_json(const uint8_t *buf_spec,
                            uint32_t       len_spec)
{
    bson_t *spec;
    mongoc_matcher_t *matcher;
    spec = bson_new_from_json(buf_spec, (uint32_t)len_spec, NULL);
    matcher = mongoc_matcher_new (spec, NULL);
    bson_free(spec);
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
bson_t *
generate_doc_from_json(const uint8_t *buf_doc,
                       uint32_t       len_doc)
{
    bson_t *doc;
    doc = bson_new_from_json (buf_doc, (uint32_t)len_doc, NULL);
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
    int freed = 0;
    struct pattern_to_regex *s, *tmp;
    HASH_ITER(hh, global_compiled_regexes, s, tmp) {
        HASH_DEL(global_compiled_regexes, s);
        pcre_free(s->re);     //malloc in _mongoc_matcher_iter_eq_match
        bson_free(s->pattern);//malloc in _mongoc_matcher_iter_eq_match
        free(s);
        freed++;
    }
    //TODO: slim chance s->re is Null?  Decided to let segfault for now to raise alarm
    return freed;
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
              const char *namespace,
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


