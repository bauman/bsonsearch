
#include <stdio.h>
#include <bsoncompare.h>
#include <aspell.h>

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

    AspellConfig * spell_config = new_aspell_config();
    aspell_config_replace(spell_config, "lang", "en_US");

    const char * word = "gret";
    int size = -1;
    AspellCanHaveError * possible_err = new_aspell_speller(spell_config);
    delete_aspell_config(spell_config);
    if (aspell_error(possible_err) != 0) {
        printf("Error: %s\n",aspell_error_message(possible_err));
        delete_aspell_can_have_error(possible_err);
        return 2;
    }
    AspellSpeller * spell_checker = 0;
    if (!aspell_error_number(possible_err)){
        spell_checker = to_aspell_speller(possible_err);
        int correct = aspell_speller_check(spell_checker, word, size);
        if (correct){
            printf("CORRECT: %d\n", correct);
        }

    }
    delete_aspell_speller(spell_checker);
    /*
    BSON_ASSERT(compare_json("{\"a\": \"hello world how are you doing traveling this fine afternoon?\"}",
                             "{\"a\": {\"$text\": {\"$size\":{\"$eq\":1}, \"$opcode\":\"$spellingerror\", \"$dictionary\":\"en_US\"}}}"));
    */
    BSON_ASSERT(compare_json("{\"a\": \"hello werld\"}",
                             "{\"a\": {\"$text\": {\"$size\":{\"$gte\":50}, \"$opcode\":\"$spellingpercentage\", \"$dictionary\":\"en_US\"}}}"));

    BSON_ASSERT(compare_json("{\"a\": \"hello werld how are you doing traveling this fine afternoon?\"}",
                             "{\"a\": {\"$text\": {\"$size\":{\"$gte\":1}, \"$opcode\":\"$spellingcorrect\", \"$dictionary\":\"en_US\"}}}"));

    return 0;
}