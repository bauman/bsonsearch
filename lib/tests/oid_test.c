#include <bson/bson.h>
//toying around with oids.  Seems like correct function to use is oid_compare
//https://github.com/mongodb/libbson/blob/b52fca2b0f2eb71674447b329967870096602352/tests/test-oid.c
//gcc -I/usr/include/libbson-1.0 -lbson-1.0 -o test.out oid_test.c
int
main (void)
{
    bson_oid_t oid1;
    bson_oid_t oid2;

    bson_oid_init_from_string(&oid1, "000000000000000000001234");
    bson_oid_init_from_string(&oid2, "000000000000000000001235");
    int eq = bson_oid_compare(&oid1, &oid2);
    printf("Result: %d\n", eq);
}