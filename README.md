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
    gcc -Wall $(pkg-config --cflags --libs libbson-1.0) -lpcre -shared -o libbsoncompare.so -fPIC bsoncompare.c mongoc-matcher.c mongoc-matcher-op.c mongoc-matcher-op-geojson.c mongoc-bson-descendants.c
```


Usage
==========

The spec parameter supports a subset of MongoDB query operators (http://docs.mongodb.org/manual/reference/operator/query/) 

Currently, that includes $and, $or, $not, $in, $nin, $eq, $neq, $gt, $gte, $lt, $lte, and $near. (See full documentation http://api.mongodb.org/c/current/mongoc_matcher_t.html)

comparison value in spec can be utf8 string, int/long, regex, compiled yara (if compiled with yara support)



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

generally, you should use the with operator to handle construction and destruction of the underlying objects

You may still want to manage memory by cleaning up your own documents, mathers, and regexes, but the with operator will clean it up as it goes out of scope.

``` python
    with bsonsearch.bsoncompare() as bc:
        doc = {'a': "hello world"}
        doc_id = bc.generate_doc(doc)
        spec = {"a": "hello world"}
        matcher = bc.generate_matcher(spec)
        print bc.match_doc(matcher, doc_id) #this will segfault if signature invalid or no yara support in libbsonsearch
        bc.destroy_doc(doc_id) #destroy the document (with/__exit__ will clean this up if you forget)
        bc.destroy_matcher(matcher) #destroy the spec (with/__exit__ will clean this up if you forget)
```



Inset operator
==========

Adds an operator not found in MongoDB ($inset).

allows you to sepecify a set (hashtable) of strings to compare against.

allows ONLY strings in the set list.  The compare will silently ignore any non-strings in either the spec or document.

$inset uses a set/hashtable to perform O(1) lookups compared to $in which does a standard compare.


``` python
    from bsonsearch import bsoncompare
    import bson
    bc = bsoncompare()
    # O(1) lookups in this list
    # length of this spec list (converted to set) does not impact lookup time
    spec = {"a":{"$inset":["test1", "test2"]}} #ideal for list of many (>100) things.
    doc  = {"a":"test2"}
    matcher = bc.generate_matcher(b) #list->set length impacts time it takes to convert to set during this call.
    print [bc.match(matcher, x) for x in c]
    bc.destroy_matcher(matcher)
```

YARA within SPEC
==================

bsonsearch supports the use of compiled yara signature using libyara.


libyara-devel is required at compile time and libbsonsearch must be compiled with -lyara and passed -DWITH_YARA macro at compile time to enable.



``` python
    import bsonsearch
    import bson
    import yara
    import io
    rule = '''
    rule example
    {
        strings:
            $c = {6c 6f 20 77 6f  72} //"lo wor"
        condition:
            any of them
    }

    '''
    compiled_rule = yara.compile(source=rule)
    compiled_binary_rule = io.BytesIO()
    compiled_rule.save(file=compiled_binary_rule)

    bc = bsonsearch.bsoncompare()
    bc.bc.bsonsearch_startup() # handles yara initialization
    doc = {'a': "hello world"}
    doc_id = bc.generate_doc(doc)

    spec = {"a": {"$yara":bson.Binary(compiled_binary_rule.getvalue())}}
    matcher = bc.generate_matcher(spec)

    print bc.match_doc(matcher, doc_id) #this will segfault if signature invalid or no yara support in libbsonsearch
    bc.destroy_doc(doc_id) #destroy the document
    bc.destroy_matcher(matcher) #destroy the spec
    bc.bc.bsonsearch_shutdown() # handles yara shutdown
    >>> True
```

if you have yara-python installed on the system, you can use the bsoncompare helper functions


``` python
    import bsonsearch
    import bson
    rule = '''
    rule example
    {
        strings:
            $c = {6c 6f 20 77 6f 72} //"lo wor"
        condition:
            any of them
    }
    '''
    bc = bsonsearch.bsoncompare()
    bc.bc.bsonsearch_startup() # handles yara initialization
    doc = {'msg': "hello world"}
    doc_id = bc.generate_doc(doc)

    spec = {"msg": bsonsearch.YARA_COMPILE_STR(rule)} #see __init__.py for yara helpers
    matcher = bc.generate_matcher(spec)

    print bc.match_doc(matcher, doc_id) #this will segfault if signature invalid or no yara support in libbsonsearch
    bc.destroy_doc(doc_id) #destroy the document
    bc.destroy_matcher(matcher) #destroy the spec
    bc.bc.bsonsearch_shutdown() # handles yara shutdown
    >>> True
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
$project command
==================
Similar support to the $project aggregation operator (https://docs.mongodb.org/manual/reference/operator/aggregation/project/)

I've often wanted to take a hodgepodge of a json/bson document and strip out everything I'm not interested in.

Of course, things get really complicated with nested documents and recursing sub documents are slow.

the bsoncomare c library supports a $project operator which projects only the requested fields into the output dict.

In this example, grab a basic key (given using dot notation) and omit everything else.  If the value of they key is int(1)/bool(True), the resultant key after the projection will be the dot notation key used to define the projection.

Output will always take the form of {key:[value1, value2,..., valueN]}



``` python
    from bsonsearch import bsoncompare
    bc = bsoncompare()
    doc = {"a":{"aa":["ii", 33]}, "b":"b"}
    doc_id = bc.generate_doc(doc)
    spec = {"$project":{"a.aa":1}} #1/True as the value to the key you wish to project
    query = bc.generate_matcher(spec)
    bc.project_bson(query, doc_id) #returns a dict object
    >>> {u'a.aa': [u'ii', 33]}

```

Sometimes you may find the dot notation (.) in a key is not supported (if you were going to pass this dict back into MongoDB or this library)

You may use a type(basestring) as the value of the $project key (a_aa will be the key in the returned dict)

``` python
    from bsonsearch import bsoncompare
    bc = bsoncompare()
    doc = {"a":{"aa":["ii", 33]}, "b":"b"}
    doc_id = bc.generate_doc(doc)
    spec = {"$project":{"a.aa":"a_aa"}} #<---- <str> as the value to the key you wish to project into
    query = bc.generate_matcher(spec)
    bc.project_bson(query, doc_id) #returns a dict object
    >>> {u'a_aa': [u'ii', 33]}

```

sometimes you're interested in information that is inside multiple keys, but for the sake of presentation to a user, would like to condense multiple keys/namespaces down into one for clarity

enter, the project-field-foundin-multiple_namespaces

notice, the key "contact_info" is used in the out (contrast with previous command where the value became the output key if present)

``` python
    from bsonsearch import bsoncompare
    bc = bsoncompare()
    player = {"real_name":"Dick Cheney",
              "email":"theRealDeal@example.com",
              "alias":"RightInTheKisser",
              "skype":"powpowpow",
              "twitter":"@bestofthebestofthebest",
              "dob":datetime(year=1941, month=1, day=30)}
    doc_id = bc.generate_doc(player)
    aliases = {"$project":{"contact_info":{"$foundin":["email",
                                                      "skype",
                                                      "twitter"]}}}
    alias_matcher = bc.generate_matcher(aliases)
    bc.project_bson(alias_matcher, doc_id)
    >>>{u'contact_info': [u'theRealDeal@example.com', u'powpowpow', u'@bestofthebestofthebest']}
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


