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
    //switch max points to 2 in geojson.h and this should segfault
    //BSON_ASSERT(compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [60.000, 10.000]}}",
    //                         "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"LineString\", \"coordinates\": [ [59.99999, 11.0000], [59.99999, 12.000], [59.99999, 13.000], [59.99999, 15.000]] }, \"$maxDistance\": 1000000.0}}}"));


    BSON_ASSERT(compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [60.000, 10.000]}}",
                             "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"LineString\", \"coordinates\": [ [59.99999, 11.0000], [59.99999, 12.000]] }, \"$maxDistance\": 1000000.0}}}"));


    BSON_ASSERT(compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [60.000, 10.000]}}",
                             "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"LineString\", \"coordinates\": [ [59.99999, 8.0000], [59.99999, 9.000]] }, \"$maxDistance\": 1000000.0}}}"));


    BSON_ASSERT(compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [60.000, 10.000]}}",
                             "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"LineString\", \"coordinates\": [ [59.99999, 8.0000], [59.99999, 12.000]] }, \"$maxDistance\": 1000000.0}}}"));

    BSON_ASSERT(!compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [20.000, 10.000]}}",
                             "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"LineString\", \"coordinates\": [ [59.99999, 8.0000], [59.99999, 12.000]] }, \"$maxDistance\": 1000000.0}}}"));




    BSON_ASSERT(compare_json("{\"loc\": {\"type\": \"Point\", \"coordinates\": [-61.08080307722216, -9.057610600760512]}}",
                             "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"Point\", \"coordinates\": [-61.08080307722216, -12.057610600760512]}, \"$maxDistance\": 100000000.0}}}"));



    BSON_ASSERT(!compare_json("{\"loc\": [{\"type\": \"Point\", \"coordinates\": [-61.08080307722216, -9.057610600760512]}]}",
                        "{\"loc\": {\"$near\": {\"$minDistance\": 0.0, \"$geometry\": {\"type\": \"Point\", \"coordinates\": [61.08080307722216, -12.057610600760512]}, \"$maxDistance\": 100000000.0}}}"));

//*/
    return 0;
}
