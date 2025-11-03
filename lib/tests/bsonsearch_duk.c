
#include <stdio.h>
#include <bsoncompare.h>



int compare_json(const char *json,
                 const char *jsonspec ){
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t *)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t *)jsonspec, -1, &error2);
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

        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":\"not the right string variable\"}, {\"world\":\"variable\"}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n    return data == 'variable' ;\\n}\\n\"}}}}}"));

        BSON_ASSERT(compare_json("{\"hello\": [ {\"world\":{\"a\": \"not the right variable\"}}, {\"world\":{\"a\": \"variable\"}} ] }",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n  d = JSON.parse(data);\\n  return d.a == 'variable' ;\\n}\\n\"}}}}}"));

        BSON_ASSERT(!compare_json("{\"hello\": [ {\"world\":{\"a\": \"not the right variable\"}}, {\"nottherightkeybutgoodvalue\":{\"a\": \"variable\"}}, {\"world\":{\"a\": \"another wrong value\"}} ] }",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n  d = JSON.parse(data);\\n  return d.a == 'variable' ;\\n}\\n\"}}}}}"));


        /////////////////////////////////
        // This one is arguable, for now, leaving this to the matcher author to deal with.
        // the module will load data variable with 'variable' (not valid json - valid is '"variable"')
        // and will error trying to parse it.
        BSON_ASSERT(!compare_json("{\"hello\": [ {\"world\":{\"a\": \"not the right variable\"}}, {\"world\": \"variable\"} ] }",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n  d = JSON.parse(data);\\n  return d.a == 'variable' ;\\n}\\n\"}}}}}"));
        // Notice the actual javascript change that accounts for both cases and successfully matches
        // catch the json.parse exception and check the string variable as well.
        BSON_ASSERT(compare_json("{\"hello\": [ {\"world\":{\"a\": \"not the right variable\"}}, {\"world\": \"variable\"} ] }",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"dukjs\", \"config\":{\"entrypoint\": \"matches\", \"code\": {\"$code\": \"\\nfunction matches(data) {\\n  try { d = JSON.parse(data); return d.a == 'variable'; } catch (e) { return data == 'variable'; }\\n}\\n\"}}}}}"));
        /////////////////////////////////
        rounds--;
    } while (rounds > 0);

    bsonsearch_shutdown();
    return 0;
}