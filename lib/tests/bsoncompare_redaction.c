#include <bsoncompare.h>
#include <bson/bson.h>

bool
project_json(const char *json,
             const char *jsonspec,
             const char *expected){
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    bson_t      *out = bson_new();
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    project_bson(matcher, doc, out);
    char * str;
    str = bson_as_canonical_extended_json(out, NULL);
    same = (0 == strcmp(str, expected));
    matcher_destroy(matcher);
    doc_destroy(spec);
    doc_destroy(doc);
    bson_destroy(out);
    bson_free(str);
    return same;
}

bool
test_json_api(const char *json,
         const char *jsonspec,
         const char *expected)
{
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);

    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    char * out = NULL;
    out = bsonsearch_project_json(matcher, doc);
    same = (0 == strcmp(out, expected));
    matcher_destroy(matcher);
    bson_destroy(doc);
    bson_destroy(spec);
    bsonsearch_free_project_str(out);
    return same;
}
bool
test_bson_api(const char *json,
              const char *jsonspec,
              const char *expected)
{
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);

    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    bson_t * bson_out = NULL;
    bson_out = bsonsearch_project_bson(matcher, doc);

    size_t json_len;
    char * out = bson_as_canonical_extended_json(bson_out, &json_len);
    same = (0 == strcmp(out, expected));
    matcher_destroy(matcher);
    bson_destroy(doc);
    bson_destroy(spec);
    bson_destroy(bson_out);
    bson_free(bson_out);
    bsonsearch_free_project_str(out);
    return same;
}
int
main (int   argc,
      char *argv[])
{

    BSON_ASSERT(project_json("{\"a\":\"aa\", \"b\": {\"bb\":\"b\"}}",
                             "{\"$redact\":{\"a\":1,\"c\":1}}",
                             "{ \"b\" : [ { \"bb\" : \"b\" } ] }"));


    BSON_ASSERT(project_json("{\"a\":\"aa\", \"b\":\"b\"}",
                             "{\"$redact\":{\"a\":1,\"c\":1}}",
                             "{ \"b\" : [ \"b\" ] }"));



    return 0;
}