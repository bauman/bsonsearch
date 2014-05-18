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
this example uses KeyValueBSONInput (https://github.com/bauman/python-bson-streaming)

raw bson file containing 7GB of metadata from the enron data set

This ammounts to nothing more than a full table scan through a single mongod but can be multiplexed across multiple bson files

The spec parameter supports all query operators (http://docs.mongodb.org/manual/reference/operator/query/) supported by the mongo-c-driver

``` python 
    import ctypes
    import bson
    from bsonstream import KeyValueBSONInput
    
    bc = ctypes.CDLL("/home/dan/bson/bsoncompare.so")
    
    spec = bson.BSON.encode({"len" : { "$gt": 4153937}})    
    matcher = bc.generate_matcher(spec, len(spec))

    f = open("/home/dan/enron/enron.bson", 'rb')
    stream = KeyValueBSONInput(fh=f, decode=False)
    for doc in stream:
        print bc.matcher_compare(matcher, doc, len(doc))
    f.close()
    bc.matcher_destroy(matcher)
```
