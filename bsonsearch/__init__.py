import bson
import ctypes
from hashlib import md5

LIBRARY_LOC="/usr/lib64/bsoncompare.so"

class bsoncompare:
    def __init__(self):
        self.bc = ctypes.CDLL(LIBRARY_LOC)
        self.matchers = {}

    def destroy_matcher(self, matcher_id):
        if isinstance(matcher_id, list) or isinstance(matcher_id, list):
            [self.destroy_matcher(self.matchers[x]) for x in matcher_id]
        elif isinstance(matcher_id, basestring):
            self.bc.matcher_destroy(self.matchers[matcher_id])

    def generate_matcher(self, spec):
        if isinstance(spec, dict):
            encoded_spec = bson.BSON.encode(spec)
        elif isinstance(spec, basestring):
            encoded_spec = spec
        else:
            raise ValueError
        matcher_id = md5(encoded_spec).hexdigest()
        if matcher_id not in self.matchers:
            self.matchers[matcher_id] = self.bc.generate_matcher(encoded_spec,
                                                                 len(encoded_spec))
        return matcher_id

    def match(self, matcher_id, document):
        if isinstance(matcher_id, dict):
            matcher_id = self.generate_matcher(matcher_id)
        matcher = self.matchers[matcher_id]
        if isinstance(document, dict):
            encoded_document = bson.BSON.encode(document)
        elif isinstance(document, basestring):
            encoded_document = document
        else:
            raise ValueError
        ismatch = self.bc.matcher_compare(matcher,
                                          encoded_document,
                                          len(encoded_document))
        return bool(ismatch)