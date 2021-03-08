#include <stdio.h>
#include <bsoncompare.h>



int
main (int   argc,
      char *argv[])
{
    double ok = bsonsearch_get_crossarc_degrees(10, 20, 10, 21, 10.5, 20.5);

    if ((ok > 52075.0) && (ok < 52077.0) ){
        return 0;
    } else {
        return -10;
    }
}
