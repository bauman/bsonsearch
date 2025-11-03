
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
    doc_destroy(spec);
    doc_destroy(doc);
    return yes;
}


int
main (int   argc,
      char *argv[])
{
    bsonsearch_startup();
    int32_t rounds = 1;
    do {


        BSON_ASSERT(compare_json("{\"hello\": [ {\"world\": false}, {\"world\": true}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"sample\", \"config\":{\"$ratio\": 1.00}}}}"));

        BSON_ASSERT(!compare_json("{\"hello\": [ {\"world\": false}, {\"world\": true}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"sample\", \"config\":{\"$ratio\": 0.00}}}}"));

        // Fails because there is no hello.world subdoc even though the ratio should be 100%
        BSON_ASSERT(!compare_json("{\"hello\": [ {\"no\": false}, {\"nono\": true}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"sample\", \"config\":{\"$ratio\": 1.00}}}}"));



        rounds--;
    } while (rounds > 0);

    bsonsearch_shutdown();
    return 0;
}