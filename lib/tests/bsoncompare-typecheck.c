#include <bsoncompare.h>
#include <bson/bson.h>

//valgrind -v --leak-check=full <this>
bool
test_bson_api(const char *json,
              bson_t *spec)
{
    bool same = false;
    bson_error_t error;
    bson_error_t error2;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    const uint8_t *spec_bson = bson_get_data(spec);
    const uint8_t *doc_bson = bson_get_data(doc);
    int yes = compare(spec_bson, spec->len, doc_bson, doc->len);
    bson_destroy(doc);
    return yes;
}
int
main (int   argc,
      char *argv[]) {
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","integer", "}");
        BSON_ASSERT(test_bson_api("{\"a\": 1000000000000000000}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","integer", "}");
        BSON_ASSERT(!test_bson_api("{\"a\": 10.34}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","number", "}");
        BSON_ASSERT(test_bson_api("{\"a\": 1000000000000000000}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","long", "}");
        BSON_ASSERT(test_bson_api("{\"a\": 1000000000000000000}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","int", "}");
        BSON_ASSERT(test_bson_api("{\"a\": 1}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type",BCON_INT64(1), "}");
        BSON_ASSERT(test_bson_api("{\"a\": 1000000000000000000}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    do {
        bson_t spec;
        bson_init(&spec);
        BCON_APPEND(&spec, "a", "{","$type","", "}");
        BSON_ASSERT(test_bson_api("{\"a\": \"a string\"}",&spec));
        bson_destroy(&spec);
    } while (false);//*/
    exit(0);
}