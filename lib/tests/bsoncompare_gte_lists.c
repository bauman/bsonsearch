#include <stdio.h>


#include <bsoncompare.h>



int compare_gt(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": [1,2,3]}";
    const char *jsonspec = "{\"hello\": {\"$gt\":1}}";
    doc = bson_new_from_json (json, -1, &error);
    spec = bson_new_from_json (jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_free(spec);
    bson_free(doc);
    return yes;
}
int compare_gte(){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;

    const char *json = "{\"hello\": [1,2,3]}";
    const char *jsonspec = "{\"hello\": {\"$gte\":3}}";
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
    BSON_ASSERT(compare_gt());
    BSON_ASSERT(compare_gte());

    return 0;
}