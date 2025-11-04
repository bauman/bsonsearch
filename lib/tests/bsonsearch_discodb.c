
#include <stdio.h>
#include <bsoncompare.h>

/*
 *  The discodb is made up of the following key<space>value pairs
key1 value1
key2 value2
key value
key3 value3
key2 value22
key2 something-else
 */

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
    doc_destroy(spec);
    doc_destroy(doc);
    return yes;
}


int
main (int   argc,
      char *argv[])
{
    bsonsearch_startup();
    int32_t rounds = 1;
    do {
        /* $Q is easiest obtained using the python module
         *      from discodb import Q
         *      from json import dumps
         *      q = Q.parse("A")
         *      dumps(a.deploy())
         *
         *          '{"num_clauses": 1, "clauses": [{"num_terms": 1, "terms": [{"entry": "A", "nnot": false}]}]}'
         *
         */
        // Passes  BECAUSE string whale is in mammals and aquatic
        BSON_ASSERT(compare_json("{\"hello\": {\"world\":[\"not in the database\", \"Nýx\", \"Shouldnt get here\"]}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$ddb\": \"/tmp/myths.ddb\", \"$Q\": {\"num_clauses\": 2, \"clauses\": [{\"num_terms\": 1, \"terms\": [{\"entry\": \"Greek\", \"nnot\": false}]}, {\"num_terms\": 1, \"terms\": [{\"entry\": \"Primordial\", \"nnot\": false}]}], \"precache\": true }}}}}"));


        // Passes  BECAUSE string whale is in mammals and aquatic
        BSON_ASSERT(compare_json("{\"hello\": {\"world\":\"Nýx\"}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$ddb\": \"/tmp/myths.ddb\", \"$Q\": {\"num_clauses\": 2, \"clauses\": [{\"num_terms\": 1, \"terms\": [{\"entry\": \"Greek\", \"nnot\": false}]}, {\"num_terms\": 1, \"terms\": [{\"entry\": \"Primordial\", \"nnot\": false}]}], \"precache\": true }}}}}"));



        // Passes  BECAUSE string whale is in mammals and aquatic
        BSON_ASSERT(compare_json("{\"hello\": {\"world\":\"Nýx\"}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$Q\": {\"num_clauses\": 2, \"clauses\": [{\"num_terms\": 1, \"terms\": [{\"entry\": \"Greek\", \"nnot\": false}]}, {\"num_terms\": 1, \"terms\": [{\"entry\": \"Primordial\", \"nnot\": false}]}]}, \"$ddb\": \"/tmp/myths.ddb\"}}}}"));



        // Passes  BECAUSE string whale is in mammals and aquatic
        BSON_ASSERT(compare_json("{\"hello\": {\"world\":\"whale\"}}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$Q\": {\"num_clauses\": 2, \"clauses\": [{\"num_terms\": 1, \"terms\": [{\"entry\": \"mammals\", \"nnot\": false}]}, {\"num_terms\": 1, \"terms\": [{\"entry\": \"aquatic\", \"nnot\": false}]}]}, \"$ddb\": \"/tmp/animals.ddb\"}}}}"));



        // PASSES  BECAUSE string whale is is in aquatic and the query is for pets or aquatic, need to loop pets first though
        BSON_ASSERT(compare_json("{\"hello\": {\"world\":\"whale\"}}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$Q\": {\"num_clauses\": 1, \"clauses\": [{\"num_terms\": 2, \"terms\": [{\"entry\": \"aquatic\", \"nnot\": false}, {\"entry\": \"pets\", \"nnot\": false}]}]}, \"$ddb\": \"/tmp/animals.ddb\"}}}}"));


        // Fails  BECAUSE string whale is in a mammal but is aquatic and the query includes not aquatic
        BSON_ASSERT(!compare_json("{\"hello\": {\"world\":\"whale\"}}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$Q\": {\"num_clauses\": 2, \"clauses\": [{\"num_terms\": 1, \"terms\": [{\"entry\": \"mammals\", \"nnot\": false}]}, {\"num_terms\": 1, \"terms\": [{\"entry\": \"aquatic\", \"nnot\": true}]}]}, \"$ddb\": \"/tmp/animals.ddb\"}}}}"));



        // Passes  BECAUSE string key3 (2nd world) only has one value "value3" in the ddb
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key3\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value3\", \"$ddb\": \"/tmp/sample.ddb\"}}}}"));




        // Fails  BECAUSE none of the world keys exist in the ddb
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"b2s=\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value2\", \"$ddb\": \"/tmp/sample.ddb\"}}}}"));



        // Passes  BECAUSE string key3 (2nd world) only has one value "value3" in the ddb
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key3\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value3\", \"$ddb\": \"/tmp/sample.ddb\"}}}}"));






        // Fails  BECAUSE none of the world keys exist in the ddb
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"b2s=\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));



        // Passes  BECAUSE string key3 (2nd world) only has one value "value3" in the ddb
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key3\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value3\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));


        // Passes  BECAUSE binary key3 (3rd world) only has one value "value3" in the ddb
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key3\"}, {\"world\":{\"$binary\": \"a2V5Mw==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value3\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));



        // Fails  BECAUSE there are multiple non-correct one of the values for binary key2 (3rd world) is
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));



        // Fails  BECAUSE there are multiple non-correct one of the values for key2 is
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key2\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueonly\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));




        // Fails BECAUSE integer 20, string key72 and bytes "ok" are not in the ddb
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"b2s=\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueis\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));


        // PASSES BECAUSE one of the values for key2 is value2
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key2\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueis\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));


        // PASSES BECAUSE one of the values for key2 (in the 3rd binary world) is value2
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$valueis\": \"value2\", \"$ddb\": {\"$binary\": \"W94UHeZb2AQMAQAAAAAAAAQAAAAGAAAABgAAAAAAAAAAAAAAAAAAAEgAAAAAAAAApAAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIsAAAAAAAAAlwAAAAAAAACkAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgMAAADBAwMAAABrZXkBAAAAYQQAAABrZXkzAQAAAILcAAAAAAAAAOIAAAAAAAAA6AAAAAAAAADtAAAAAAAAAPMAAAAAAAAA+gAAAAAAAAAIAQAAAAAAAHZhbHVlMXZhbHVlMnZhbHVldmFsdWUzdmFsdWUyMnNvbWV0aGluZy1lbHNlAAAAAA==\", \"$type\": \"00\"}}}}}"));



        // FAILS BECAUSE integer 20, string key72 and bytes "ok" are not in the ddb
        BSON_ASSERT(!compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key72\"}, {\"world\":{\"$binary\": \"b2s=\", \"$type\": \"00\"}}]}",
                                  "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$keyexists\": 0, \"$ddb\": {\"$binary\": \"W94UHeZb2ATmAAAAAAAAAAQAAAAEAAAABAAAAAAAAAAAAAAAAAAAAEgAAAAAAAAAowAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIoAAAAAAAAAlgAAAAAAAACjAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgEAAABBAwAAAGtleQEAAABhBAAAAGtleTMBAAAAgssAAAAAAAAA0QAAAAAAAADXAAAAAAAAANwAAAAAAAAA4gAAAAAAAAB2YWx1ZTF2YWx1ZTJ2YWx1ZXZhbHVlMwAAAAA=\", \"$type\": \"00\"}}}}}"));

        // PASSES BECAUSE string key2 is in the ddb (2nd world)
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key2\"}, {\"world\":{\"$binary\": \"b2s=\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$keyexists\": 0, \"$ddb\": {\"$binary\": \"W94UHeZb2ATmAAAAAAAAAAQAAAAEAAAABAAAAAAAAAAAAAAAAAAAAEgAAAAAAAAAowAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIoAAAAAAAAAlgAAAAAAAACjAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgEAAABBAwAAAGtleQEAAABhBAAAAGtleTMBAAAAgssAAAAAAAAA0QAAAAAAAADXAAAAAAAAANwAAAAAAAAA4gAAAAAAAAB2YWx1ZTF2YWx1ZTJ2YWx1ZXZhbHVlMwAAAAA=\", \"$type\": \"00\"}}}}}"));

        // PASSES BECAUSE bytes key2 is in the ddb (3rd world)
        BSON_ASSERT(compare_json("{\"hello\": [{\"world\":20},{\"world\":\"key02\"}, {\"world\":{\"$binary\": \"a2V5Mg==\", \"$type\": \"00\"}}]}",
                                 "{\"hello.world\":{\"$module\":{\"name\":\"disco\", \"config\":{\"$keyexists\": 0, \"$ddb\": {\"$binary\": \"W94UHeZb2ATmAAAAAAAAAAQAAAAEAAAABAAAAAAAAAAAAAAAAAAAAEgAAAAAAAAAowAAAAAAAABIAAAAAAAAAAAAAAAAAAAAcAAAAAAAAAB9AAAAAAAAAIoAAAAAAAAAlgAAAAAAAACjAAAAAAAAAAQAAABrZXkxAQAAACAEAAAAa2V5MgEAAABBAwAAAGtleQEAAABhBAAAAGtleTMBAAAAgssAAAAAAAAA0QAAAAAAAADXAAAAAAAAANwAAAAAAAAA4gAAAAAAAAB2YWx1ZTF2YWx1ZTJ2YWx1ZXZhbHVlMwAAAAA=\", \"$type\": \"00\"}}}}}"));






        rounds--;
    } while (rounds > 0);

    bsonsearch_shutdown();
    return 0;
}