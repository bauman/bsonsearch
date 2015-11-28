cmake_minimum_required(VERSION 3.3)
project(bsonsearch)

INCLUDE_DIRECTORIES(/usr/include/ /usr/include/libbson-1.0/ lib/)
LINK_DIRECTORIES(/usr/lib64/)
set(SOURCE_FILES lib/bsoncompare.c lib/mongoc-matcher.c lib/mongoc-matcher-op.c)


add_library(libbsonsearch SHARED ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(libbsonsearch bson-1.0 pcre)

enable_testing()


function(bsonsearch_add_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} libbsonsearch )
endfunction()


bsonsearch_add_test(simple lib/tests/simple.c)
bsonsearch_add_test(mongodbtests lib/tests/matchertest.c)
bsonsearch_add_test(searchtest lib/tests/bsoncompare_simple.c)
