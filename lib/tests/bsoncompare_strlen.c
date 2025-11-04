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
    bson_destroy(doc);
    bson_destroy(spec);
    return yes;
}


int
main (int   argc,
      char *argv[])
{

    BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
                             "{\"hello\": {\"$strlen\":5}}"));
    BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
                             "{\"hello\": {\"$strlen\":{\"$gte\":3}}}"));
    BSON_ASSERT(!compare_json("{\"hello\": \"world\"}",
                             "{\"hello\": {\"$strlen\":{\"$gt\":5}}}"));
    BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
                             "{\"hello\": {\"$strlen\":{\"$lte\":5}}}"));
    BSON_ASSERT(!compare_json("{\"hello\": \"world\"}",
                             "{\"hello\": {\"$strlen\":{\"$lt\":5}}}"));
    BSON_ASSERT(!compare_json("{\"hello\": \"world\"}",
                              "{\"hello\": {\"$strlen\":{\"$lt\":4}}}"));


    BSON_ASSERT(compare_json("{\"hel\": [{\"lo\":\"world\"}, {\"lo\":\"world!!\"}]}",
                              "{\"hel.lo\": {\"$strlen\":{\"$gte\":5}}}"));
    BSON_ASSERT(compare_json("{\"hel\": [{\"lo\":\"world\"}, {\"lo\":\"world!!\"}]}",
                             "{\"hel.lo\": {\"$strlen\":{\"$gte\":6}}}"));

    BSON_ASSERT(compare_json("{\"hello\": {\"$regex\": \"world\", \"$options\": \"\"}}",
                             "{\"hello\": {\"$strlen\":{\"$gte\":5}}}"));

    BSON_ASSERT(compare_json("{\"hello\": {\"$binary\": \"d29ybGQ=\", \"$type\": \"00\"}}",
                             "{\"hello\": {\"$strlen\":{\"$gte\":5}}}"));
    return 0;
}