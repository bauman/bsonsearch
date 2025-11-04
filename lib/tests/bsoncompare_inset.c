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



    /* Direct Equal Compare */
    BSON_ASSERT(!compare_json("{\"dt\": [\"2\"]}",
                              "{\"dt\": {\"$inset\": [\"ok1\", 2, 4.44, \"ok3\"]}}"));

    BSON_ASSERT(!compare_json("{\"dt\": [\"ok0\"]}",
                             "{\"dt\": {\"$inset\": [\"ok1\", \"ok2\", \"ok3\"]}}"));

   BSON_ASSERT(compare_json("{\"dt\": [\"ok4\",\"ok3\"]}",
                             "{\"dt\": {\"$inset\": [\"ok1\", \"ok2\", \"ok3\"]}}"));

    BSON_ASSERT(!compare_json("{\"dt\": \"ok4\"}",
                             "{\"dt\": {\"$inset\": [\"ok1\", \"ok2\", \"ok3\"]}}"));

    BSON_ASSERT(compare_json("{\"dt\": \"ok2\"}",
                              "{\"dt\": {\"$inset\": [\"ok1\", \"ok2\", \"ok3\"]}}"));

    return 0;
}