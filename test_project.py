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
while True:
    result = bc.project_bson(query, doc_id) #should not memory leak the str

