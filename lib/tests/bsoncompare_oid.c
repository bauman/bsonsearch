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
    bson_free(spec);
    bson_free(doc);
    return yes;
}


int
main (int   argc,
      char *argv[])
{



    /* Direct Equal Compare */
    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}"));

    BSON_ASSERT(compare_json("{\"dt\": [{\"$oid\": \"56b4c534e138236ac3127edd\"}, {\"$oid\": \"56b4c539e138236ac3127ede\"}]}",
                             "{\"dt\": {\"$oid\": \"56b4c539e138236ac3127ede\"}}"));

    BSON_ASSERT(!compare_json("{\"dt\": [{\"$oid\": \"56b4c534e138236ac3127edd\"}, {\"$oid\": \"56b4c539e138236ac3127ede\"}]}",
                             "{\"dt\": {\"$oid\": \"57b41534e138236ac3127edd\"}}"));


    /* Greater thans */
    BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                              "{\"dt\": {\"$gte\":{\"$oid\": \"57b42b5be138236ac3127edb\"}}}"));



    BSON_ASSERT(compare_json("{\"dt\": [{\"$oid\": \"56b4c534e138236ac3127edd\"}, {\"$oid\": \"56b4c539e138236ac3127ede\"}]}",
                              "{\"dt\": {\"$gte\":{\"$oid\": \"56b4c534e138236ac3127ede\"}}}"));


    BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$gte\":{\"$oid\": \"56b42b5be138236ac3127edc\"}}}"));


    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$gte\":{\"$oid\": \"00042b5be138236ac3127eda\"}}}"));


    /*lte */
    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$lte\":{\"$oid\": \"56b42b5be138236ac3127eda\"}}}"));

    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$lte\":{\"$oid\": \"56b42b5be138236ac3127edb\"}}}"));
    BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$lte\":{\"$oid\": \"56b42b5be138236ac3127ed0\"}}}"));

    /* lt */
    BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$lt\":{\"$oid\": \"56b42b5be138236ac3127eda\"}}}"));

    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                              "{\"dt\": {\"$lt\":{\"$oid\": \"56b42b5be138236ac3127edb\"}}}"));

    BSON_ASSERT(compare_json("{\"dt\": [{\"$oid\": \"56b4c534e138236ac3127ede\"}, {\"$oid\": \"56b4c534e138236ac3127edd\"}]}",
                             "{\"dt\": {\"$lt\":{\"$oid\": \"56b4c534e138236ac3127ede\"}}}"));

    /* gt */
    BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                             "{\"dt\": {\"$gt\":{\"$oid\": \"56b42b5be138236ac3127eda\"}}}"));
    BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                              "{\"dt\": {\"$gt\":{\"$oid\": \"56b42b5be138236ac3127ed0\"}}}"));
    BSON_ASSERT(compare_json("{\"dt\": [{\"$oid\": \"56b4c534e138236ac3127edd\"}, {\"$oid\": \"56b4c534e138236ac3127ede\"}]}",
                             "{\"dt\": {\"$gt\":{\"$oid\": \"56b4c534e138236ac3127edd\"}}}"));



    return 0;
}