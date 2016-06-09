#include <bson.h>
#include <bcon.h>

int
main (int   argc,
      char *argv[])
{

    bson_t * query;
    query = BCON_NEW("$and","[",
                                 "{", "a", BCON_INT32(1), "}",
                                 "{", "b", "{", "$gte", BCON_INT32(1), "}", "}",
                                 "{", "b", "{", "$lt", BCON_INT32(3), "}", "}",
                             "]");
    //prove it looks right
    size_t *s;
    const char * as_json;
    as_json = bson_as_json(query, s);
    printf("%s\n", as_json);
    return 0;
}