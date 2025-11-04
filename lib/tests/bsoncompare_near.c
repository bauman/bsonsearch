#include <stdio.h>
#include <bsoncompare.h>

int compare_json(const char *json,
                 const char *jsonspec ){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;
}


int
main (int   argc,
      char *argv[])
{
    BSON_ASSERT(compare_json("{\"hello\": [1,3]}",
                             "{\"hello\": {\"$near\":[1,3], \"$maxDistance\":0}}"));
    BSON_ASSERT(compare_json("{\"hello\": [1,3,5]}",
                             "{\"hello\": {\"$near\":[1,3,5], \"$maxDistance\":0}}"));
    BSON_ASSERT(compare_json("{\"hello\": [1,3,5,7]}",
                             "{\"hello\": {\"$near\":[1,3,5,7], \"$maxDistance\":0}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1,3,5,7]}",
                             "{\"hello\": {\"$near\":[1,3,5,8], \"$maxDistance\":0}}"));
    BSON_ASSERT(compare_json("{\"hello\": [1,3]}",
                             "{\"hello\": {\"$near\":[1,4], \"$maxDistance\":2}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1,3]}",
                             "{\"hello\": {\"$near\":[1,10], \"$maxDistance\":2}}"));
    BSON_ASSERT(compare_json("{\"hello\": [1, 1, 3]}",
                             "{\"hello\": {\"$near\":[1, 1, 4], \"$maxDistance\":2}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1, 1, 3]}",
                              "{\"hello\": {\"$near\":[1, 1, 10], \"$maxDistance\":2}}"));
    return 0;
}