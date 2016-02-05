
``` python
    import bsonsearch
    bc = bsonsearch.bsoncompare()
    import bson


    doc = {'pos':[200, 150]} #Legacy Point format.
    doc_id = bc.generate_doc(doc)
```

If you are seeing bsonsearch segfault with the $near command, it's almost
certainly because the command sequence is out of order.

Be incredibly careful generating $near queries.  Just like MongoDB, the "$near"
operator must be the first object in the bson.

Python dictionaries have no inherent organization.

You have 3 options,

1. Rely on undefined (but consistent) behavior. If you put the "near" after the
"maxDistance", it should serialize it correctly.

2. Use python OrderedDict to handle it. (slightly more challenging that what
would be obvious)

3. Build the BSON string yourself using SON.

Obviously you should do 2 or 3, but most people are just here looking for 1.
It's your data.

Here are 3 examples.

``` python
    #Option #1
    spec = {"pos":{"$maxDistance":100, "$near":[200, 151] }} #putting $maxDistance first will serialize $near correctly in CPython
    spec = bson.BSON.encode(spec)
    print spec.index("$near"), spec.index("$max")
    assert spec.index("$near") <  spec.index("$max")

    14 40
```



``` python
    #Option #2
    from collections import OrderedDict
    from bson.codec_options import CodecOptions
    near_cmd = OrderedDict([("$near", [200, 151]), ("$maxDistance", 100)])
    spec = {"pos":near_cmd}
    spec = bson.BSON.encode(spec, codec_options=CodecOptions(document_class=OrderedDict))
    print spec.index("$near"), spec.index("$max")
    assert spec.index("$near") <  spec.index("$max")
    
    14 40
```

``` python
    #option 3
    from bson.son import SON
    near_cmd = SON()
    near_cmd["$near"] = [200, 151]
    near_cmd["$maxDistance"] = 100
    spec = SON()
    spec['pos'] = near_cmd
    spec = bson.BSON.encode(spec)
    print spec.index("$near"), spec.index("$max")
    assert spec.index("$near") <  spec.index("$max")

    14 40
```
