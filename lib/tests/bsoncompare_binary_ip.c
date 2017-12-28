#include <stdio.h>
#include <bsoncompare.h>

int compare_json(const char *json,
                 const char *jsonspec ){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json (json, -1, &error);
    //char * str;
    //str = bson_as_json(doc, NULL);
    //printf("%s\n", str);
    spec = bson_new_from_json (jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    doc_destroy(spec);
    doc_destroy(doc);
    return yes;
}


int
main (int   argc,
      char *argv[]) {

    BSON_ASSERT(compare_json("{\"i6\": {\"$binary\": \"EREiIjMzRERVVWZmd3eIiA==\", \"$type\": \"86\"}}",
                             "{\"i6\": {\"$inIPrange\": [{\"$binary\": \"EREiIjMzRERVVWZmd3eIiA==\", \"$type\": \"86\"}, {\"$binary\": \"/////////////////////w==\", \"$type\": \"86\"}]}}"));

    BSON_ASSERT(!compare_json("{\"i4\": {\"$binary\": \"AAAAAAAAAAAAAAAACgsMDQ==\", \"$type\": \"84\"}}",
                             "{\"i4\": {\"$inIPrange\": [{\"$binary\": \"AAAAAAAAAAAAAAAACgoAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAAAAAAA/////w==\", \"$type\": \"84\"}]}}"));

    BSON_ASSERT(compare_json("{\"i4\": {\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}}",
                             "{\"i4\": {\"$inIPrangeset\": [[{\"$binary\": \"AAAAAAAAAAAAAAoKAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAD/////AAAAAA==\", \"$type\": \"84\"}], [{\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAAAAP//AAAAAA==\", \"$type\": \"84\"}]]}}"));

    BSON_ASSERT(compare_json("{\"i6\": {\"$binary\": \"REQzMyIiERGIiHd3ZmZVVQ==\", \"$type\": \"86\"}}",
                             "{\"i6\": {\"$inIPrange\": [{\"$binary\": \"REQzMyIiERGIiHd3ZmZVVQ==\", \"$type\": \"86\"}, {\"$binary\": \"/////////////////////w==\", \"$type\": \"86\"}]}}"));


    BSON_ASSERT(!compare_json("{\"i4\": {\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}}",
                             "{\"i4\": {\"$inIPrangeset\": [[{\"$binary\": \"AAAAAAAAAAAAAAoKAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAD/////AAAAAA==\", \"$type\": \"84\"}], [{\"$binary\": \"AAAAAAAAAAAAAAoKAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAD/////AAAAAA==\", \"$type\": \"84\"}]]}}"));

    BSON_ASSERT(!compare_json("{\"i4\": {\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}}",
                             "{\"i4\": {\"$inIPrange\": [{\"$binary\": \"AAAAAAAAAAAAAAoKAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAD/////AAAAAA==\", \"$type\": \"84\"}]}}"));


    BSON_ASSERT(compare_json("{\"i4\": {\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}}",
                             "{\"i4\": {\"$inIPrange\": [{\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAAAAP//AAAAAA==\", \"$type\": \"84\"}]}}"));

    BSON_ASSERT(compare_json("{\"i4\": [{\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}]}",
                             "{\"i4\": {\"$inIPrange\": [{\"$binary\": \"AAAAAAAAAAAUFKjAAAAAAA==\", \"$type\": \"84\"}, {\"$binary\": \"AAAAAAAAAAAAAP//AAAAAA==\", \"$type\": \"84\"}]}}"));


    BSON_ASSERT(compare_json("{\"i6\": [{\"$binary\": \"AAAAALgNASApg0IAAP8AAA==\", \"$type\": \"86\"}]}",
                             "{\"i6\": {\"$inIPrange\": [{\"$binary\": \"AAAAALgNASApg0IAAP8AAA==\", \"$type\": \"86\"}, {\"$binary\": \"////////3/8AAAAAAAAAAA==\", \"$type\": \"86\"}]}}"));

    BSON_ASSERT(compare_json("{\"i6\": {\"$binary\": \"AAAAALgNASApg0IAAP8AAA==\", \"$type\": \"86\"}}",
                             "{\"i6\": {\"$inIPrange\": [{\"$binary\": \"AAAAALgNASApg0IAAP8AAA==\", \"$type\": \"86\"}, {\"$binary\": \"////////3/8AAAAAAAAAAA==\", \"$type\": \"86\"}]}}"));
return 0;
}