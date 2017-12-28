#include "mongoc-matcher-op-modules.h"
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
    int started = bsonsearch_startup();
    int i =0;
    do {
        i++;
        BSON_ASSERT(compare_json("{ \"arr\" : { \"$binary\" : { \"base64\": \"qGC2JlwgCAAn9gzECABFEEBI6ApAAEAG4PXAqFg2wKhYGAAW7McL7DTmTwx9KoAYBZhx2gAAAQEIChYLl5g0Mjly\", \"subType\" : \"00\" } }}",
                                 "{\"arr\":{\"$module\":{\"name\":\"ether\", \"config\":{\"type\":\"srcip&dstip\", \"query\":[ [\"192.168.88.54\"],[\"192.168.88.54\",\"255.255.255.0\"] ] }}}}"));

        BSON_ASSERT(compare_json("{ \"arr\" : { \"$binary\" : { \"base64\": \"qGC2JlwgCAAn9gzECABFEEBI6ApAAEAG4PXAqFg2wKhYGAAW7McL7DTmTwx9KoAYBZhx2gAAAQEIChYLl5g0Mjly\", \"subType\" : \"00\" } }}",
                                  "{\"arr\":{\"$module\":{\"name\":\"ether\", \"config\":{\"type\":\"dstiplist\", \"query\":[ \"192.168.88.1\", \"192.168.88.24\", \"192.168.88.3\", \"192.168.88.52\"] }}}}"));


        BSON_ASSERT(!compare_json("{ \"arr\" : { \"$binary\" : { \"base64\": \"qGC2JlwgCAAn9gzECABFEEBI6ApAAEAG4PXAqFg2wKhYGAAW7McL7DTmTwx9KoAYBZhx2gAAAQEIChYLl5g0Mjly\", \"subType\" : \"00\" } }}",
                                  "{\"arr\":{\"$module\":{\"name\":\"ether\", \"config\":{\"type\":\"srciplist\", \"query\":[ \"192.168.88.1\", \"192.168.88.2\", \"192.168.88.3\", \"192.168.88.52\"] }}}}"));
        BSON_ASSERT(compare_json("{ \"arr\" : { \"$binary\" : { \"base64\": \"qGC2JlwgCAAn9gzECABFEEBI6ApAAEAG4PXAqFg2wKhYGAAW7McL7DTmTwx9KoAYBZhx2gAAAQEIChYLl5g0Mjly\", \"subType\" : \"00\" } }}",
                                 "{\"arr\":{\"$module\":{\"name\":\"ether\", \"config\":{\"type\":\"srciplist\", \"query\":[ \"192.168.88.1\", \"192.168.88.2\", \"192.168.88.3\", \"192.168.88.54\"] }}}}"));

        BSON_ASSERT(compare_json("{ \"arr\" : { \"$binary\" : { \"base64\": \"qGC2JlwgCAAn9gzECABFEEBI6ApAAEAG4PXAqFg2wKhYGAAW7McL7DTmTwx9KoAYBZhx2gAAAQEIChYLl5g0Mjly\", \"subType\" : \"00\" } }}",
                                 "{\"arr\":{\"$module\":{\"name\":\"ether\", \"config\":{\"type\":\"srcip&dstip\", \"query\":[ [\"192.168.88.54\",\"255.255.255.255\"],[\"192.168.88.54\",\"255.255.255.0\"] ] }}}}"));
        /**/
    } while (i<10);



    int finished = bsonsearch_shutdown();

    return 0;
}