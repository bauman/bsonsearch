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
    do {
        BSON_ASSERT(!test_bson_api("{\"a\": 1}",
                                   "{\"a\": {\"$exists\": {\"a\":{\"$lte\":0}}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": 1}",
                                  "{\"b\": {\"$exists\": {\"b\":{\"$gte\":0}}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}, {\"d\":1}]}",
                                  "{\"a.f\": {\"$exists\": {\"a.f\":{\"$gte\":0}}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}, {\"d\":1}]}",
                                  "{\"a.c\": {\"$exists\": {\"a.c\":{\"$gte\":0}}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}, {\"c\":100}]}",
                                  "{\"a.c\": {\"$exists\": {\"a.c\":{\"$gte\":90}}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}, {\"d\":1}]}",
                                  "{\"a.d\": {\"$exists\": {\"a.c\":1}}}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}, {\"d\":1}]}",
                                  "{\"a.d\": {\"$exists\": true }}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": [{\"b\":1}, {\"c\":1}]}",
                                   "{\"a.b\": {\"$exists\": true }}"));
    } while (false);//*/
    do {
        BSON_ASSERT(test_bson_api("{\"a\": {\"x\": {\"f\":1}, \"y\": {\"f\":2}, \"z\": {\"f\":3}}}",
                                  "{\"a.x.f\": {\"$exists\": true }}"));
    } while (false);//*/
    do {
        BSON_ASSERT(!test_bson_api("{\"a\": {\"x\": {\"f\":1}, \"y\": {\"f\":2}, \"z\": {\"f\":3}}}",
                                  "{\"a.x.e\": {\"$exists\": true }}"));
    } while (false);//*/
    exit(0);
}