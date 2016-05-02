'''
this file performs a leak check on project bson function.
python and c_char_p seem to have trouble freeing memory.
had to force the free to not leak.
'''
from bsonsearch import bsoncompare
bc = bsoncompare()
doc = {"a":{"aa":[2, 33]}, "b":"b"}
doc_id = bc.generate_doc(doc)
spec = {"$project":{"a.aa":1}}
query = bc.generate_matcher(spec)


i = 0
max = 100000
TEST_DESTROY = True
TEST_PROJECT = False
assert (TEST_DESTROY != TEST_PROJECT)

AS_DICT=False

##test the projection
while TEST_PROJECT:
    i +=1
    if i>max:
        i = 0
        print max
    result = bc.project_bson(query, doc_id) #should not memory leak the str



#test the destruction
while TEST_DESTROY:
    i +=1
    if i>max:
        i = 0
        print max
    doc = {"a":{"aa":[2, 33]}, "b":"b"}
    doc_id = bc.generate_doc(doc)
    spec = {"$project":{"a.aa":1}}
    query = bc.generate_matcher(spec)
    result = bc.project_json(query, doc_id) #should not memory leak the str
    assert(result == '{ "a.aa" : [ 2, 33 ] }' )
    result = bc.project_bson_as_dict(query, doc_id) #should not memory leak the str
    assert(len(result['a.aa'])==2)
    bc.destroy_doc(doc_id)
    bc.destroy_matcher(query)

