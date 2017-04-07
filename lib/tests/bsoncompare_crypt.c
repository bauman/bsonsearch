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

    BSON_ASSERT(compare_json("{\"pk\": {\"$binary\": \"jkIR1Wh0kwXu7RCttJC3XZIErIK1oiJ2+VvWXhSLEh0=\", \"$type\": \"00\"}, \"enc\": {\"$binary\": \"9QdGvookp+rksHsLdMUAP6JRWzwLfbAVgyQkNy1OJwmNKLt/QCbEcQy3EiXXHhYjvVRpo+myLE6rduy3LBBxG0Z4wjkPibSqf6xrPr4T+8nm8rRzC+1G/KZTe5g48jrpgIRQ/DKsD41cDOQgrffJe4Uer4KnZkzJO6/lJlSmlFmknCSM5hbhDr7ZPBKLgBZh7qF/gLzLz1nE3A==\", \"$type\": \"00\"}}",
                             "{\"enc\": {\"$sealOpen\": {\"$keys\": {\"sk\": {\"$binary\": \"4haaLLGbtPY36fiDDyJfqH100mk6u/21iolYLhsTklo=\", \"$type\": \"00\"}, \"pk\": {\"$binary\": \"jkIR1Wh0kwXu7RCttJC3XZIErIK1oiJ2+VvWXhSLEh0=\", \"$type\": \"00\"}}, \"$query\": {\"data\": 2}}}}"));

    BSON_ASSERT(compare_json("{\"pk\": {\"$binary\": \"jkIR1Wh0kwXu7RCttJC3XZIErIK1oiJ2+VvWXhSLEh0=\", \"$type\": \"00\"}, \"enc\": {\"$binary\": \"9QdGvookp+rksHsLdMUAP6JRWzwLfbAVgyQkNy1OJwmNKLt/QCbEcQy3EiXXHhYjvVRpo+myLE6rduy3LBBxG0Z4wjkPibSqf6xrPr4T+8nm8rRzC+1G/KZTe5g48jrpgIRQ/DKsD41cDOQgrffJe4Uer4KnZkzJO6/lJlSmlFmknCSM5hbhDr7ZPBKLgBZh7qF/gLzLz1nE3A==\", \"$type\": \"00\"}}",
                             "{\"enc\": {\"$sealOpen\": {\"$keys\": {\"sk\": {\"$binary\": \"4haaLLGbtPY36fiDDyJfqH100mk6u/21iolYLhsTklo=\", \"$type\": \"00\"}, \"pk\": {\"$binary\": \"jkIR1Wh0kwXu7RCttJC3XZIErIK1oiJ2+VvWXhSLEh0=\", \"$type\": \"00\"}}, \"$query\": {\"data\": 2}}}}"));


    return 0;
}