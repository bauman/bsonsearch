bsonsearch
==========

shared object to perform mongodb-like queries against raw bson rather than through a mongod




compile
========

this requires 

c libbson (https://github.com/mongodb/libbson)
mongo-c-driver (https://github.com/mongodb/mongo-c-driver)


```
    gcc $(pkg-config --cflags --libs libbson-1.0 libmongoc-1.0) -shared -o bsoncompare.so -fPIC bsoncompare.c
```


Usage
==========

The spec parameter supports all query operators (http://docs.mongodb.org/manual/reference/operator/query/) supported by the mongo-c-driver

Currently, that includes $in, $nin, $eq, $neq, $gt, $gte, $lt, and $lte. (See full documentation http://api.mongodb.org/c/current/mongoc_matcher_t.html)



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


detailed example


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