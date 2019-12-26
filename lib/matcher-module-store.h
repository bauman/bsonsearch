#ifdef WITH_MODULES
#ifndef MATCHER_MODULE_STORE_H
#define MATCHER_MODULE_STORE_H
#include <uthash.h>

#include "mongoc-matcher-op-modules-private.h"
#include "matcher-module-between.h"
#include "matcher-module-math.h"
#include "matcher-module-ether.h"
#include "matcher-module-ip.h"
#include "matcher-module-disco.h"
#include "matcher-module-duk.h"
#include "matcher-module-sample.h"


uint32_t
_matcher_module_store_startup();

uint32_t
_matcher_module_store_shutdown();

uint32_t
_matcher_module_store_count();

#endif /* MATCHER_MODULE_STORE_H */
#endif /* WITH_MODULES */