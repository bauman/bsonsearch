#ifdef WITH_MODULES
#ifdef WITH_DISCO

#include <bson/bson.h>
#include "matcher-module-disco.h"

/* The below code is from the discoproject/DiscoDB project
 * Licensed 3-Clause BSD
 * https://github.com/bauman/discodb/blob/py3/DISCODB_LICENSING.txt
 * forked from
 * https://github.com/discoproject/discodb/blob/master/LICENSE
 */
#define FEAT(x) (long long unsigned int)feat[x]
static const char yes[] = "true";
static const char no[] = "false";
const char *boolstr(int boolean) { return boolean ? yes: no; }
static void debug_print_info(struct ddb *db)
{
    ddb_features_t feat;
    ddb_features(db, feat);
    printf("Total size:              %llu bytes\n", FEAT(DDB_TOTAL_SIZE));
    printf("Items size:              %llu bytes\n", FEAT(DDB_ITEMS_SIZE));
    printf("Values size:             %llu bytes\n", FEAT(DDB_VALUES_SIZE));
    printf("Number of keys:          %llu\n", FEAT(DDB_NUM_KEYS));
    printf("Number of items:         %llu\n", FEAT(DDB_NUM_VALUES));
    printf("Number of unique values: %llu\n", FEAT(DDB_NUM_UNIQUE_VALUES));
    printf("Compressed?              %s\n", boolstr(feat[DDB_IS_COMPRESSED]));
    printf("Hashed?                  %s\n", boolstr(feat[DDB_IS_HASHED]));
    printf("Multiset?                %s\n", boolstr(feat[DDB_IS_MULTISET]));
}

static struct ddb *open_discodb(const char *file)
{
    struct ddb *db;
    int fd;

    if (!(db = ddb_new())){
        fprintf(stderr, "Couldn't initialize discodb: Out of memory\n");
        exit(1);
    }
    if ((fd = open(file, O_RDONLY)) == -1){
        fprintf(stderr, "Couldn't open discodb %s\n", file);
        exit(1);
    }
    if (ddb_load(db, fd)){
        const char *err;
        ddb_error(db, &err);
        fprintf(stderr, "Invalid discodb in %s: %s\n", file, err);
        exit(1);
    }
    return db;
}

static void debug_print_cursor(struct ddb *db, struct ddb_cursor *cur, struct ddb_entry *es)
{
    if (!cur){
        const char *err;
        ddb_error(db, &err);
        fprintf(stderr, "Query failed: %s\n", err);
        exit(1);
    }

    if (ddb_notfound(cur)){
        fprintf(stderr, "Not found\n");
        exit(1);
    }
    int errno, i = 0;
    const struct ddb_entry *e;
    while ((e = ddb_next(cur, &errno))){
        printf("Key: %.*s -- Value:%.*s\n", es->length, es->data, e->length, e->data);
        ++i;
    }
    if (errno){
        fprintf(stderr, "Cursor failed: out of memory\n");
        exit(1);
    }
    //ddb_free_cursor(cur); // caller made the cursor, caller should free the cursor
}

/* The above code is from the discoproject/DiscoDB project
 * Licensed 3-Clause BSD
 * https://github.com/bauman/discodb/blob/py3/DISCODB_LICENSING.txt
 * forked from
 * https://github.com/discoproject/discodb/blob/master/LICENSE
 */

static bool
matcher_module_process_term(matcher_container_disco_holder_t* md, bson_iter_t *config,  uint32_t clause_number, uint32_t term_number){
    bool result = true;
    bson_iter_t child;
    bson_type_t ttt = bson_iter_type(config);
    if (BSON_ITER_HOLDS_DOCUMENT(config)){
        bson_iter_recurse(config, &child);
        while (bson_iter_next(&child)){
            const char * key = bson_iter_key(&child);
            int j = strlen(key);
            if ( strcmp(key, "entry") == 0
                    && BSON_ITER_HOLDS_UTF8(&child)){
                uint32_t  entry_len;
                md->clauses[clause_number].terms[term_number].key.data = bson_iter_utf8(&child, &entry_len);
                md->clauses[clause_number].terms[term_number].key.length = entry_len;
            } else if (strcmp(key, "nnot") == 0
                && BSON_ITER_HOLDS_BOOL(&child)) {
                md->clauses[clause_number].terms[term_number].nnot = (int) bson_iter_bool(&child);
            }
        }
    }
    return result;
}

static bool
matcher_module_process_clause(matcher_container_disco_holder_t* md, bson_iter_t *config, uint32_t clause_number){
    bool result = true;
    bson_iter_t child, term_doc;
    while (bson_iter_next(config)){
        const char * key = bson_iter_key(config);
        int j = strlen(key);
        if (strcmp(key, "num_terms") == 0
            && BSON_ITER_HOLDS_INT32(config)){
            md->clauses[clause_number].num_terms = (uint32_t) bson_iter_int32(config);
        } else if (strcmp(key, "terms") == 0
                   && md->clauses[clause_number].num_terms > 0
                   && BSON_ITER_HOLDS_ARRAY(config)){
            md->clauses[clause_number].terms = (struct ddb_query_term *)calloc(md->clauses[clause_number].num_terms, sizeof(struct ddb_query_term));

            bson_iter_recurse(config, &child);
            uint32_t term_number = 0;
            while (bson_iter_next(&child)
                    && term_number < md->clauses[clause_number].num_terms){
                matcher_module_process_term(md, &child, clause_number, term_number);
                term_number++;
            }
        }
    }

    return result;
}


static bool
matcher_module_disco_loop_clauses(matcher_container_disco_holder_t* md, bson_iter_t *config){
    bool result = true;
    bson_iter_t clause_iter, child;
    uint32_t clause_number = 0;
    if (BSON_ITER_HOLDS_ARRAY(config)){
        bson_iter_recurse(config,  &clause_iter);
        while (bson_iter_next(&clause_iter)){
            if (BSON_ITER_HOLDS_DOCUMENT(&clause_iter)
                    && clause_number < md->clauses_len) {
                bson_iter_recurse(&clause_iter, &child);
                matcher_module_process_clause(md, &child, clause_number);
            }
            clause_number++;
        }
    }
    return result;
}




static bool
matcher_module_disco_unload_clauses(matcher_container_disco_holder_t* md, bson_iter_t *config){
    bool result = true;
    bson_iter_t child;

    if (BSON_ITER_HOLDS_DOCUMENT(config)){
        int a = 0;
        bson_iter_recurse(config,  &child);
        while (bson_iter_next(&child)){
            const char * key = bson_iter_key(&child);
            if (strcmp(key, "num_clauses") == 0
                    && BSON_ITER_HOLDS_INT32(&child)){
                md->clauses_len = bson_iter_int32(&child);
            } else if (strcmp(key, "clauses") == 0
                    && md->clauses_len > 0
                    && BSON_ITER_HOLDS_ARRAY(&child)){
                md->clauses = (struct ddb_query_clause *)calloc(md->clauses_len, sizeof(struct ddb_query_clause));
                matcher_module_disco_loop_clauses(md, &child);
            } else if (strcmp(key, "precache") == 0
                       && BSON_ITER_HOLDS_BOOL(&child)){
                md->precache = bson_iter_bool(&child);
                if (md->precache && md->db && md->clauses && md->clauses_len){
                    struct ddb_cons * cons = ddb_cons_new();
                    struct ddb_cursor * cur = ddb_query(md->db, md->clauses, md->clauses_len);
                    struct ddb_entry xentry = {.data="-", .length=1};
                    const struct ddb_entry * ventry;
                    int ddb_errno;
                    while ((ventry = ddb_next(cur, &ddb_errno))){
                        ddb_cons_add(cons, ventry, &xentry);
                    }
                    ddb_free_cursor(cur);
                    md->precache_data = ddb_cons_finalize(cons, &md->precache_data_len, 0);
                    ddb_cons_free(cons);
                    ddb_free(md->db);
                    md->db = ddb_new();
                    ddb_loads(md->db, md->precache_data, md->precache_data_len);
                }
            }
        }
    }
    return result;
}

bool
matcher_module_disco_startup(mongoc_matcher_op_t * op, bson_iter_t * config){
    bool result = false;
    matcher_container_disco_holder_t *md;
    md = (matcher_container_disco_holder_t*) bson_malloc0(sizeof *md);
    md->compare = MATCHER_MODULE_DISCO_UNDEFINED;
    while (bson_iter_next(config)) {
        const char *key = bson_iter_key(config);
        if (strcmp(key, "$keyexists") == 0) {
            md->compare = MATCHER_MODULE_DISCO_KEY_EXISTS;
            md->state = md->state | MODULE_DISCO_HAS_ACTION;
        } else if (strcmp(key, "$valueis") == 0) {
            if (BSON_ITER_HOLDS_UTF8(config)){
                md->search = (const uint8_t *) bson_iter_utf8(config, &md->search_len);
                md->compare = MATCHER_MODULE_DISCO_VALUE_IS;
                md->state = md->state | MODULE_DISCO_HAS_ACTION;
            }
        } else if (strcmp(key, "$valueonly") == 0) {
            if (BSON_ITER_HOLDS_UTF8(config)){
                md->search = (const uint8_t *) bson_iter_utf8(config, &md->search_len);
                md->compare = MATCHER_MODULE_DISCO_VALUE_ONLY;
                md->state = md->state | MODULE_DISCO_HAS_ACTION;
            }
        } else if (strcmp(key, "$Q") == 0) {
            if (BSON_ITER_HOLDS_ARRAY(config) || BSON_ITER_HOLDS_DOCUMENT(config)){
                md->compare = MATCHER_MODULE_DISCO_VALUE_MATCHES_Q;
                matcher_module_disco_unload_clauses(md, config);
                md->state = md->state | MODULE_DISCO_HAS_ACTION;
            } else if (BSON_ITER_HOLDS_UTF8(config)) {
                // TODO: Parse directly from cnf string - see query.c/parse_cnf
            }
        } else if (strcmp(key, "$ddb") == 0){
            bson_type_t btype = bson_iter_type(config);
            switch (btype){
                case BSON_TYPE_BINARY: {
                    bson_subtype_t subtype;
                    uint32_t binary_len;
                    const uint8_t * binary = NULL;
                    md->db = ddb_new(); // verify this isn't null
                    bson_iter_binary(config, &subtype, &binary_len, &binary);
                    int load_error = ddb_loads(md->db, (char*)binary, binary_len);
                    //debug_print_info(md->db);
                    if (!load_error) {
                        md->state = md->state | MODULE_DISCO_HAS_DDB;
                    }
                    break;
                }
                case BSON_TYPE_UTF8: {
#ifdef ALLOW_FILESYSTEM
                    // assume this is a clean path to a disk or network path with a ddb on it
                    // super unsafe because this library doesn't really want to deal with safe disk access
                    // blocked by a compile time flag that should be default off
                    // recompile enableing it and the filesytem access safety is on the library user
                    uint32_t assumed_path_len;
                    const char * assumed_path =  bson_iter_utf8(config, &assumed_path_len); // check this looks right?
                    md->db_fd = open(assumed_path, O_RDONLY); //check this isn't -1?
                    md->db = ddb_new(); // check this isn't null
                    int load_error = ddb_load(md->db, md->db_fd); // check this is valid
                    if (!load_error){
                        md->state = md->state | MODULE_DISCO_HAS_DDB;
                    }
#endif
                    break; //  Outside the pre-processor block so it doesnt fall to default if default changes later
                }
                default:
                    break;
            }

        } else {
            // some other key, who knows... skip it
        }
    }
    if (md->state != MODULE_DISCO_G2G){
        md->compare = MATCHER_MODULE_DISCO_INVALID;  //result = false, explain why
    } else {
        result = true;
    }
    op->module.config.container.module_data = (void*)md;
    return result;
}

void *
matcher_module_disco_prep(mongoc_matcher_op_t *op){
    matcher_container_disco_usermem_t *ud; // user data
    ud = (matcher_container_disco_usermem_t*) bson_malloc0(sizeof *ud);
    return (void*)ud;
}

mongoc_matcher_module_callback_t
matcher_module_disco_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    matcher_container_disco_holder_t *md;
    matcher_container_disco_usermem_t *ud;
    bson_subtype_t subtype;
    uint32_t binary_len;
    const uint8_t * binary = NULL;

    md = (matcher_container_disco_holder_t*) op->module.config.container.module_data;
    ud = (matcher_container_disco_usermem_t*) usermem;

    // DO THE SEARCH
    // usrmem ddb_entry is exactly like bson_binary  any binary string and a length
    if (iter){
        bson_type_t btype = bson_iter_type(iter);
        //----------------------------------------------------------------------
        //  --Consider move to a "get_binary" static function?
        switch (btype){
            case BSON_TYPE_BINARY:{
                bson_iter_binary(iter, &subtype, &binary_len, &binary);
                break;
            }
            case BSON_TYPE_UTF8:{
                binary = (const uint8_t *) bson_iter_utf8(iter, &binary_len);
                break;
            }
            case BSON_TYPE_ARRAY:{
                bson_iter_t child;
                bson_iter_recurse(iter, &child);
                while(cb == MATCHER_MODULE_CALLBACK_CONTINUE && bson_iter_next(&child)){
                    cb = matcher_module_disco_search(op, &child, usermem);
                }
                return cb;  // This sucks
                // break;   // Should be break and the rest of this function should be another function
            }
            default:
                break;
        }
        // --- Consider moving above to get_binary static function
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        //  --Consider move to a "key_exists" static function?
        if (binary && md->compare == MATCHER_MODULE_DISCO_KEY_EXISTS) {
            ud->kentry.data = (const char *)binary;
            ud->kentry.length = binary_len;
            struct ddb_cursor * cur = ddb_getitem(md->db, &ud->kentry);
            int ddb_errno;
            //debug_print_cursor(md->db, cur, &ud->kentry);
            while ((ud->ventry = ddb_next(cur, &ddb_errno))){
                /*
                 printf("Key: %.*s -- Value:%.*s\n",
                        ud->kentry.length, ud->kentry.data,
                        ud->ventry->length, ud->ventry->data);
                */
                cb = MATCHER_MODULE_CALLBACK_FOUND;
                break; // because stop once we know it's there
            }
            ddb_free_cursor(cur);
        }
        //  --Consider move to a "key_exists" static function?
        // ----------------------------------------------------------------------
        //----------------------------------------------------------------------
        //  --Consider move to a "value_in" static function?
        else if (binary && md->compare == MATCHER_MODULE_DISCO_VALUE_IS) {
            ud->kentry.data = (const char *)binary;
            ud->kentry.length = binary_len;
            struct ddb_cursor * cur = ddb_getitem(md->db, &ud->kentry);
            int ddb_errno;
            while ((ud->ventry = ddb_next(cur, &ddb_errno))){
                if (md->search_len == ud->ventry->length &&
                    strncmp((const char *)md->search, ud->ventry->data, md->search_len) == 0){
                    cb = MATCHER_MODULE_CALLBACK_FOUND;
                    break; // because stop once we know it's there
                }
            }
            ddb_free_cursor(cur);
        }
        //  --Consider move to a "value_in" static function?
        // ----------------------------------------------------------------------
        //----------------------------------------------------------------------
        //  --Consider move to a "value_only" static function?
        else if (binary && md->compare == MATCHER_MODULE_DISCO_VALUE_ONLY) {
            ud->kentry.data = (const char *)binary;
            ud->kentry.length = binary_len;
            struct ddb_cursor * cur = ddb_getitem(md->db, &ud->kentry);
            int ddb_errno;
            while ((ud->ventry = ddb_next(cur, &ddb_errno))){
                if (md->search_len == ud->ventry->length &&
                    strncmp((const char*)md->search, ud->ventry->data, md->search_len) == 0){
                    cb = MATCHER_MODULE_CALLBACK_FOUND;
                } else {
                    cb = MATCHER_MODULE_CALLBACK_STOP;
                    break;
                }
            }
            ddb_free_cursor(cur);
        }
        //  --Consider move to a "value_only" static function?
        // ----------------------------------------------------------------------
        //----------------------------------------------------------------------
        //  --Consider move to a "Q" static function?
        else if (binary && md->compare == MATCHER_MODULE_DISCO_VALUE_MATCHES_Q) {
            ud->kentry.data = (const char *)binary;
            ud->kentry.length = binary_len;
            if (md->precache){
                // START THIS HAS BECOME KEY.EXISTS NOW, READY FOR FUNCTION
                struct ddb_cursor * cur = ddb_getitem(md->db, &ud->kentry);
                int ddb_errno;
                while ((ud->ventry = ddb_next(cur, &ddb_errno))){
                    cb = MATCHER_MODULE_CALLBACK_FOUND;
                    break; // because stop once we know it's there
                }
                ddb_free_cursor(cur);
                // END THIS HAS BECOME KEY.EXISTS NOW, READY FOR FUNCTION
            } else {
                struct ddb_cursor * cur = ddb_query(md->db, md->clauses, md->clauses_len);
                int ddb_errno;
                while ((ud->ventry = ddb_next(cur, &ddb_errno))){
                    if (ud->kentry.length == ud->ventry->length &&
                        strncmp(ud->kentry.data, ud->ventry->data, ud->kentry.length) == 0){
                        cb = MATCHER_MODULE_CALLBACK_FOUND;
                        break;
                    }
                }
                ddb_free_cursor(cur);
            }

        }
        //  --Consider move to a "value_only" static function?
        // ----------------------------------------------------------------------
    } else {
        // API will always provide a null iter at the end if module always calls back continue
        // Null iter means this is the last descendant in this document,
        // last chance to decide if it matches
    }
    return cb;
}


mongoc_matcher_module_callback_t
matcher_module_disco_cleanup(mongoc_matcher_op_t *op, void * usermem){
    mongoc_matcher_module_callback_t cb = MATCHER_MODULE_CALLBACK_CONTINUE;
    bson_free(usermem);
    return cb;
}

bool
matcher_module_disco_destroy(mongoc_matcher_op_t *op){
    bool result = false;
    matcher_container_disco_holder_t *md;
    md = (matcher_container_disco_holder_t *) op->module.config.container.module_data;
    ddb_free(md->db);
    md->db = NULL;
    uint32_t i = 0, j=0;
    for (i=0; i<md->clauses_len; i++){
        if (md->clauses[i].terms){
            free(md->clauses[i].terms);
        }
    }
    if (md->clauses){
        free(md->clauses);
        md->clauses = NULL;
    }
#ifdef ALLOW_FILESYSTEM
    if (md->db_fd > 0){
        close(md->db_fd);  // maybe check this is 0 ,<- means actually closed?
    }
#endif
    if (md->precache_data){
        free(md->precache_data);
        md->precache_data = NULL;
    }
    bson_free(md);
    return result;
}


#endif /* WITH_DISCO */
#endif /* WITH_MODULES */