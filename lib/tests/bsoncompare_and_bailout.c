#include <stdio.h>
#include <bsoncompare.h>
#include <time.h>

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
    bson_free(spec);
    bson_free(doc);
    return yes;
}


int
main (int   argc,
      char *argv[])
{
    /*
     * first function will fail fast.  a:1 does not exist in the doc
     * therefore will not perform the a:3 a:4
     *
     * Important query optimization for the user to write $and to fail
     * as quickly as possible
     *
     * See 6.5.13 Item 4:Semantics
     * "...  If the first operand compares equal to 0,
     *       the second operand is not evaluated"
     *
     */
    /* Fail Early if you can */
    BSON_ASSERT(!compare_json("{\"a\": [3, 4]}",
                              "{\"$and\": [{\"a\": 1}, {\"a\": 3}, {\"a\": 4}]}"));

    /* Failing late causes more comparisons. */
    BSON_ASSERT(!compare_json("{\"a\": [3, 4]}",
                             "{\"$and\": [{\"a\": 4}, {\"a\": 3}, {\"a\": 1}]}"));

    return 0;
}