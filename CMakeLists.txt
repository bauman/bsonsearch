cmake_minimum_required(VERSION 3.3)
project(bsonsearch)

# Default cmake includes all features and modules
# Optional "lite" version available with only bson and pcre dependencies
#   mkdir cmake-build-lite && cd cmake-build-lite
#   cmake -D include-yara=OFF -D include-modules=OFF -D include-text=OFF -D include-projection=OFF -D include-utils=OFF -D include-conditional=OFF -D include-ip=OFF -D include-crypt=OFF ..
#   make
#   make test


## File Contents: This file is set in 3 main sections
#    1.  Compile options and dependency checking
#    2.  Library Compilation and INSTALL instructions
#    3.  Test and Utility Compilation


INCLUDE_DIRECTORIES(/usr/include/ /usr/local/include/libbson-1.0/ /usr/include/libbson-1.0/ lib/ lib/acceleration/)
LINK_DIRECTORIES(/usr/local/lib/ /usr/lib64/ /usr/local/lib/ lib/acceleration/)

##  SECTION 1 -- COMPILE Checks
include(CMakeDependentOption)
INCLUDE(CheckIncludeFiles)
INCLUDE (CheckLibraryExists)

option(include-projection "include-projection" ON)
option(include-utils "include-utils" ON)
option(include-conditional "include-conditional" ON)

option(include-text "include-text" ON)
CMAKE_DEPENDENT_OPTION(include-aspell "include-aspell" ON "include-text" OFF)
CMAKE_DEPENDENT_OPTION(include-stemmer "include-stemmer " ON "include-text" OFF)

option(include-ip "include-ip" ON)
option(include-yara "include-yara" ON)
option(include-crypt "include-crypt" ON)

option(include-modules "include-modules" ON)
CMAKE_DEPENDENT_OPTION(include-math "include-math" ON "include-modules" OFF)
CMAKE_DEPENDENT_OPTION(include-ether "include-ether" ON "include-modules" OFF)
CMAKE_DEPENDENT_OPTION(include-disco "include-disco" ON "include-modules" OFF)
CMAKE_DEPENDENT_OPTION(include-filesystem "include-filesystem" ON "include-modules" OFF)
CMAKE_DEPENDENT_OPTION(include-dukjs "include-dukjs" ON "include-modules" OFF)

CHECK_INCLUDE_FILES(math.h HAVE_MATH_H)
if(NOT HAVE_MATH_H)
    message(SEND_ERROR "math header is required, stopping")
    return()
endif()
CHECK_LIBRARY_EXISTS(m atan2 "" HAVE_MATH_LIB)
if(NOT HAVE_MATH_LIB)
    message(SEND_ERROR "math library is required, stopping")
    return()
endif()


CHECK_INCLUDE_FILES(uthash.h HAVE_UTHASH_H)
if(NOT HAVE_UTHASH_H)
    message(SEND_ERROR "uthash is required, stopping")
    return()
endif()

CHECK_INCLUDE_FILES(pcre.h HAVE_PCRE_H)
if(NOT HAVE_PCRE_H)
    message(SEND_ERROR "PCRE is required, stopping")
    return()
endif()


message("-- Looking for include file bson.h")
find_path(HAVE_BSON_H bson.h PATHS /usr/include/libbson-1.0 /usr/local/include/libbson-1.0/)
if(NOT HAVE_BSON_H)
    message(SEND_ERROR "BSON is required, stopping")
    return()
else()
    message("-- Looking for include file bson.h - found")
endif()

if(include-yara)
    message("-- requested yara with -D include-yara, checking for yara.h")
    CHECK_INCLUDE_FILES(yara.h HAVE_YARA_H)
    if(HAVE_YARA_H)
        CHECK_LIBRARY_EXISTS(yara yr_rules_scan_mem "" HAVE_YARA_LIB)
        if (HAVE_YARA_LIB)
            message("-- -- have yara.h and library, adding -DWITH_YARA")
            add_definitions(-DWITH_YARA)
        else()
            message(SEND_ERROR "-- yara enabled but cannot find libyara, is it installed?")
            return()
        endif()
    else()
        message(SEND_ERROR "-- yara enabled but cannot find yara.h, is yara-dev(el) installed?")
        return()
    endif()
endif()

if(include-text)
    message("-- requested text with -D include-text, adding -DWITH_TEXT")
    add_definitions(-DWITH_TEXT)
    if (include-stemmer)
        CHECK_INCLUDE_FILES(libstemmer.h HAVE_STEMMER_H)
        if(HAVE_STEMMER_H)
            CHECK_LIBRARY_EXISTS(stemmer sb_stemmer_new "" HAVE_STEMMER_LIB)
            if (HAVE_STEMMER_LIB)
                message("-- -- have libstemmer.h and library, adding -DWITH_STEMMER")
                add_definitions(-DWITH_STEMMER)
            else()
                message(SEND_ERROR "-- stemmer enabled but cannot find libstemmer, is libstemmer and -dev(el) installed?")
                return()
            endif()
        endif()
        CHECK_INCLUDE_FILES(aspell.h HAVE_ASPELL_H)
        if(HAVE_ASPELL_H)
            CHECK_LIBRARY_EXISTS(aspell aspell_speller_check "" HAVE_ASPELL_LIB)
            if (HAVE_ASPELL_LIB)
                message("-- -- have aspell.h, adding -DWITH_ASPELL")
                add_definitions(-DWITH_ASPELL)
            else()
                message(SEND_ERROR "-- ASPELL enabled but cannot find libaspell, is libaspell and -dev(el) installed?")
                return()
            endif()
        endif()
    endif()
endif()
if(include-projection)
    message("-- requested projection with -D include-projection, adding -DWITH_PROJECTION")
    add_definitions(-DWITH_PROJECTION)
endif()
if(include-utils)
    message("-- requested utils with -D include-utils, adding -DWITH_UTILS")
    add_definitions(-DWITH_UTILS)
endif()
if(include-conditional)
    message("-- requested conditional with -D include-conditional, adding -DWITH_CONDITIONAL")
    add_definitions(-DWITH_CONDITIONAL)
endif()
if(include-crypt)
    message("-- requested crypt with -D include-crypt")
    CHECK_INCLUDE_FILES(sodium.h HAVE_SODIUM_H)
    if(HAVE_SODIUM_H)
        CHECK_LIBRARY_EXISTS(sodium crypto_box_seal_open "" HAVE_SODIUM_LIB)
        if (HAVE_SODIUM_LIB)
            message("-- -- have sodium.h and library, adding -DWITH_CRYPT")
            add_definitions(-DWITH_CRYPT)
        endif()
    endif()
endif()
if(include-ip)
    message("-- requested ip with -D include-ip, adding -DWITH_IP")
    add_definitions(-DWITH_IP)
endif()
if (include-filesystem)
    message("-- requested filesystem access with -D include-filesystem, adding -DALLOW_FILESYSTEM")
    add_definitions(-DALLOW_FILESYSTEM)
endif()
if(include-modules)
    message("-- requested modules with -D include-modules")
    add_definitions(-DWITH_MODULES)
    if (include-math)
        message("-- requested math with -D include-math")
        add_definitions(-DWITH_MATH)
    endif()
    if (include-ether)
        message("-- requested ether with -D include-ether")
        CHECK_INCLUDE_FILES(net/ethernet.h HAVE_ETHERNET_H)
        CHECK_INCLUDE_FILES(netinet/tcp.h HAVE_TCP_H)
        CHECK_INCLUDE_FILES(netinet/ip.h HAVE_IP_H)
        CHECK_INCLUDE_FILES(sys/socket.h HAVE_SOCKET_H)
        CHECK_INCLUDE_FILES(netinet/in.h HAVE_NETINET_IN_H)
        CHECK_INCLUDE_FILES(arpa/inet.h HAVE_ARPA_INET_H)
        if(HAVE_ETHERNET_H AND HAVE_TCP_H AND HAVE_IP_H AND HAVE_SOCKET_H AND HAVE_NETINET_IN_H AND HAVE_ARPA_INET_H)
            add_definitions(-DWITH_ETHER)
        else()
            message(SEND_ERROR "-- ETHER module requires network header files")
            return()
        endif()
    endif()
    if (include-disco)
        message("-- requested disco with -D include-disco")
        CHECK_INCLUDE_FILES(discodb.h HAVE_DISCO_H)
        if(HAVE_DISCO_H)
            CHECK_LIBRARY_EXISTS(discodb ddb_getitem "" HAVE_DISCO_LIB)
            if (HAVE_DISCO_LIB)
                message("-- -- have discodb.h and library, adding -DWITH_DISCO")
                add_definitions(-DWITH_DISCO)
            endif()
        endif()
    endif()
    if (include-dukjs)
        message("-- requested dukjs with -D include-dukjs")
        CHECK_INCLUDE_FILES(duktape.h HAVE_DUK_H)
        if(HAVE_DUK_H)
            message("-- -- have duktape.h, adding -DWITH_DUKJS")
            add_definitions(-DWITH_DUKJS)
        else()
            message(SEND_ERROR "-- dukjs enabled but cannot find libduktape, is duktape-dev(el) installed?")
            return()
        endif()
    endif()
endif()
add_definitions(-DHAVE_STDBOOL_H)    # for yara on alpine
#add_definitions(-DWITH_YARA)
#add_definitions(-DWITH_PROJECTION)
#add_definitions(-DWITH_UTILS)
#add_definitions(-DWITH_CONDITIONAL)
#add_definitions(-DWITH_TEXT)
#add_definitions(-DWITH_STEMMER)
#add_definitions(-DWITH_ASPELL)
#add_definitions(-DWITH_CRYPT)
#add_definitions(-DWITH_IP)
#add_definitions(-DWITH_MODULES)
#add_definitions(-DWITH_MATH)
#add_definitions(-DWITH_ETHER)
#add_definitions(-DWITH_DISCO)
#add_definitions(-DALLOW_FILESYSTEM)   # for discodb now, yara or dukjs later crypt may need it to pull secret keys
#add_definitions(-DWITH_DUKJS)


##  END SECTION 1 -- COMPILE Checks

##  SECTION 2 -- Library Compilation
set(SOURCE_FILES lib/bsoncompare.c
                 lib/mongoc-matcher.c
                 lib/mongoc-matcher-op.c
                 lib/mongoc-matcher-op-geojson.c
                 lib/mongoc-bson-descendants.c
                 lib/mongoc-matcher-op-yara.c
                 lib/mongoc-projection.c
                 lib/mongoc-redaction.c
                 lib/mongoc-matcher-op-unwind.c
                 lib/mongoc-matcher-op-conditional.c
                 lib/mongoc-matcher-op-crypt.c
                 lib/mongoc-matcher-op-text.c
                 lib/mongoc-matcher-op-ip.c
                 lib/mongoc-matcher-op-modules.c
                 lib/matcher-module-store.c
                 lib/matcher-module-between.c
                 lib/matcher-module-math.c
                 lib/matcher-module-ether.c
                 lib/matcher-module-ip.c
                 lib/matcher-module-disco.c
                 lib/matcher-module-sample.c
                 lib/matcher-module-duk.c
        )

set(BQ_SOURCE_FILES utils/bq.c)

set(HEADER_FILES   lib/bsoncompare.h
        lib/matcher-module-between.h
        lib/matcher-module-disco.h
        lib/matcher-module-duk.h
        lib/matcher-module-ether.h
        lib/matcher-module-ip.h
        lib/matcher-module-math.h
        lib/matcher-module-sample.h
        lib/matcher-module-store.h
        lib/mongoc-bson-descendants.h
        lib/mongoc-error.h
        lib/mongoc-matcher.h
        lib/mongoc-matcher-op-conditional.h
        lib/mongoc-matcher-op-crypt.h
        lib/mongoc-matcher-op-geojson.h
        lib/mongoc-matcher-op-ip.h
        lib/mongoc-matcher-op-modules.h
        lib/mongoc-matcher-op-modules-private.h
        lib/mongoc-matcher-op-private.h
        lib/mongoc-matcher-op-text.h
        lib/mongoc-matcher-op-unwind.h
        lib/mongoc-matcher-op-yara.h
        lib/mongoc-matcher-private.h
        lib/mongoc-projection.h
        lib/mongoc-redaction.h
        )

add_library(bsonsearch SHARED ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(bsonsearch bson-1.0 pcre m)  # core systems



if (include-yara)
    TARGET_LINK_LIBRARIES(bsonsearch yara)           # if -D WITH_YARA
endif()
if (include-stemmer)
    TARGET_LINK_LIBRARIES(bsonsearch stemmer)        # if -D WITH_TEXT WITH_STEMMER
endif()
if (include-aspell)
    TARGET_LINK_LIBRARIES(bsonsearch aspell)         # if -D WITH_TEXT
endif()
if (include-crypt)
    TARGET_LINK_LIBRARIES(bsonsearch sodium)         # if -D WITH_CRYPT
endif()
if (include-disco)
    TARGET_LINK_LIBRARIES(bsonsearch discodb)        # if -D WITH_DISCO
endif()
if (include-dukjs)
    TARGET_LINK_LIBRARIES(bsonsearch duktape)        # if -D WITH_DUKJS
endif()

add_executable(bq ${BQ_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bq bsonsearch)

install(TARGETS bsonsearch bq
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(FILES ${HEADER_FILES} DESTINATION include)
install(FILES lib/LICENSING.txt DESTINATION share RENAME BSONSEARCH_LICENSING.txt)


##  END SECTION 2 -- Library Compilation and INSTALL instructions

##  SECTION 3 -- Test and Utility Compilation
enable_testing()


function(bsonsearch_add_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} bsonsearch )
    add_test(test_${test_name}  ${test_name})

endfunction()


bsonsearch_add_test(bcon-sample lib/tests/bcon_samples.c)
bsonsearch_add_test(compare-equal lib/tests/bsoncompare_simple.c)
bsonsearch_add_test(simple lib/tests/simple.c)
bsonsearch_add_test(mongodbtests lib/tests/matchertest.c)
bsonsearch_add_test(compare-regex-equal lib/tests/bsoncompare_regex.c)
bsonsearch_add_test(compare-leak-test lib/tests/bsoncompare_leak_test.c)
bsonsearch_add_test(gte-compare lib/tests/bsoncompare_gte_lists.c)
bsonsearch_add_test(size-compare lib/tests/bsoncompare_opcode_size.c)
bsonsearch_add_test(near-compare lib/tests/bsoncompare_near.c)
bsonsearch_add_test(dt-compare lib/tests/bsoncompare_datetime.c)
bsonsearch_add_test(oid-compare lib/tests/bsoncompare_oid.c)
bsonsearch_add_test(geonear-compare lib/tests/bsoncompare_geonear.c)
if(include-utils)
    bsonsearch_add_test(geonear-compare-util lib/tests/bsoncompare_geonear_util.c)
endif()
bsonsearch_add_test(compare-box lib/tests/bsoncompare_box.c)
bsonsearch_add_test(bson-fix lib/tests/bson_descendants.c)
bsonsearch_add_test(big-doc lib/tests/bigdoctest.c)
bsonsearch_add_test(inset lib/tests/bsoncompare_inset.c)
if (include-yara)
    bsonsearch_add_test(yara-test lib/tests/bsonsearch_yara.c)
endif()

bsonsearch_add_test(and-bail-test lib/tests/bsoncompare_and_bailout.c)
bsonsearch_add_test(json_decode lib/tests/bsoncompare_json_input.c)
bsonsearch_add_test(type-checking lib/tests/bsoncompare-typecheck.c)
bsonsearch_add_test(exists-check lib/tests/bsoncompare_exists.c)
bsonsearch_add_test(strlen-check lib/tests/bsoncompare_strlen.c)
bsonsearch_add_test(geospace lib/tests/bsoncompare_geospace.c)
if(include-conditional)
    bsonsearch_add_test(conditional lib/tests/bsoncompare_conditional.c)
endif()
if(include-crypt)
    bsonsearch_add_test(cryptcheck lib/tests/bsoncompare_crypt.c)
endif()
if (include-text)
    bsonsearch_add_test(textsearch lib/tests/bsoncompare_text.c)
endif()
if (include-aspell)
    bsonsearch_add_test(spellingsearch lib/tests/bsoncompare_spelling.c)
endif()
if(include-projection)
    bsonsearch_add_test(projection_leak lib/tests/projection_leak.c)
    bsonsearch_add_test(redaction lib/tests/bsoncompare_redaction.c)
    bsonsearch_add_test(projection-test lib/tests/bsoncompare_projection.c)
    bsonsearch_add_test(unwind_compare lib/tests/bsoncompare_unwind.c)
endif()
if(include-ip)
    bsonsearch_add_test(ipaddrs lib/tests/bsoncompare_binary_ip.c)
endif()
if(include-modules)
    bsonsearch_add_test(modulebase lib/tests/bsoncompare_modules.c)
    bsonsearch_add_test(samplingmodule lib/tests/bsonsearch_sample.c)
    if (include-math)
        bsonsearch_add_test(mathmodule lib/tests/bsoncompare_math.c)
    endif()
    if(include-ether)
        bsonsearch_add_test(ethermodule lib/tests/bsoncompare_ether.c)
    endif()
    if(include-ip)
        bsonsearch_add_test(ipmodule lib/tests/bsoncompare_module_ip.c)
    endif()
    if(include-disco)
        bsonsearch_add_test(discomodule lib/tests/bsonsearch_discodb.c)
    endif()
    if(include-dukjs)
        bsonsearch_add_test(dukjs lib/tests/bsonsearch_duk.c)
    endif()
endif()

##  END SECTION 3 -- Test and Utility Compilation

