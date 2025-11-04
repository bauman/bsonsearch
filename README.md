[![Build Status](https://travis-ci.org/bauman/bsonsearch.svg?branch=py3)](https://travis-ci.org/bauman/bsonsearch)


License
================
This code is licensed under permissive Apache 2 and MIT license.

Linking to the official server matching engine falls under copyleft AGPL, this library provides minimal matching capability with permissive license


Code contributed by MongoDB, Inc is licesned Apache 2

Code contributed by bauman is licensed MIT

Code developed by Wm. Randolph Franklin is licensed BSD Simplified


Acknowledgement
===============

Many thanks to Christian Hergert and A. Jesse Jiryu Davis from MongoDB Inc for creating and maintaining the minimized matcher code from the mongo-c-driver.


This repo continues the effort for a minimally functional and complimentary bson matching engine.


The officially supported [server matching engine](https://github.com/mongodb/mongo/tree/master/src/mongo/db/matcher) will always be far superior option.




bsonsearch
==========

shared object to perform mongodb-like queries against raw bson rather than through a mongod

```shell
python -m pip install bsonsearch
```


# Features


These features are disabled in the lite version to reduce the memory footprint of each matcher object and streamline dependencies

Projection Operator ($project)
- WITH_PROJECTION macro at compile time
- WITH_UTILS macro (optional) provides useful utilities to assist

Conditional Operator ($cond)
- WITH_CONDITIONAL macro at compile time.

Text Search (string tokenization and comparison) requires:
- WITH_TEXT macro at compile time

Text Stemmer requires
- libstemmer (link at compile time, SO at runtime)
- libstemmer-devel (import at compile time)
- WITH_STEMMER macro at compile time

Text Spell Checking requirements (ASPELL library is licensed LGPL)
- aspell (link at compile time, SO at runtime)
- aspell-devel (import at compile time
- aspell-en (or language dictionary of choice package at runtime)
- WITH_ASPELL macro at compile time (requires WITH_TEXT)

YARA signature matching requirements:
- yara (link at compile time, SO at runtime)
- yara-devel (import at compile time)
- WITH_YARA macro at compile time


to perform the automated testing, download these files into /tmp
```bash
          wget http://pkgs.bauman.space/discodb/sample.ddb -O /tmp/sample.ddb;
          wget http://pkgs.bauman.space/discodb/animals.ddb -O /tmp/animals.ddb;
          wget http://pkgs.bauman.space/discodb/cjk.ddb -O /tmp/cjk.ddb;
          wget http://pkgs.bauman.space/discodb/myths.ddb -O /tmp/myths.ddb;
```


Usage
==========

The spec parameter supports a subset of MongoDB query operators (http://docs.mongodb.org/manual/reference/operator/query/) 

Currently, that includes $and, $or, $not, $in, $nin, $eq, $neq, $gt, $gte, $lt, $lte, and $near. (See full documentation http://api.mongodb.org/c/current/mongoc_matcher_t.html)

comparison value in spec can be utf8 string, int/long, regex, compiled yara (if compiled with yara support)



``` python
    import bsonsearch
    import json
    spec = bsonsearch.Matcher(json.dumps({"a":{"$gte":1}}))
    docs = [
        bsonsearch.Document(json.dumps({"a":0})),
        bsonsearch.Document(json.dumps({"a":1})),
        bsonsearch.Document(json.dumps({"a":2}))
    ]
    spec.match_doc(docs[0])  # False
    spec.match_doc(docs[1])  # True
    spec.match_doc(docs[2])  # True
```


Inset operator
==========

Adds an operator not found in MongoDB ($inset).

allows you to sepecify a set (hashtable) of strings to compare against.

allows ONLY strings in the set list.  The compare will silently ignore any non-strings in either the spec or document.

$inset uses a set/hashtable to perform O(1) lookups compared to $in which does a standard compare.


``` python
    import bsonsearch
    import json
    # O(1) lookups in this list
    # length of this spec list (converted to set) does not impact lookup time
    spec = bsonsearch.Matcher(json.dumps(
        {"a":{"$inset":["test1", "test2"]}} #ideal for list of many (>100) things.
    ))
    doc  = bsonsearch.Document(json.dumps(
        {"a":"test2"}
    ))
    spec.match_doc(doc)  # True
```

Regex within SPEC
==================

bsonsearch supports the use of compiled regex using libpcre.  
The only regex option allowed is re.IGNORECASE, and only that option. Adding other options seperately or in addition to ingnore case is undefined.

You MUST have the mongo-python-driver installed via `python -m pip install pymongo`


``` python
    import bsonsearch
    from bson import json_util
    import re

    doc = bsonsearch.Document(json_util.dumps(
        {'a': "hello world"}
    ))
    spec = bsonsearch.Matcher(json_util.dumps(
        {"a": re.compile("orl")}
    ))
    spec.match_doc(doc)  # True
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
    import bsonsearch
    import json
    
    doc = bsonsearch.Document(json.dumps(
        {"a":{"aa":["ii", 33]}, "b":"b"}
    ))
    spec = bsonsearch.Matcher(json.dumps(
        {"$project":{"a.aa":1}} #1/True as the value to the key you wish to project
    ))
    spec.project_json(doc) #returns a string/json object
    >>> '{ "a.aa" : [ "ii", 33 ] }'

```

Sometimes you may find the dot notation (.) in a key is not supported (if you were going to pass this dict back into MongoDB or this library)

You may use a type(basestring) as the value of the $project key (a_aa will be the key in the returned dict)

``` python

    import bsonsearch
    import json
    
    doc = bsonsearch.Document(json.dumps(
        {"a":{"aa":["ii", 33]}, "b":"b"}
    ))
    spec = bsonsearch.Matcher(json.dumps(
        {"$project":{"a.aa":"a_aa"}}  # <---- <str> as the value to the key you wish to project into
    ))
    spec.project_json(doc) #returns a string/json object
    >>> '{ "a_aa" : [ "ii", 33 ] }'

```

sometimes you're interested in information that is inside multiple keys, but for the sake of presentation to a user, would like to condense multiple keys/namespaces down into one for clarity

enter, the project-field-foundin-multiple_namespaces

notice, the key "contact_info" is used in the out (contrast with previous command where the value became the output key if present)

``` python
    import bsonsearch
    from bson import json_util
    from datetime import datetime
    player = {"real_name":"Dick Cheney",
              "email":"theRealDeal@example.com",
              "alias":"RightInTheKisser",
              "skype":"powpowpow",
              "twitter":"@bestofthebestofthebest",
              "dob":datetime(year=1941, month=1, day=30)}
    doc = bsonsearch.Document(json_util.dumps(
        player
    ))
    aliases = {"$project":{"contact_info":{"$foundin":["email",
                                                      "skype",
                                                      "twitter"]}}}
    spec = bsonsearch.Matcher(json_util.dumps(
        aliases
    ))
    spec.project_json(doc)
    >>>'{ "contact_info" : [ "theRealDeal@example.com", "powpowpow", "@bestofthebestofthebest" ] }'
```


$near example
==================
Makes a flat grid distance calculation.

Grid units are arbitrarily determined by the user.

Currently supports 2D or 3D calculations.
``` python
    import bsonsearch
    from bson.son import SON
    from bson import json_util
    import bson
    
    doc = bsonsearch.Document(json.dumps(
        {'pos':[200, 150]} # Legacy Point format.
    ))

    # The ordering fo these keys MATTER
    # Test your luck using a python doc, but I'd recommend on using SON
    #spec = {"pos":{"$maxDistance":100, "$near":[200, 151] }} # putting $maxDistance first will serialize $near correctly in CPython
    near_cmd = SON()
    near_cmd["$near"] = [200, 151]
    near_cmd["$maxDistance"] = 100
    spec_son = SON()
    spec_son['pos'] = near_cmd
    spec = bsonsearch.Matcher(json_util.dumps(
        spec_son
    ))
    
    spec.match_doc(doc)  # True
```

GeoNear $near example
==================
Uses GeoJSON (https://docs.mongodb.org/manual/reference/operator/query/near/#op._S_near)

Only supports point difference

Grid units are in meters

``` python
    import bsonsearch
    from bson.son import SON
    from bson import json_util
    import bson

    doc = bsonsearch.Document(json_util.dumps(
        {"loc": {
            "type": "Point" ,
            "coordinates": [ -61.08080307722216 , -9.057610600760512 ]
        }}
    ))
    #Test your luck using a python doc, but I'd recommend on using SON
    spec = bsonsearch.Matcher(json_util.dumps(
        {  "loc":{
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
    ))
    
    spec.match_doc(doc)
    >>>True
```

$cond (conditional) example
==================

See https://docs.mongodb.com/manual/reference/operator/aggregation/cond/ for more details

This library **ONLY** supports the dictionary style spec

    ```{ $cond: { if: <boolean-expression>, then: <true-case>, else: <false-case-> } }```

the true case and false case must all be matchers.  You cannot "hardcode" a true/false.

This will lead to multiple copies of the same matcher but keeps the complexity down.

In this spec:

```
    doc  = {"hello": "world"}
    spec = {"$cond":{"if": {"hello":{"$exists":true}}, "then": {"hello":"world"}, "else": {"hello":{"$exists":false}} }}}
```

the coditional statement says If the key "hello" exists, the value must be "world", if the key doesn't exist, the document still matches the spec.

This is useful for validating or checking optional fields.


```python
import bsonsearch
import json

doc1 = bsonsearch.Document(json.dumps({"hello": "world"}))
doc2 = bsonsearch.Document(json.dumps({"hello": "something else"}))
doc3 = bsonsearch.Document(json.dumps({"not-hello": "doesn't matter"}))

spec = bsonsearch.Matcher(json.dumps(
    {"$cond":
         {
             "if": {"hello":{"$exists":True}},  # if the hello key exists 
             "then": {"hello":"world"},   # then the value of hello MUST be true
             "else": {"hello":{"$exists":False}}   # else (doesn't exist) return any document which does NOT have the key
         }
    }    
))

spec.match_doc(doc1)  # True because it has `hello` and the value equals `world`
spec.match_doc(doc2)  # False because it has `hello` and the value DOES NOT equal `world`
spec.match_doc(doc3)  # True because it does NOT have the key `hello`
```


$unwind example
==================
Unwind lists and perform the compare against each unwound document individually

This is useful where {"doc":[{"a":1,"b":1}, {"a":2,"b":2}]}

the above document would (notionally) incorrectly match on {"doc.a":1, "doc.b":2}

There may be cases where the default matching is desired.

If you want to find only documents where {"a":1,"b":2} in the SAME subdocument, you need to unwind the subdocuments first using the $projection operator.

bsoncompare must be compiled using the WITH_PROJECTION macro for $unwind command to work.


``` python
    import bsonsearch
    import json
    doc = bsonsearch.Document(json.dumps(
        {"doc":[{"a":1,"b":1}, {"a":2,"b":2}]}
    ))
    incorrect_spec = bsonsearch.Matcher(json.dumps(
        {"doc.a":1, "doc.b":2}
    ))
    incorrect_spec.match_doc(doc)  # True  (but may want that to be False)

    
    correct_spec = bsonsearch.Matcher(json.dumps(
        {"$unwind": {"$project": {"doc": {"$foundin": ["doc"]}}, "$query": {"$and": [{"doc.a": 1}, {"doc.b": 2}]}}}
    ))
    correct_spec.match_doc(doc)  # False (because .a and .b didn't match in the same sub-doc)
    
```

Javascript Processor
====================

**NOTE: dukjs is a _module_ which requires a call to `Utils.startup()`**

bsonsearch is linked to the [Duktape](https://duktape.org) JS processor

given a document like

```javascript
{"hello": {"world":{"a": "variable"}}}
```

we can execute arbitrary javascript to determine if the document matches

In this simple example, create a function `matches` which loads some data and sees if the value of the `a` key is `variable` 

```javascript
function matches(data) {
    d = JSON.parse(data);
    return d.a == 'variable' ;
}
```



```python
import bsonsearch
from bson import json_util
from bson import Code

# call this exactly once when your program starts to configure the modules
utils = bsonsearch.Utils()
utils.startup()
# if you do not do this, the matcher will compile, but the comparison will quietly return false

# do NOT forget to wrap the code string with a bson.Code wrapper
js_func = Code("""
function matches(data) {
    d = JSON.parse(data);
    return d.a == 'variable' ;
}
""")

spec = bsonsearch.Matcher(json_util.dumps(
    {
        "hello.world": {  # the location we'd like to apply the JS
            "$module": {
                "name": "dukjs",  # activate the duktape module
                "config": {
                    "entrypoint": "matches",  # the name of the function YOU created
                    "code": js_func  # the bson.Code wrapped js function
                }
            }
        }
    }
))

doc = bsonsearch.Document(json_util.dumps(
    {"hello": {"world":{"a": "variable"}}}
))

spec.match_doc(doc)  # True

```

summation module
================
**NOTE: math/sum is a _module_ which requires a call to `Utils.startup()`**

This module intends to serve cases where multiple objects intend to give a running total of something.

In this case, say we want to find students that have earned AT LEAST 270 of a possible 300 points.

given a documents like

```javascript
{
    "student": "their-name",
    "grades": [
        {"name": "assignment1", "score":94},
        {"name": "assignment2", "score":89},
        {"name": "assignment2", "score":98},
    ]
}
```

we'd set up a matcher as follows


```python
import bsonsearch
from bson import json_util

# call this exactly once when your program starts to configure the modules
utils = bsonsearch.Utils()
utils.startup()
# if you do not do this, the matcher will compile, but the comparison will quietly return false

spec = bsonsearch.Matcher(json_util.dumps(
    {
        "grades.score":{  # the location of the data you want to inspect
            "$module":{
                "name":"sum",  # `sum` is the name of this module 
                "config":{
                    "$gte":270  # $gte, $gt, $lte, $lt, $eq, and $not are supported 
                }
            }
        }
    }
))
doc1 = bsonsearch.Document(json_util.dumps(
    {
        "student": "their-name",
        "grades": [
            {"name": "assignment1", "score":94},
            {"name": "assignment2", "score":89},
            {"name": "assignment3", "score":98},
        ]
    }
))

doc2 = bsonsearch.Document(json_util.dumps(
    {
        "student": "their-name",
        "grades": [
            {"name": "assignment1", "score":77},
            {"name": "assignment2", "score":81},
            {"name": "assignment3", "score":100},
        ]
    }
))

spec.match_doc(doc1)  # true
spec.match_doc(doc2)  # false

```



streaming example
==================

this example uses KeyValueBSONInput (https://github.com/bauman/python-bson-streaming)

raw bson file containing 7GB of metadata from the enron data set

This amounts to nothing more than a full table scan through a single mongod but can be multiplexed across multiple bson files with multiple processes using tools like xargs


``` python
    from bsonstream import KeyValueBSONInput
    from bsonsearch import bsoncompare
    import json
    
    spec = bsonsearch.Document(json.dumps(
        {"a":{"$gte":1}}
    ))
    f = open("/home/dan/enron/enron.bson", 'rb')
    stream = KeyValueBSONInput(fh=f, decode=False)
    for doc in stream:
        print(spec.match_doc(json.dumps(doc))
    f.close()
```
