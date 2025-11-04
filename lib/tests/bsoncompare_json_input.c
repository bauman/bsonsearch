#include <bsoncompare.h>
#include <bson/bson.h>


bool
compare_json(const char *json,
              const char *jsonspec)
{
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *doc;
    doc = generate_doc_from_json ((const uint8_t*)json, strlen(json));

    mongoc_matcher_t * matcher = generate_matcher_from_json((const uint8_t*)jsonspec, strlen(jsonspec));
    int yes = matcher_compare_doc(matcher, doc);
    matcher_destroy(matcher);
    bson_free(doc);
    return yes;
}
int
main (int   argc,
      char *argv[])
{

    //test foundin command with deep doc
    do {
        BSON_ASSERT(compare_json("{\"dt\": 1, \"other\": 2 }",
                         "{\"dt\": 1}"));

        BSON_ASSERT(compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                                 "{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}"));

        BSON_ASSERT(!compare_json("{\"dt\": {\"$oid\": \"56b42b5be138236ac3127eda\"}}",
                                  "{\"dt\": {\"$oid\": \"56b42b5be138236ac3000000\"}}"));
    }while(0);




    return 0;
}