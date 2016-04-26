from bsonsearch import bsoncompare
bc = bsoncompare()
doc = {"a":{"aa":["ii", 33]}, "b":"b"}
doc_id = bc.generate_doc(doc)
spec = {"$project":{"a.aa":1}}
query = bc.generate_matcher(spec)
bc.project_bson(query, doc_id)
