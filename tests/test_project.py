'''
this file performs a leak check on project bson function.
python and c_char_p seem to have trouble freeing memory.
had to force the free to not leak.
'''
import bsonsearch
import json
from time import sleep

doc = bsonsearch.Document(json.dumps(
    {"a":{"aa":[2, 33]}, "b":"b"}
))

spec = bsonsearch.Matcher(json.dumps(
    {"$project":{"a.aa":1}}
))

i = 0
max = 100000
TEST_PROJECT = True

##test the projection
while TEST_PROJECT:
    i +=1
    if i>max:
        i = 0
        print(max)
        # break
    result = spec.project_json(doc)

del spec
del doc

