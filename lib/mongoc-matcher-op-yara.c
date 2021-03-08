

#ifdef WITH_YARA
#include <yara.h>

#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"

#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-yara.h"

/*
 *--------------------------------------------------------------------------
 *
 * yara_callback --
 *
 *       use this function as a basic yara callback that aborts additional
 *       scanning after the first rule fires.
 *
 *       This is a good function to use if you don't care WHICH rule fires
 *       as much as you care that any of the rules fired.
 *
 * Returns:
 *       <int> that is wither CONTINUE or ABORT for yara to act upon.
 *
 * Notes:
 *      Additional information about yara C api can be found
 *      https://yara.readthedocs.io/en/latest/capi.html
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
int yara_callback(
        YR_SCAN_CONTEXT* context,
        int message,
        void* message_data,
        void* user_data)
{
    int result = CALLBACK_CONTINUE;
    mongoc_matcher_op_yara_callback_data *callback_user_data = (mongoc_matcher_op_yara_callback_data *)user_data;

    switch (message){
        case CALLBACK_MSG_RULE_MATCHING:
        {
            callback_user_data->matches += 1;
            return CALLBACK_ABORT;
        }
        case CALLBACK_MSG_RULE_NOT_MATCHING:
        {
            return CALLBACK_CONTINUE;
        }
        case CALLBACK_MSG_SCAN_FINISHED:
        {
            return CALLBACK_CONTINUE;
        }
        case CALLBACK_MSG_IMPORT_MODULE:
        {
            return CALLBACK_CONTINUE;
        }
    }
    return result;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_yara_match --
 *
 *       Main function responsible to translate the bsoncompare data
 *          into a file-like-object (flo) that is yara consumable
 *       and yara's response
 *          into a bsoncompare true/false.
 *
 *       This function hands the flo off to the helper function to perform
 *          the compare.
 * Returns:
 *       <bool> if ANY rule fires.
 *
 * Notes:
 *      None
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
bool
_mongoc_matcher_op_yara_match (mongoc_matcher_op_compare_t *compare, /* IN */
                               bson_iter_t                 *iter)    /* IN */
{
    bool result = false;
    mongoc_matcher_op_binary_flo *bin_flo;
    bin_flo = (mongoc_matcher_op_binary_flo *)bson_malloc (sizeof *bin_flo);
    bin_flo->cursor_pos = 0;

    switch (bson_iter_type ((iter))) {
        case BSON_TYPE_BINARY:
        {
            bson_subtype_t subtype;
            bson_iter_binary(iter, &subtype, &bin_flo->binary_len, &bin_flo->binary);
            result = _mongoc_matcher_op_yara_compare(compare, bin_flo);
            break;
        }
        case BSON_TYPE_UTF8:
        {
            bin_flo->binary = (uint8_t *)bson_iter_utf8(iter, &bin_flo->binary_len);
            result = _mongoc_matcher_op_yara_compare(compare, bin_flo);
            break;
        }
        case BSON_TYPE_ARRAY:
        {
            bson_iter_t right_array;
            if (bson_iter_recurse(iter, &right_array))
            {

                while (bson_iter_next(&right_array)) {
                    if (result){
                        break; //don't keep scanning if we're going to return true anyway.
                    }
                    result |= _mongoc_matcher_op_yara_match(compare, &right_array);
                }
            }
            break;
        }
        default:
            break;
    }
    bson_free(bin_flo);
    return result;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_yara_compare --
 *
 *       Helper function responsible to manage the internals of the
 *          call to libyara.
 *
 *       This is worth a separate function as it may provide utility
 *          outside bsoncompare itself.  Developers could import this
 *          function and generate the bin_flo object rather than
 *          creating the bson_iter_t to use the yara capi.
 *
 *
 * Returns:
 *       <bool> if ANY rule fires.
 *
 * Notes:
 *      None
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
bool
_mongoc_matcher_op_yara_compare(mongoc_matcher_op_compare_t *compare,
                                mongoc_matcher_op_binary_flo *bin_flo)
{
    int error;
    bool result = false;
    mongoc_matcher_op_yara_callback_data *user_data;
    user_data = (mongoc_matcher_op_yara_callback_data *)bson_malloc (sizeof *user_data);
    user_data->matches=0;
    user_data->next_hit = NULL;
    error = yr_rules_scan_mem(
            compare->rules,
            (uint8_t *)bin_flo->binary, //TODO: removing const relies on the fact that yara does NOT modify memory
                                        //      this will segfault if yr_rules_scan_mem changes behavior.
            (size_t)bin_flo->binary_len,
            compare->fast_mode,
            yara_callback,
            user_data,
            compare->timout);

    if (error == ERROR_SUCCESS && user_data->matches > 0){
        result = true;
    }
    bson_free(user_data);
    return result;
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_yara_new --
 *
 *       This function takes a bson.Binary type object which must contain
 *          the raw compiled
 *
 *       There is no input validation on this function, but if magic string
 *          on the object passed as binary should be "YARA".
 *
 *       value may be "standard form" with the pointer placed as follows:
 *          { "$yara" : Binary(<compiled data>) }
 *                     ^
 *          -----------^
 *        or a "expanded form" with the pointer places as follows:
 *          { "$yara" : { "source":Binary(<compiled data>),
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
_mongoc_matcher_op_yara_new     ( const char              *path,   /* IN */
                                 bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_t *op = NULL;
    int timeout = MONGOC_MATCHER_YARA_TIMEOUT_DEFAULT;
    bool fast_mode = MONGOC_MATCHER_YARA_FAST_MODE_DEFAULT;
    switch (bson_iter_type ((child))) {
        case BSON_TYPE_DOCUMENT:
        {
            //TODO: allow non-default config:
            //          user specified {"$yara":{"source":<Binary>/<utf8>, binary=load(), utf8=compile(),
            //                                   "filename":<utf8>, compile the sig written to this file
            //                                   "timeout":<int>,
            //                                   "fastmode":<bool>}}
            bson_iter_t yara_config_iter;
            if (bson_iter_recurse (child, &yara_config_iter)) {
                while (bson_iter_next (&yara_config_iter)) {
                    const char * key = bson_iter_key (&yara_config_iter);
                    switch (bson_iter_type ((&yara_config_iter))) {
                        case BSON_TYPE_BINARY:
                        {
                            if (strcmp(key, "source")==0){
                                op = _mongoc_matcher_op_yara_new(path, &yara_config_iter);
                            }
                            break;
                        }
                        case BSON_TYPE_BOOL:
                        {
                            if (strcmp(key, "fastmode")==0 ) {
                                fast_mode = bson_iter_bool(&yara_config_iter);
                            }
                            break;
                        }
                        case BSON_TYPE_INT32:
                        {
                            if (strcmp(key, "timeout")==0 ) {
                                timeout = bson_iter_int32(&yara_config_iter);
                            }
                            break;
                        }
                        case BSON_TYPE_UTF8:
                        {
                            //this should be eaten by the compiler.
                            if (strcmp(key, "source")==0 ) {
                                //call yara.compile on the source string
                                op = _mongoc_matcher_op_yara_new_op_from_string(path, &yara_config_iter);
                            } else if ( strcmp(key, "filename")==0 ) {
#ifdef ALLOW_FILESYSTEM
                                //call yara.compile on the source string file handle

#endif /* ALLOW_FILESYSTEM */
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            break;
        }
        case BSON_TYPE_BINARY:
        {
            op = _mongoc_matcher_op_yara_new_op_from_bin(path, child);
            break;
        }
        default:
            break;
    }
    if (op != NULL){
        op->compare.fast_mode = fast_mode;
        op->compare.timout = timeout;
    }

    return op; //is NULL if the spec isn't correct.  Will cause segfault later.
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_yara_new_op_from_bin --
 *
 *       responsible for taking a bson_iter_t which MUST be pointing at a
 *       Binary object.
 *
 *       THIS IS THE ONLY FUNCTION THAT SHOULD EVER ALLOCATE A FLO
 *       TO CALL yr_rules_load_stream
 *
 *       yara's capi only loads streams via a file like object.
 *
 *       this function is responsible for generating the flo,
 *       allocating the yara stream, and freeing the file like object.
 *
 * Returns:
 *        mongoc_matcher_op_t
 *        or NULL
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
_mongoc_matcher_op_yara_new_op_from_bin     ( const char              *path,   /* IN */
                                              bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_t *op = NULL;
    op = (mongoc_matcher_op_t *)bson_malloc (sizeof *op);
    op->compare.base.opcode = MONGOC_MATCHER_OPCODE_YARA;
    op->compare.path = bson_strdup (path);
    mongoc_matcher_op_binary_flo *bin_flo;
    bin_flo = (mongoc_matcher_op_binary_flo *)bson_malloc (sizeof *bin_flo);
    bin_flo->cursor_pos = 0;
    bson_subtype_t subtype;
    bson_iter_binary(child, &subtype, &bin_flo->binary_len, &bin_flo->binary);

    YR_STREAM stream;
    stream.user_data = bin_flo;
    stream.read = binary_read;

    int error;
    error = yr_rules_load_stream(&stream, &op->compare.rules);
    bson_free(bin_flo);
    if (error > 0 )
    {
        return NULL; //Cause a segfault- easy to trace error.  Didn't clean up op malloc
    }
    return op;
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_matcher_op_yara_new_op_from_string --
 *
 *       responsible for taking a bson_iter_t which MUST be pointing at a
 *       utf8 string representing a plain text yara rule fset.
 *
 *       THIS IS THE ONLY FUNCTION THAT SHOULD EVER ALLOCATE A FLO
 *       TO CALL yr_compiler_add_string
 *
 * Returns:
 *        mongoc_matcher_op_t (success)
 *        or NULL             (failure)
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
_mongoc_matcher_op_yara_new_op_from_string     ( const char              *path,   /* IN */
                                                 bson_iter_t             *child)   /* IN */
{
    mongoc_matcher_op_yara_compiler_data cr;
    cr.errors = 0;
    cr.warnings  = 0;
    const char * ns = NULL;
    int rule_errors = 0;
    YR_COMPILER* compiler = NULL;

    mongoc_matcher_op_t *op = NULL;
    op = (mongoc_matcher_op_t *)bson_malloc (sizeof *op);
    op->compare.base.opcode = MONGOC_MATCHER_OPCODE_YARA;
    op->compare.path = bson_strdup (path);
    if (ERROR_SUCCESS == yr_compiler_create(&compiler)){
        uint32_t rule_len = 0;
        const char *  rule_string  = bson_iter_utf8(child, &rule_len);
        cr.errors = yr_compiler_add_string(compiler, rule_string, ns);
        if (cr.errors == 0){
            rule_errors = yr_compiler_get_rules(compiler, &op->compare.rules);
        }
    }
    if (compiler != NULL) {
        yr_compiler_destroy(compiler);
    }
    if (rule_errors > 0){
        _mongoc_matcher_op_destroy(op);
        return NULL;
    }
    return op;
}

/*
 *--------------------------------------------------------------------------
 *
 * binary_read --
 *
 *       yara capi dedicated function.  This intentionally does NOT have
 *       bsoncompare types in here.
 *
 *       yara issues this callback with a mongoc_matcher_op_binary_flo
 *          object that was issued by _mongoc_matcher_op_yara_new_op_from_bin
 *
 *       Use extreme caution and avoid using this function outside of
 *          _mongoc_matcher_op_yara_new_op_from_bin
 *
 *       This function will segfault if the void user_data pointer
 *          points to other than a mongoc_matcher_op_binary_flo obj
 *
 *       This function attempts to match a standard file.read() function
 *
 *       It's correct, and it doesnt leak.  Other than that
 *          I'm a little shicked it actually worked, so I'm not touching it
 *          from here on out
 *
 *
 *
 * Returns:
 *        <size_t> number of bytes that this function handed back to yara.
 *
 *        if return value does not equal input size+count:
 *              yara will stop reading from the buffer
 *
 *
 * Notes:
 *        ptr value is terrifying.  I don't understand what is happening to
 *          it between callbacks.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
size_t
binary_read(
        void* ptr,
        size_t size,
        size_t count,
        void* user_data) {
    size_t i;
    mongoc_matcher_op_binary_flo *binary_user_data = (mongoc_matcher_op_binary_flo *)user_data;
    for (i = 0; i < count; i++)
    {
        if (binary_user_data->cursor_pos < binary_user_data->binary_len)
        {
            if ((int64_t)size + binary_user_data->cursor_pos > binary_user_data->binary_len) {
                size = (size_t) (binary_user_data->binary_len - binary_user_data->cursor_pos);
            }
            memcpy((char *) ptr + i * size,
                   binary_user_data->binary + binary_user_data->cursor_pos,
                   size);
            binary_user_data->cursor_pos += (uint32_t) size;
        }
        else
        {
            return i;
        }
    }
    return count;
}

#endif //WITH_YARA
