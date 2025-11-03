#include <bsoncompare.h>
#include <bson/bson.h>
//valgrind -v --leak-check=full <this>
bool
test_bson_api(const char *json,
              const char *jsonspec)
{
    bool same = false;
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
      char *argv[]) {

    //test $any command with list projections
    do {
        BSON_ASSERT(test_bson_api("{\"a\": {\"x\": {\"f\":1}, \"y\": {\"f\":2}, \"z\": {\"f\":3}}}",
                                  "{\"$unwind\": {\"$project\": {\"a\": {\"$foundin\": [\"a.$any.f\"]}}, \"$query\": {\"a\": 2}}}"));
    } while (false);//*/

    //test $any command with list projections
    do {
        BSON_ASSERT(!test_bson_api("{\"a\": [{\"c\": 1, \"b\": 1}, {\"c\": 2, \"b\": 2}]}",
                                  "{\"$unwind\": {\"$project\": {\"a\": {\"$foundin\": [\"a\"]}}, \"$query\": {\"$and\": [{\"a.b\": 1}, {\"a.c\": 2}]}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"c\": 1, \"b\": 1}, {\"c\": 2, \"b\": 2}]}",
                                  "{\"$unwind\": {\"$project\": {\"a\": {\"$foundin\": [\"a\"]}}, \"$query\": {\"$and\": [{\"a.b\": 2}, {\"a.c\": 2}]}}}"));
    } while (false);//*/

    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"c\": 1, \"b\": 1}, {\"c\": 2, \"b\": 2}]}",
                                  "{\"$unwind\": {\"$project\": {\"a\": 1}, \"$query\": {\"$and\": [{\"a.b\": 2}, {\"a.c\": 2}]}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"c\": 1, \"b\": 1}, {\"c\": 2, \"b\": 2}]}",
                                  "{\"$unwind\": {\"$project\": {\"a\": \"aa\"}, \"$query\": {\"$and\": [{\"aa.b\": 2}, {\"aa.c\": 2}]}}}"));
    } while (false);//*/
    exit(0);
}