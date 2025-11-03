#include <bson/bson.h>
#include <time.h>

int
main (int   argc,
      char *argv[])
{

    bson_t * query;


    /*
    time_t oid_thinks_time;  //what time does the OID think it is
    bson_oid_t oid;
    bson_oid_t *oid_pointer = &oid;
    bson_oid_init (&oid, NULL);  // get a standard ObjectId
    oid_thinks_time = bson_oid_get_time_t (&oid); //It was just made
    printf ("The OID was generated at %u\n", (unsigned) oid_thinks_time); //prove it



    time_t  ts = time(NULL);  //make a new time
    struct tm * timeinfo = localtime(&ts);
    timeinfo->tm_year = 2014-1900;  //-1900 because time.h
    timeinfo->tm_mon  = 12 - 1;     // time.h off by one (starts at 0)
    timeinfo->tm_mday = 25;
    ts = mktime(timeinfo);          // create the time
    u_int32_t ts_uint = (uint32_t)ts;
    ts_uint = BSON_UINT32_TO_BE (ts_uint); //BSON wants big endian time
    memcpy (&oid_pointer->bytes[0], &ts_uint, sizeof (ts_uint));  //overwrite the first 4 bytes with user selected time
    oid_thinks_time = bson_oid_get_time_t (&oid);
    printf ("The OID was fixed to time %u\n", (unsigned) oid_thinks_time);//prove it
*/

    query = BCON_NEW ("ping", BCON_INT32 (1));
    query = BCON_NEW("$and","[",
                     "{", "a", BCON_INT32(1), "}",
                     "{", "yyyyyyy", "{", "$ne", BCON_UTF8("xxxxxxxxxx"), "}", "}","]");

    bson_t *doc;

    query = BCON_NEW (
                    "$and", "[", "{", "_id", BCON_INT32(1), "}",
                                 "{", "yyyyyy", "{", "$ne", BCON_UTF8 ("xxxxxxx"), "}", "}","]"
                    );
    query = BCON_NEW("$and","[",
                     "{", "timestamp", "{", "$eq", BCON_INT64(1111111111112), "}", "}",
                     "{", "timestamp", "{", "$gte", BCON_INT64(1111111111111), "}", "}",
                     "{", "timestamp", "{", "$lt", BCON_INT64(1111111111110), "}", "}","]");
    //prove it looks right

    size_t s;
    char * as_json;
    as_json = bson_as_canonical_extended_json(query, &s);
    printf("%s\n", as_json);
    return 0;
}