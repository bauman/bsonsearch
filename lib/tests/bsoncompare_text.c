
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


int
main (int   argc,
      char *argv[])
{

/*
    char * lang = "english";
    char * enc  = NULL;
    const char * word = "actually";
    const char * stopchars = " ,.-";
    struct sb_stemmer * stemmer;
    stemmer = sb_stemmer_new(lang, enc);
    char * str ="- This is actually a really sample string that won't cause any problems.";
    char * pch;
    printf ("Splitting string \"%s\" into tokens:\n",str);
    pch = strtok (str,stopchars);
    while (pch != NULL)
    {
        printf ("%s\n",pch);
        const sb_symbol * stemmed = sb_stemmer_stem(stemmer, (const sb_symbol*)pch, strlen(pch));
        printf("%s\n", stemmed);
        pch = strtok (NULL, stopchars);
    }
    sb_stemmer_delete(stemmer);
*/


    BSON_ASSERT(compare_json("{\"a\": \"world how are you doing traveling this fine afternoon?\"}",
                             "{\"a\": {\"$text\": {\"$size\":{\"$gte\":1}, \"$search\": \"hello worldly travelers\", \"$language\":\"english\"}}}"));

    BSON_ASSERT(!compare_json("{\"a\": \"world how are you doing this fine afternoon?\"}",
                             "{\"a\": {\"$text\": {\"$size\":{\"$gt\":2}, \"$search\": \"hello worldly travelers\", \"$language\":\"english\"}}}"));

    BSON_ASSERT(compare_json("{\"a\": [{\"b\": \"nothing to see\"}, {\"b\":\"world how are you doing this fine afternoon?\"}]}",
                              "{\"a.b\": {\"$text\": {\"$size\":{\"$gt\":0}, \"$search\": \"hello worldly travelers\", \"$language\":\"english\"}}}"));
    return 0;
}