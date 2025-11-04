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

    BSON_ASSERT(compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$gt\":2}}"));//should skip "abc"
    BSON_ASSERT(!compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$gt\":4}}"));
    BSON_ASSERT(compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$gt\":-1}}"));//should stop at 1st item in list

    BSON_ASSERT(compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$gte\":3}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$gte\":4}}"));

    BSON_ASSERT(compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$lt\":2}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$lt\":1}}"));


    BSON_ASSERT(compare_json("{\"hello\": [1,2,3]}", "{\"hello\": {\"$lte\":4}}"));
    BSON_ASSERT(!compare_json("{\"hello\": [1,\"abc\",3]}", "{\"hello\": {\"$lte\":0}}"));

    return 0;
}