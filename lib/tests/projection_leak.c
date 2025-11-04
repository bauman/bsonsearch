#include <bsoncompare.h>
#include <bson/bson.h>


bool
test_bson_api(const char *json,
              const char *jsonspec)
{
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);

    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    bson_t * bson_out = NULL;
    //bson_out = bson_new();
    bson_out = bsonsearch_project_bson(matcher, doc);

    //project_bson(matcher, doc, bson_out);
    bson_destroy(bson_out);
    bson_free(bson_out);

    matcher_destroy(matcher);
    bson_destroy(doc);
    bson_destroy(spec);
    return true;
}


bool
test_json_api(const char *json,
              const char *jsonspec)
{
    bson_error_t error;
    bson_error_t error2;
    bson_t      *spec;
    bson_t      *doc;
    doc = bson_new_from_json ((const uint8_t*)json, -1, &error);
    spec = bson_new_from_json ((const uint8_t*)jsonspec, -1, &error2);
    const uint8_t *spec_bson = bson_get_data(spec);

    mongoc_matcher_t * matcher = generate_matcher(spec_bson, spec->len);
    char* projected_json = bsonsearch_project_json(matcher, doc);

    matcher_destroy(matcher);
    bson_destroy(doc);
    bson_destroy(spec);
    bson_free(projected_json);
    return true;
}

int
main (int   argc,
      char *argv[])
{
    uint32_t i = 1;
    uint32_t j = 0;
    do {
        j++;
        // BSON_ASSERT(test_bson_api("{\"FN Info Creation date\": \"\", \"Parent File Rec. Seq. #\": \"5\", \"Filename #4\": \"\", \"Filename #3\": \"\", \"Filename #2\": \"\", \"Filename #1\": \"/$MFT\", \"Filename\": \"True\", \"Std Info Access date\": \"2013-04-23 19:09:18.476946\", \"Log/Notes\": \"\", \"Reparse Point\": \"False\", \"Birth Object ID\": \"\", \"Good\": \"Good\", \"Index Root\": \"False\", \"Index Allocation\": \"False\", \"FN Info Modify date\": \"\", \"FN Info Entry date\": \"\", \"Record type\": \"File\", \"Birth Domain ID\": \"\", \"Active\": \"Active\", \"Data\": \"False\", \"EA Information\": \"False\", \"uSec Zero\": \"N\", \"Logged Utility Stream\": \"False\", \"ADS\": \"N\", \"Attribute List\": \"False\", \"Record Number\": \"0\", \"STF FN Shift\": \"N\", \"EA\": \"False\", \"Object ID\": \"False\", \"FN Info Modification date\": \"2013-04-23 19:09:18.476946\", \"Std Info Entry date\": \"2013-04-23 19:09:18.476946\", \"Sequence Number\": \"1\", \"Standard Information\": \"True\", \"Std Info Modification date\": \"2013-04-23 19:09:18.476946\", \"FN Info Access date\": \"\", \"Std Info Creation date\": \"2013-04-23 19:09:18.476946\", \"Volume Info\": \"False\", \"Bitmap\": \"True\", \"Birth Volume ID\": \"\", \"Parent File Rec. #\": \"5\", \"Volume Name\": \"False\", \"Property Set\": \"False\"}",
        //                           "{\"$project\": {\"filename\": {\"$foundin\": [\"Filename #1\", \"Filename #2\", \"Filename #3\", \"Filename #4\"]}}}"));
        BSON_ASSERT(test_json_api("{\"FN Info Creation date\": \"\", \"Parent File Rec. Seq. #\": \"5\", \"Filename #4\": \"\", \"Filename #3\": \"\", \"Filename #2\": \"\", \"Filename #1\": \"/$MFT\", \"Filename\": \"True\", \"Std Info Access date\": \"2013-04-23 19:09:18.476946\", \"Log/Notes\": \"\", \"Reparse Point\": \"False\", \"Birth Object ID\": \"\", \"Good\": \"Good\", \"Index Root\": \"False\", \"Index Allocation\": \"False\", \"FN Info Modify date\": \"\", \"FN Info Entry date\": \"\", \"Record type\": \"File\", \"Birth Domain ID\": \"\", \"Active\": \"Active\", \"Data\": \"False\", \"EA Information\": \"False\", \"uSec Zero\": \"N\", \"Logged Utility Stream\": \"False\", \"ADS\": \"N\", \"Attribute List\": \"False\", \"Record Number\": \"0\", \"STF FN Shift\": \"N\", \"EA\": \"False\", \"Object ID\": \"False\", \"FN Info Modification date\": \"2013-04-23 19:09:18.476946\", \"Std Info Entry date\": \"2013-04-23 19:09:18.476946\", \"Sequence Number\": \"1\", \"Standard Information\": \"True\", \"Std Info Modification date\": \"2013-04-23 19:09:18.476946\", \"FN Info Access date\": \"\", \"Std Info Creation date\": \"2013-04-23 19:09:18.476946\", \"Volume Info\": \"False\", \"Bitmap\": \"True\", \"Birth Volume ID\": \"\", \"Parent File Rec. #\": \"5\", \"Volume Name\": \"False\", \"Property Set\": \"False\"}",
                                  "{\"$project\": {\"filename\": {\"$foundin\": [\"Filename #1\", \"Filename #2\", \"Filename #3\", \"Filename #4\"]}}}"));
    }while(j<i);

    return 0;
}