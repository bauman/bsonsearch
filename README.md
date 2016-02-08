License
================
This code is licensed under permissive Apache 2 and MIT license.

Linking to the official server matching engine falls under copyleft AGPL, this library provides minimal matching capability with permissive license


Code contributed by MongoDB, Inc is licesned Apache 2

Code contributed by bauman is licensed MIT


Acknowledgement
===============

Many thanks to Christian Hergert and A. Jesse Jiryu Davis from MongoDB Inc for creating and maintaining the minimized matcher code from the mongo-c-driver.


I've moved the matcher codebase into this repo rather than link to the driver as MongoDB will rightly [remove the c matcher](https://jira.mongodb.org/browse/CDRIVER-955).


This repo continues the effort for a minimally functional and complimentary bson matching engine.


The officially supported [server matching engine](https://github.com/mongodb/mongo/tree/master/src/mongo/db/matcher) will always be far superior option.

speed comparison against json
==========
https://github.com/bauman/bsonsearch/blob/master/data/bsonsearch_vs_ujson_and_dict_search.ipynb



bsonsearch
==========

shared object to perform mongodb-like queries against raw bson rather than through a mongod

install on centos (el7) with RPMs

http://pkgs.bauman.in/el7/repoview/python-bsonsearch.html

or add from repo


```
    [pkgs.bauman.in]
    name=Packaged tools for RHEL7 (centos)
    type=rpm-md
    baseurl=http://pkgs.bauman.in/el7/
    gpgcheck=1
    gpgkey=http://pkgs.bauman.in/el7/repodata/RPM-GPG-KEY-Bauman
    enabled=1
```

yum install python-bsonsearch


compile
========

runtime requires

libbson (https://github.com/mongodb/libbson)

libpcre

compilation also requires

libbson-devel

pcre-devel

uthash-devel



```
    gcc -Wall $(pkg-config --cflags --libs libbson-1.0) -lpcre -shared -o libbsoncompare.so -fPIC bsoncompare.c mongoc-matcher.c mongoc-matcher-op.c mongoc-matcher-op-geojson.c
```


Usage
==========

The spec parameter supports a subset of MongoDB query operators (http://docs.mongodb.org/manual/reference/operator/query/) 

Currently, that includes $and, $or, $not, $in, $nin, $eq, $neq, $gt, $gte, $lt, $lte, and $near. (See full documentation http://api.mongodb.org/c/current/mongoc_matcher_t.html)

comparison value in spec can be utf8 string, int/long, regex


``` python
    from bsonsearch import bsoncompare
    import bson
    bc = bsoncompare()
    b = {"a":{"$gte":1}}
    c=[ {"a":0},{"a":1},{"a":2}]#dict
    c2=[bson.BSON.encode(x) for x in c] #already bson
    matcher = bc.generate_matcher(b)
    print [bc.match(matcher, x) for x in c]
    print [bc.match(matcher, x) for x in c2]
    bc.destroy_matcher(matcher)
```


Regex within SPEC
==================

bsonsearch supports the use of compiled regex using libpcre.  The only regex option allowed is re.IGNORECASE, and only that option. Adding other options seperately or in addition to ingnore case is undefined.


``` python
    import bsonsearch
    import re

    bc = bsonsearch.bsoncompare()
    doc = {'a': "hello world"}
    doc_id = bc.generate_doc(doc)
    spec = {"a": re.compile("orl")}
    matcher = bc.generate_matcher(spec)
    print bc.match_doc(matcher, doc_id)
    bc.destroy_doc(doc_id) #destroy the document
    bc.destroy_matcher(matcher) #destroy the spec
    bc.destroy_regexes() #BSONCOMPARE caches compiled regex (caller MUST explicitly destroy the cache)

    >>> True
```


If the document contains lists within the namespace, libbson cannot handle queries like mongodb server.

User needs to call the convert_to_and function to build an appropriate spec for the document.

    convert_to_and(spec, doc_id)


Optionally, the user may call destory_matcher following the match, or choose to wait and clear old matchers at a later time.

ipython notebook

``` python
    import bsonsearch
    bc = bsonsearch.bsoncompare()
    doc = {'a': [{'b': [1, 2]}, {'b': [3, 5]}],
           "c":{"d":"dan"}}
    doc_id = bc.generate_doc(doc)
    spec = {"a.b":{"$in":[7, 6, 5]},
            "c.d":"dan"}
    query = bc.convert_to_and(spec, doc_id)
    print query
    matcher = bc.generate_matcher(query)
    print bc.match_doc(matcher, doc_id)
    bc.destroy_doc(doc_id)
    bc.destroy_doc(bc.docs)
    bc.destroy_matcher(bc.matchers)

    >>> {'$and': [{'$or': [{'c.d': 'dan'}]}, {'$or': [{'a.0.b.0': {'$in': [7, 6, 5]}}, {'a.0.b.1': {'$in': [7, 6, 5]}}, {'a.1.b.0': {'$in': [7, 6, 5]}}, {'a.1.b.1': {'$in': [7, 6, 5]}}]}]}
    >>> True
```

$near example
==================
Makes a flat grid distance calculation.

Grid units are arbitrarily determined by the user.

Currently supports 2D or 3D calculations.
``` python
    import bsonsearch
    from bson.son import SON
    import bson
    
    bc = bsonsearch.bsoncompare()
    doc = {'pos':[200, 150]} #Legacy Point format.
    doc_id = bc.generate_doc(doc)
    #Test your luck using a python doc, but I'd recommend on using SON
    #spec = {"pos":{"$maxDistance":100, "$near":[200, 151] }} #putting $maxDistance first will serialize $near correctly in CPython
    near_cmd = SON()
    near_cmd["$near"] = [200, 151]
    near_cmd["$maxDistance"] = 100
    spec = SON()
    spec['pos'] = near_cmd
    spec = bson.BSON.encode(spec)
    
    matcher = bc.generate_matcher(spec)
    print bc.match_doc(matcher, doc_id) #--True--
    
    bc.destroy_doc(bc.docs)
    bc.destroy_matcher(bc.matchers)
    
    >>>True
```

GeoNear $near example
==================
Uses GeoJSON (https://docs.mongodb.org/manual/reference/operator/query/near/#op._S_near)

Only supports point difference

Grid units are in meters

``` python
    import bsonsearch
    from bson.son import SON
    import bson

    bc = bsonsearch.bsoncompare()
    doc = {"loc": {
                    "type": "Point" ,
                    "coordinates": [ -61.08080307722216 , -9.057610600760512 ]
                 }
           }
    doc_id = bc.generate_doc(doc)
    #Test your luck using a python doc, but I'd recommend on using SON
    spec = {  "loc":{
              "$near": {
                 "$geometry": {
                    "type": "Point" ,
                    "coordinates": [ -61.08080307722216 , -9.057610600760512 ]
                 },
                 "$maxDistance": 1.0,
                 "$minDistance": 0.0
              }
            }
           }
    matcher = bc.generate_matcher(spec)
    print bc.match_doc(matcher, doc_id) #Same coordinate, evaluates to 0

    bc.destroy_doc(bc.docs)
    bc.destroy_matcher(bc.matchers)

    >>>True
```

streaming example
==================

this example uses KeyValueBSONInput (https://github.com/bauman/python-bson-streaming)

raw bson file containing 7GB of metadata from the enron data set

This amounts to nothing more than a full table scan through a single mongod but can be multiplexed across multiple bson files with multiple processes using tools like xargs


``` python
    from bsonstream import KeyValueBSONInput
    from bsonsearch import bsoncompare
    bc = bsoncompare()
    b = {"a":{"$gte":1}}
    matcher = bc.generate_matcher(b)
    f = open("/home/dan/enron/enron.bson", 'rb')
    stream = KeyValueBSONInput(fh=f, decode=False)
    for doc in stream:
        print bc.match(matcher, doc)
    f.close()
    bc.destroy_matcher(matcher)
```


