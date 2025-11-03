#include <stdio.h>


#include <bsoncompare.h>
#include <uthash.h>

int compare_rgx_good(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": \"world\"}";
    const char *jsonspec = "{\"hello\": {\"$options\": \"\", \"$regex\": \"orl\"}}";
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;

}
int compare_rgx_bad(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": \"world\"}";
    const char *jsonspec = "{\"hello\": {\"$options\": \"\", \"$regex\": \"oRl\"}}";
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;

}
int compare_rgx_good_case_insensitive(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": \"world\"}";
    const char *jsonspec = "{\"hello\": {\"$options\": \"i\", \"$regex\": \"oRl\"}}";
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;

}
int compare_rgx_good_in_list(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": [\"hello world\", 8]}";
    const char *jsonspec = "{\"hello\": {\"$options\": \"\", \"$regex\": \"orl\"}}";
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;

}
int check_precompiled_regex(){
    compare_rgx_good();
    compare_rgx_good();
    return 1;
}
int
main (int   argc,
      char *argv[])
{

    BSON_ASSERT(compare_rgx_good());
    regex_destroy();
    BSON_ASSERT(!compare_rgx_bad());
    regex_destroy();
    BSON_ASSERT(compare_rgx_good_case_insensitive());
    regex_destroy();
    BSON_ASSERT(compare_rgx_good_in_list());
    regex_destroy();

    check_precompiled_regex();
    exit(0);
}