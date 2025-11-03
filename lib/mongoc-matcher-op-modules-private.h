#ifdef WITH_MODULES


#ifndef MONGOC_MATCHER_OP_MODULES_PRIVATE_H
#define MONGOC_MATCHER_OP_MODULES_PRIVATE_H


#include <bson/bson.h>

BSON_BEGIN_DECLS

typedef struct  _mongoc_matcher_op_module_base_t  mongoc_matcher_op_module_base_t;
typedef struct  _mongoc_matcher_op_module_container_t  mongoc_matcher_op_module_container_t;
typedef union   _mongoc_matcher_op_module_t mongoc_matcher_op_module_t;

typedef enum {
    MATCHER_MODULE_OPCODE_BASE,
    //--ADD MODULE OPCODES BELOW THIS LINE------------


    MATCHER_MODULE_OPCODE_BETWEEN,
    MATCHER_MODULE_OPCODE_SUM,
    MATCHER_MODULE_OPCODE_ETHER,
    MATCHER_MODULE_OPCODE_IP,
    MATCHER_MODULE_OPCODE_DISCO,
    MATCHER_MODULE_OPCODE_DUKJS,
    MATCHER_MODULE_OPCODE_SAMPLE,

    //--ADD MODULE OPCODES ABOVE THIS LINE------------
    MATCHER_MODULE_OPCODE_UNKNOWN,
    MATCHER_MODULE_OPCODE_UNDEFINED,
} mongoc_matcher_module_opcode_t;

typedef enum {
    MATCHER_MODULE_CALLBACK_UNDEFINED,
    MATCHER_MODULE_CALLBACK_CONTINUE,
    MATCHER_MODULE_CALLBACK_STOP,
    MATCHER_MODULE_CALLBACK_FOUND,
} mongoc_matcher_module_callback_t;

struct _mongoc_matcher_op_module_base_t {
    mongoc_matcher_module_opcode_t opcode;
    char * name;
};
struct _mongoc_matcher_op_module_container_t {
    mongoc_matcher_module_opcode_t opcode;
    char * name;
    void * module_data;
};
union  _mongoc_matcher_op_module_t
{
    mongoc_matcher_op_module_base_t base;
    mongoc_matcher_op_module_container_t container;

};

BSON_END_DECLS
#endif /* MONGOC_MATCHER_OP_MODULES_PRIVATE_H */
#endif /* WITH_MODULES */