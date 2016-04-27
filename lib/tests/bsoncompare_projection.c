#include <bsoncompare.h>

bool
project_json(const char *json,
             const char *jsonspec,
             const char *expected){
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    bson_t      *out = bson_new();
    doc = bson_new_from_json (json, -1, &error);
    spec = bson_new_from_json (jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    project_bson(matcher, doc, out);

    char * str;
    str = bson_as_json(out, NULL);
    same = (0 == strcmp(str, expected));
    matcher_destroy(matcher);
    doc_destroy(spec);
    doc_destroy(doc);
    doc_destroy(out);
    bson_free(str);
    return same;
}

bool
test_api(const char *json,
         const char *jsonspec,
         const char *expected)
{
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json (json, -1, &error);
    spec = bson_new_from_json (jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);

    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    char * out = NULL;
    out = bsonsearch_project_bson(matcher, doc);
    same = (0 == strcmp(out, expected));
    matcher_destroy(matcher);
    bson_destroy(doc);
    bson_destroy(spec);
    bsonsearch_free_project_str(out);
    return same;
}
int
main (int   argc,
      char *argv[])
{

    //test foundin command with deep doc
    BSON_ASSERT(project_json("{\"a\":[{\"aa\":[\"ii\", 33]}], \"b\":\"b\", \"c\":[\"33\",44]}",
                             "{\"$project\":{\"zzz\":{\"foundin\":[\"a.aa\", \"c\", \"b\"]}}}",
                             "{ \"zzz\" : [ \"ii\", 33, \"33\", 44, \"b\" ] }"));


    //test foundin command with deep doc
    BSON_ASSERT(project_json("{\"a\":[{\"aa\":[\"ii\", 33]}], \"b\":\"b\", \"c\":[\"33\",44]}",
                             "{\"$project\":{\"zzz\":{\"foundin\":[\"c\",\"a.aa\"]}}}",
                             "{ \"zzz\" : [ \"33\", 44, \"ii\", 33 ] }"));



    //test foundin command
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", 33]}, \"b\":\"b\", \"c\":[\"33\",44]}",
                             "{\"$project\":{\"zzz\":{\"foundin\":[\"c\",\"a.aa\"]}}}",
                             "{ \"zzz\" : [ \"33\", 44, \"ii\", 33 ] }"));


    //test foundin command
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", 33]}, \"b\":\"b\", \"c\":[\"b\",44]}",
                             "{\"$project\":{\"zz\":{\"foundin\":[\"c\",\"a.aa\"]}}}",
                             "{ \"zz\" : [ \"b\", 44, \"ii\", 33 ] }"));


    //test document
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", 33]}, \"b\":\"b\", \"c\":[\"b\",44]}",
                             "{\"$project\":{\"zz\":{\"foundin\":[\"b\",\"c\"]}}}",
                             "{ \"zz\" : [ \"b\", \"b\", 44 ] }"));

    //test direct descent
    do
    {
        BSON_ASSERT(test_api("{\"a\":{\"aa\":[2, 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1,\"c\":1}}}",
                             "{ \"a.aa\" : [ 2, 33 ], \"c\" : [  ] }"));
    }while(false); //true to leak test

    //test regex
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", {\"$options\": \"\", \"$regex\": \"oRl\"}]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1}}}",
                             "{ \"a.aa\" : [ \"ii\", { \"$regex\" : \"oRl\", \"$options\" : \"\" } ] }"));

    //test document
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"ii\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a\":1}}}",
                             "{ \"a\" : [ { \"aa\" : [ \"ii\", 33 ] } ] }"));
    //test complex descent
    BSON_ASSERT(project_json("{\"a\":[{\"aa\":[\"a\", 33]}, {\"aa\":999}], \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1,\"c\":1}}}",
                             "{ \"a.aa\" : [ \"a\", 33, 999 ], \"c\" : [  ] }"));

    //test project as
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"a\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":\"a_aa\",\"c\":1}}}",
                             "{ \"a_aa\" : [ \"a\", 33 ], \"c\" : [  ] }"));

    //test direct descent
    BSON_ASSERT(project_json("{\"a\":{\"aa\":[\"a\", 33]}, \"b\":\"b\"}",
                             "{\"$project\":{\"a.aa\":1,\"c\":1}}}",
                             "{ \"a.aa\" : [ \"a\", 33 ], \"c\" : [  ] }"));

    //test root keys
    BSON_ASSERT(project_json("{\"a\":\"aa\", \"b\":\"b\"}",
                             "{\"$project\":{\"a\":1,\"c\":1}}}",
                             "{ \"a\" : [ \"aa\" ], \"c\" : [  ] }"));

    //test date_time

    //test int64

    //test datetime

    //test double

    //test oid

    //test bool

    //test binary  */

    return 0;
}