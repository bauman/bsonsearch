cmake_minimum_required(VERSION 3.3)
project(bsonsearch)

INCLUDE_DIRECTORIES(/usr/include/ /usr/include/libbson-1.0/ lib/)
LINK_DIRECTORIES(/usr/lib64/)
add_definitions(-DWITH_YARA)
add_definitions(-DWITH_PROJECTION)
set(SOURCE_FILES lib/bsoncompare.c lib/mongoc-matcher.c lib/mongoc-matcher-op.c lib/mongoc-matcher-op-geojson.c lib/mongoc-bson-descendants.c lib/mongoc-matcher-op-yara.c lib/mongoc-projection.c)


add_library(libbsonsearch SHARED ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(libbsonsearch bson-1.0 pcre)
TARGET_LINK_LIBRARIES(libbsonsearch yara)
enable_testing()


function(bsonsearch_add_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} libbsonsearch )
endfunction()

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
bsonsearch_add_test(compare-box lib/tests/bsoncompare_box.c)
bsonsearch_add_test(bson-fix lib/tests/bson_descendants.c)
bsonsearch_add_test(big-doc lib/tests/bigdoctest.c)
bsonsearch_add_test(inset lib/tests/bsoncompare_inset.c)
bsonsearch_add_test(yara-test lib/tests/bsonsearch_yara.c)
bsonsearch_add_test(projection-test lib/tests/bsoncompare_projection.c)



