

#ifdef WITH_YARA
#include <yara.h>

#include "mongoc-matcher.h"
#include "mongoc-matcher-private.h"

#include "mongoc-matcher-op-private.h"
#include "mongoc-matcher-op-yara.h"


int yara_callback(
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
            //                                   "fast_mode":<bool>}}
            //          loop the keys, hold onto timeout/fastmode locally, pass binary to self recursive
            //          allow the recursive call to attempt to malloc op.
            //          if op is malloc'd, put the timeout/fastmode vars into op and pass it along.
            break;
        }
        case BSON_TYPE_BINARY:
        {
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
            if ((uint32_t) size + binary_user_data->cursor_pos > binary_user_data->binary_len) {
                size = (size_t)(binary_user_data->binary_len - binary_user_data->cursor_pos);
            }
            memcpy((char *) ptr + i * size,
                   binary_user_data->binary + binary_user_data->cursor_pos + i * size,
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
