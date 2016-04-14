#include <bsoncompare.h>

bson_t *
project_json(const char *json,
                 const char *jsonspec ){
    bson_t * result;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    bson_t       out;
    doc = bson_new_from_json (json, -1, &error);
    spec = bson_new_from_json (jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    project_bson(matcher, doc, &out);
    matcher_destroy(matcher);
    doc_destroy(doc);
    doc_destroy(&out);
    bson_free(spec);
    bson_free(doc);
    return result;
}


int
main (int   argc,
      char *argv[])
{
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a\":1}}}"));

    BSON_ASSERT(project_json("{\"a\":[{\"aa\":[\"a\", 33]}, {\"aa\":999}], \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1,\"c\":1}}}"));

    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"a\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":\"a_aa\",\"c\":1}}}"));


    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"a\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1,\"c\":1}}}"));

    BSON_ASSERT(project_json("{\"a\":\"aa\", \"b\":\"b\"}",
                             "{\"$project\":{\"a\":1,\"c\":1}}}"));
    return 0;
}