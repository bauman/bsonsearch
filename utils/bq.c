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
    bson_destroy(doc);
    bson_destroy(spec);
    return yes;
}


int main(int argc, char **argv)
{
    char *json = NULL;
    int c;
    size_t size;
    mongoc_matcher_t * matcher = NULL;
    bson_error_t error;
    bson_t      *doc;
    if (argc < 2){
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "%s '{valid json spec}'\n", argv[0]);
        exit(1);
    }
    bsonsearch_startup();
    while ((c = getopt(argc, argv, "s:")) != -1)
    {
        switch (c)
        {
            case 's':
                if (optarg == NULL || *optarg == '\0')
                {
                    (void) fprintf(stderr, "%s", "Illegal disposition spec\n");
                    exit(-1);
                }
                bson_t      *buf_spec = NULL;
                buf_spec = bson_new_from_json ((const uint8_t*)optarg, -1, NULL);
                if (buf_spec){
                    matcher = mongoc_matcher_new (buf_spec, NULL);
                    doc_destroy(buf_spec);
                    if (!matcher){
                        fprintf(stderr, "%s is not a valid matcher spec\n", optarg);
                        exit(2);
                    }
                }
                break;

        }
    }

    while(1){
        int64_t read = getline(&json, &size, stdin);
        if (read < 0) {
            break;
        } else {
            if (read >= 6){  // {"k":0} is the smallest doc this can handle
                doc = bson_new_from_json ((const uint8_t *) json, read, &error);
                if (doc) {
                    int yes = matcher_compare_doc(matcher, doc);
                    if (yes) {
                        printf("%s", json);
                    }
                    doc_destroy(doc);
                    doc = NULL;
                }
            }
        }
    }
    if(json){
        free(json);
    }
    matcher_destroy(matcher);
    bsonsearch_shutdown();
    return 0;
}
