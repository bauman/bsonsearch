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

    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$date\": 1454621111904}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$gte\":{\"$date\": 1454621111904}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$lte\":{\"$date\": 1454621111904}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$gt\":{\"$date\": 1454621111903}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$lt\":{\"$date\": 1454621111905}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$ne\":{\"$date\": 1454621111905}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$in\": [{\"$date\": 1454621111904}]}}"));
    BSON_ASSERT(!compare_json("{\"dt\": {\"$date\": 1454621111904}}",
                             "{\"dt\": {\"$in\": [{\"$date\": 1454621111905}]}}"));
    BSON_ASSERT(compare_json("{\"dt\": [{\"$date\": 1454621111905}]}",
                             "{\"dt\": {\"$in\": [{\"$date\": 1454621111905}]}}"));
    return 0;
}