#ifdef WITH_MODULES
#ifdef WITH_ETHER
#ifndef MATCHER_MODULE_ETHER_H
#define MATCHER_MODULE_ETHER_H
#include <bson/bson.h>
#include "mongoc-matcher-op-private.h"

#include "uthash.h"

#include<net/ethernet.h>
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MODULE_ETHER_COMMAND "ether"
#define MODULE_ETHER_IPV4_255_MASK 4294967295LL

typedef struct _matcher_container_ether_t matcher_container_ether_t;
typedef struct _module_ether_compare module_ether_compare;

typedef enum {
    MATCHER_ETHER_UNKNOWN,
    MATCHER_ETHER_SOURCE,
    MATCHER_ETHER_SOURCE_LIST,
    MATCHER_ETHER_DEST,
    MATCHER_ETHER_DEST_LIST,
    MATCHER_ETHER_SOURCEANDDEST,
    MATCHER_ETHER_SOURCEORDEST,
    MATCHER_ETHER_DESTPORT,
    MATCHER_ETHER_SOURCEPORT,
    MATCHER_ETHER_SOURCEDESTPORT,
    MATCHER_ETHER_UNDEFINED,
} matcher_ether_opcode_t;

bool
matcher_module_ether_startup(mongoc_matcher_op_t * op, bson_iter_t * config);

bool
matcher_module_ether_destroy(mongoc_matcher_op_t *op);

mongoc_matcher_module_callback_t
matcher_module_ether_search(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem);

mongoc_matcher_module_callback_t
matcher_module_ether_search_i4(mongoc_matcher_op_t * op, bson_iter_t * iter, void * usermem, matcher_container_ether_t * md);

mongoc_matcher_module_callback_t
matcher_module_ether_search_i4_list(mongoc_matcher_op_t * op, bson_iter_t * iter,
                                    void * usermem, matcher_container_ether_t * md);
matcher_ether_opcode_t
matcher_module_ether_get_opcode(const char *type, uint32_t len);

bool
matcher_module_ether_get_query(matcher_container_ether_t *md, bson_iter_t *query);

bool
matcher_module_ether_get_query_data(matcher_container_ether_t *md, bson_iter_t *query,
                                    struct in_addr *target, struct in_addr *mask);

typedef struct _module_ether_list_ip4 module_ether_list_ip4;
struct _module_ether_list_ip4 {
    in_addr_t addr;
    UT_hash_handle hh;
};

struct _matcher_container_ether_t {
    matcher_ether_opcode_t opcode;
    module_ether_list_ip4 *addrset;
    struct ethhdr *e;
    struct iphdr *i4;
    struct tcphdr *t;
    uint32_t smask;
    uint32_t dmask;
};


#endif /* MATCHER_MODULE_ETHER_H */
#endif /* WITH_ETHER */
#endif /* WITH_MODULES */