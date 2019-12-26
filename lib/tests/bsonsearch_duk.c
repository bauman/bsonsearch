
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
    doc_destroy(spec);
    doc_destroy(doc);
    return yes;
}

/*
 * python code
 *
 *
 import bson
 match_js = bson.Code(
    """
        function matches() {
            return true;
        }
    """
 )
 bson.json_util.dumps({"code":match_js})

 */

int
main (int   argc,
      char *argv[])
{
    bsonsearch_startup();
    int32_t rounds = 1;
    do {

        BSON_ASSERT(compare_json("{\"hello\": {\"world\":\"variable\"}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n    return data == 'variable' ;\\n}\\n\"}}}}}"));


        BSON_ASSERT(!compare_json("{\"hello\": {\"world\":\"variable\"}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"not_real\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n    return data == 'variable' ;\\n}\\n\"}}}}}"));

        BSON_ASSERT(!compare_json("{\"hello\": {\"world\":\"variable\"}}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n    return 'not a boolean' ;\\n}\\n\"}}}}}"));

        BSON_ASSERT(compare_json("{\"hello\": {\"world\":{\"a\": \"variable\"}}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n  d = JSON.parse(data);\\n  return d.a == 'variable' ;\\n}\\n\"}}}}}"));


        rounds--;
    } while (rounds > 0);

    bsonsearch_shutdown();
    return 0;
}