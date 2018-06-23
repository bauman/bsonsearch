/*
 * Copyright (c) 2016 Bauman
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifdef WITH_TEXT

#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"
#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-text.h"
#include "mongoc-bson-descendants.h"
#ifdef WITH_ASPELL
#include <aspell.h>
#endif /*WITH_ASPELL*/

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_parse_text --
 *
 *       This function takes a bson.Binary type object which must contain
 *          the raw compiled
 *
 *        or a "expanded form" with the pointer places as follows:
 *          { "$text" : { "source":Binary(<compiled data>),
 *                     ^  "timeout":<int32>,  //seconds before yara aborts
 *                     ^  "fastmode":<bool>} //only look at strings once
 *                     ^
 *        -------------^
 *
 * Returns:
 *       NULL if error
 *       or
 *       <mongoc_matcher_op_t> which can be used to call either of:
 *                              _mongoc_matcher_op_yara_match
 *                              _mongoc_matcher_op_yara_compare
 *
 *        (variables are populated in the op->compare section only)
 *        with the exception of OPCODE_YARA placed in main op.
 *        caller responsible to check OPCODE_YARA before calling
 *        the afforementioned functions.
 *
 * Notes:
 *      None
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_matcher_op_t *
_mongoc_matcher_text_new (const char   *path,   /* IN */
                            bson_iter_t   *iter)   /* OUT */
{
    mongoc_matcher_op_t *op = NULL;
    BSON_ASSERT (iter);

    bson_iter_t child;
    if (bson_iter_recurse(iter, &child)) {
        op = _mongoc_matcher_parse_text_loop(path, &child);
    }
    if (op) {
        op->text.path = bson_strdup(path);
    }
    return op;
}
static bool
mongoc_verify_dict_name(const char * dict_name)
{
    return ((strcmp(dict_name, "en_US")==0) ||
            (strcmp(dict_name, "en_GB")==0)

    );
}

static bool
mongoc_verify_stem_algo(const char * stem_algo)
{
    return ( (strcmp(stem_algo, "english")==0) ||
            (strcmp(stem_algo, "porter")==0) ||
             (strcmp(stem_algo, "arabic")==0) ||
            (strcmp(stem_algo, "danish")==0) ||
            (strcmp(stem_algo, "dutch")==0) ||
            (strcmp(stem_algo, "finnish")==0) ||
            (strcmp(stem_algo, "french")==0) ||
            (strcmp(stem_algo, "german")==0) ||
            (strcmp(stem_algo, "german2")==0) ||
            (strcmp(stem_algo, "hungarian")==0) ||
            (strcmp(stem_algo, "italian")==0) ||
            (strcmp(stem_algo, "norwegian")==0) ||
            (strcmp(stem_algo, "portuguese")==0) ||
            (strcmp(stem_algo, "romanian")==0) ||
            (strcmp(stem_algo, "spanish")==0) ||
            (strcmp(stem_algo, "sweedish")==0) ||
            (strcmp(stem_algo, "tamil")==0) ||
            (strcmp(stem_algo, "turkish")==0));
}
static void
_populate_mongoc_matcher_populate_wordlist(mongoc_matcher_op_t * op,
                                           char                *full_string_maloc){

#ifdef WITH_STEMMER
    mongoc_matcher_op_str_hashtable_t *s;
    char * pch;
    pch = strtok (full_string_maloc, op->text.stop_word);
    while (pch != NULL)
    {
        const sb_symbol * matcher_hash_key_cst = sb_stemmer_stem(op->text.stemmer, (const sb_symbol*)pch, strlen(pch));
        char * matcher_hash_key = bson_strdup((const char * )matcher_hash_key_cst);
        s = ( mongoc_matcher_op_str_hashtable_t *)malloc(sizeof( mongoc_matcher_op_str_hashtable_t ));
        s->matcher_hash_key = matcher_hash_key;
        HASH_ADD_KEYPTR(hh, op->text.wordlist, s->matcher_hash_key, strlen(s->matcher_hash_key), s);
        pch = strtok (NULL, op->text.stop_word);
    }
    free(pch);
#endif /*WITH_STEMMER*/
    return;
}
static bool
_mongoc_matcher_text_parse_opcode(mongoc_matcher_op_t *op,
                                  const char          *opcode,
                                  uint32_t             opcode_len)
{
    bool result = false;
    if (strncmp(opcode, "$wordcount", (size_t)opcode_len)==0){
        op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_COUNT;
    }
#ifdef WITH_ASPELL
    else if (strncmp(opcode, "$spellingerror", (size_t)opcode_len)==0) {
        op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_SPELLING_INCORRECT;
    } else if (strncmp(opcode, "$spellingcorrect", (size_t)opcode_len)==0) {
        op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_SPELLING_CORRECT;
    } else if (strncmp(opcode, "$spellingpercentage", (size_t)opcode_len)==0) {
        op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_SPELLING_PERCENTAGE_CORRECT;
    }
#endif /*WITH_ASPELL*/
    return result;
}
mongoc_matcher_op_t *
_mongoc_matcher_parse_text_loop (const char              * path,
                                 bson_iter_t             *iter)
{
    mongoc_matcher_op_t *op = NULL;
    char * enc = NULL, * search = NULL;
    uint32_t search_len = 0;
    op = (mongoc_matcher_op_t *) bson_malloc0(sizeof *op);
    op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_COUNT;
    op->text.case_sensitive = DEFAULT_TEXT_CASE_SENSITIVE;
#ifdef WITH_STEMMER
    op->text.language = bson_strdup(DEFAULT_TEXT_STEMMING_LANG);
#endif /*WITH_SEMMER*/
    op->text.stop_word = bson_strdup(DEFAULT_TEXT_STOPCHARS);
    while (bson_iter_next(iter)) {
        const char * key = bson_iter_key(iter);
        if (strcmp(key, "$size")==0) {
            op->text.size_container = _mongoc_matcher_op_size_new(MONGOC_MATCHER_OPCODE_SIZE, path, iter);
        }
#ifdef WITH_STEMMER
        else if (strcmp(key, "$search")==0){
            if (BSON_ITER_HOLDS_UTF8(iter)){
                const char * csearch = bson_iter_utf8(iter, &search_len);
                if (search_len > 0){
                    op->base.opcode = MONGOC_MATCHER_OPCODE_TEXT_COUNT_MATCHES;
                    search = bson_strdup(csearch);
                }
            }
        }
        else if (strcmp(key, "$language")==0){
            if (BSON_ITER_HOLDS_UTF8(iter)){
                uint32_t lang_len = 0;
                const char *  langc  = bson_iter_utf8(iter, &lang_len);
                if (lang_len > 0 && mongoc_verify_stem_algo(langc)){
                    bson_free(op->text.language); //free the default
                    op->text.language = bson_strdup(langc);
                }
            }
        }
#endif /*WITH_STEMMER*/
#ifdef WITH_ASPELL
        else if (strcmp(key, "$dictionary")==0){
            if (BSON_ITER_HOLDS_UTF8(iter)){
                uint32_t dict_len = 0;
                const char *  dict  = bson_iter_utf8(iter, &dict_len);
                if (dict_len > 0 && mongoc_verify_dict_name(dict)){
                    op->text.dictionary = bson_strdup(dict);
                    AspellConfig * spell_config = new_aspell_config();
                    aspell_config_replace(spell_config, "lang", dict);
                    AspellCanHaveError * possible_err = new_aspell_speller(spell_config);
                    delete_aspell_config(spell_config);
                    if (aspell_error(possible_err) != 0) {
                        delete_aspell_can_have_error(possible_err);
                    } else {
                        op->text.spell_checker = to_aspell_speller(possible_err);
                    }

                }
            }
        }
#endif /*WITH_ASPELL*/
        else if (strcmp(key, "$stopWord")==0){
            if (BSON_ITER_HOLDS_UTF8(iter)){
                uint32_t swc_len = 0;
                const char *  swc  = bson_iter_utf8(iter, &swc_len);
                if (swc_len > 0){
                    bson_free(op->text.stop_word); //free the default
                    op->text.stop_word = bson_strdup(swc);
                }
            }
        } else if (strcmp(key, "$opcode")==0){
            if (BSON_ITER_HOLDS_UTF8(iter)){
                uint32_t opcode_len;
                const char *  opcode  = bson_iter_utf8(iter, &opcode_len);
                _mongoc_matcher_text_parse_opcode(op, opcode, opcode_len);
            }
        }
    }

    //need to build the stemmer after all the configs are found
#ifdef WITH_STEMMER
    if (search){
        op->text.stemmer = sb_stemmer_new(op->text.language, enc);
        _populate_mongoc_matcher_populate_wordlist(op, search);
        bson_free(search);
    }
    if (op->base.opcode == MONGOC_MATCHER_OPCODE_TEXT_COUNT_MATCHES &&
            !op->text.stemmer){
        //out of memory or invalid $search
        _mongoc_matcher_op_destroy(op); // config not right, nuke and null
        op = NULL;
    }
#endif /*WITH_STEMMER*/
    return op;
}

bool
_mongoc_matcher_op_text_match  (mongoc_matcher_op_text_t     *compare,
                                const bson_t                 *bson)
{
    bson_iter_t tmp;
    bson_iter_t iter;
    bool found_one = false;
    int checked = 0, skip=0;
    BSON_ASSERT (compare);
    BSON_ASSERT (bson);
    if (strchr (compare->path, '.')) {
        if (!bson_iter_init (&tmp, bson) ||
            !bson_iter_find_descendant (&tmp, compare->path, &iter)) { //try this way first
            while (!found_one &&
                   bson_iter_init (&tmp, bson) &&
                   bson_iter_find_descendants (&tmp, compare->path, &skip, &iter)){
                found_one |= _mongoc_matcher_op_text_match_iter(compare, &iter);
                skip = ++checked;
            }
            return ((checked>0) && found_one);
        }
    } else if (!bson_iter_init_find (&iter, bson, compare->path)) {
        return false;
    }
    return _mongoc_matcher_op_text_match_iter(compare, &iter);

}
#ifdef WITH_ASPELL
static uint32_t
_mongoc_matcher_op_text_spellcheck(mongoc_matcher_op_text_t *compare,
                                   char * word_to_check)
{
    bool correct ;
    uint32_t  matches = 0;
    correct = (bool)aspell_speller_check(compare->spell_checker, word_to_check, -1);
    switch (compare->base.opcode){
        case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_PERCENTAGE_CORRECT:
        case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_CORRECT:
            matches += (uint32_t)correct;
            break;
        case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_INCORRECT:
            matches += (uint32_t)(!correct);
            break;
        default:
            break;
    }
    return matches;
}
#endif /*WITH_ASPELL*/

bool
_mongoc_matcher_op_text_match_iter (mongoc_matcher_op_text_t *compare, /* IN */
                                    bson_iter_t                 *iter)    /* IN */
{
    mongoc_matcher_op_str_hashtable_t *check=NULL;
    bson_iter_t right_array;
    if BSON_ITER_HOLDS_UTF8(iter){
        uint32_t str_len = 0, matches = 0, total=0;
        const char * source_data = bson_iter_utf8(iter, &str_len);
        char * source_data_mallocd = bson_strdup(source_data);
        char * pch;
        pch = strtok (source_data_mallocd, compare->stop_word);
        while (pch != NULL){
            total++;
            switch (compare->base.opcode)
            {
                case MONGOC_MATCHER_OPCODE_TEXT_COUNT:
                {
                    matches++;
                    break;
                }
#ifdef WITH_STEMMER
                case MONGOC_MATCHER_OPCODE_TEXT_COUNT_MATCHES:
                {
                    const sb_symbol * matcher_hash_key = sb_stemmer_stem(compare->stemmer,
                                                                         (const sb_symbol*)pch,
                                                                         strlen(pch));
                    HASH_FIND_STR(compare->wordlist, (const char *)matcher_hash_key, check);
                    if (check != NULL){
                        matches++;
                    }
                    break;
                }
#endif /*WITH_STEMMER*/
#ifdef WITH_ASPELL
                case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_PERCENTAGE_CORRECT:
                case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_INCORRECT:
                case MONGOC_MATCHER_OPCODE_TEXT_SPELLING_CORRECT:
                {
                    matches += _mongoc_matcher_op_text_spellcheck(compare, pch);
                    break;
                }
#endif /*WITH_ASPELL*/
                default:
                {
                    break;
                }
            }
            pch = strtok (NULL, compare->stop_word);
        }
        bson_free(pch);
        bson_free(source_data_mallocd);
#ifdef WITH_ASPELL
        if (compare->base.opcode == MONGOC_MATCHER_OPCODE_TEXT_SPELLING_PERCENTAGE_CORRECT){
            matches = (uint32_t) (100*((double)matches/(double)total));
        }
#endif /*WITH_ASPELL*/
        return _mongoc_matcher_op_length_match_value(&compare->size_container->size, matches);

    } else if (BSON_ITER_HOLDS_ARRAY (iter) && bson_iter_recurse(iter, &right_array)) {
        while (bson_iter_next (&right_array)) {
            if (_mongoc_matcher_op_text_match_iter(compare, &right_array)) {
                return true;
            }
        }
    }
    return false;
}

#endif /* WITH_TEXT */