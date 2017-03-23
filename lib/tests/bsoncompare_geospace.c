#include <stdio.h>
#include <bsoncompare.h>

int compare_json(const char *json,
                 const char *jsonspec ){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json (json, -1, &error);
    spec = bson_new_from_json (jsonspec, -1, &error2);
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
    int i = 0;
    do {

        i++;
        printf ("run %d\n", i);



        BSON_ASSERT(compare_json(
                "{\"loc\": {\"type\": \"Point\", \"coordinates\": [2,2]}}",
                "{\"loc\": { \"$geoWithin\": { \"$polygon\": [ [ 0 , 0 ], [ 0 , 100 ], [ 100 , 100 ], [ 100, 0 ] ] } } }"));

        BSON_ASSERT(!compare_json(
                "{\"loc\": {\"type\": \"Point\", \"coordinates\": [-2,2]}}",
                "{\"loc\": { \"$geoWithin\": { \"$polygon\": [ [ 0 , 0 ], [ 0 , 100 ], [ 100 , 100 ], [ 100, 0 ] ] } } }"));
        BSON_ASSERT(compare_json(
                "{\"loc\": {\"type\": \"Polygon\", \"coordinates\": [ [[30, 99.9], [40, 40], [20, 40], [10, 20], [30, 10]] ]}}",
                "{\"loc\": { \"$geoWithin\": { \"$polygon\": [ [ 0 , 0 ], [ 0 , 100 ], [ 100 , 100 ], [ 100, 0 ] ] } } }"));
        BSON_ASSERT(compare_json(
                "{\"loc\": {\"type\": \"Polygon\", \"coordinates\": [ [[30, 10], [40, 40], [20, 40], [10, 20], [30, 10]] ]}}",
                "{\"loc\": { \"$geoWithin\": { \"$polygon\": [ [ 0 , 0 ], [ 0 , 100 ], [ 100 , 100 ], [ 100, 0 ] ] } } }"));
    } while (false);
    return 0;
}