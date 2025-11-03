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
    //BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
    //                         "{\"$cond\": {\"then\": {\"world\": \"hello\"}, \"else\": {\"$cond\": {\"then\": {\"hello\": \"world\"}, \"else\": {\"hello\": {\"$exists\": true}}, \"if\": {\"hello\": {\"$exists\": true}}}}, \"if\": {\"world\": {\"$exists\": true}}}}"));

    BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
                             "{\"$cond\":{\"if\": {\"hello\":{\"$exists\":true}}, \"then\": {\"hello\":\"world\"}, \"else\": {\"hello\":{\"$exists\":true}} }}"));
    BSON_ASSERT(compare_json("{\"hello\": \"world\"}",
                             "{\"$cond\":{\"if\": {\"hello\":{\"$exists\":false}}, \"then\": {\"hello\":\"world\"}, \"else\": {\"hello\":{\"$exists\":true}} }}"));
    return 0;
}