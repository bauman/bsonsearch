import bson
import ctypes
from ctypes import c_void_p, c_double, c_char_p, c_uint, c_bool, py_object
from ctypes import cast as c_cast
from ctypes.util import find_library
from hashlib import md5
from bson.json_util import loads
try:
    import bsonhelper
except ImportError:
    pass

try:
    import yara
    import io
    def YARA_DICT(compiled_rule):
        compiled_binary_rule = io.BytesIO()
        compiled_rule.save(file=compiled_binary_rule)
        return {"$yara":bson.Binary(compiled_binary_rule.getvalue())}
    def YARA_LOAD(fn):
        compiled_rule = yara.load(fn)
        return YARA_DICT(compiled_rule)
    def YARA_COMPILE(fn):
        compiled_rule = yara.compile(fn)
        return YARA_DICT(compiled_rule)
    def YARA_COMPILE_STR(source):
        compiled_rule = yara.compile(source=source)
        return YARA_DICT(compiled_rule)
except ImportError:
    yara = None


class bsoncompare(object):
    def __init__(self):
        self.bc = ctypes.CDLL(find_library("bsoncompare")) #libbsoncompare rpm.
        #utils
        self.bc.bsonsearch_haversine_distance.argtypes = [c_double, c_double, c_double, c_double]
        self.bc.bsonsearch_haversine_distance.restype = c_double
        self.bc.bsonsearch_haversine_distance_degrees.argtypes = [c_double, c_double, c_double, c_double]
        self.bc.bsonsearch_haversine_distance_degrees.restype = c_double
        self.bc.bsonsearch_yara_gte1_hit_raw.argtypes = [c_void_p, c_char_p, c_uint]
        self.bc.bsonsearch_yara_gte1_hit_raw.restype = c_bool
        self.bc.bsonsearch_free_project_str.argtypes = [c_void_p]
        self.bc.bsonsearch_free_project_str.restype = c_uint

        self.bc.bsonsearch_project_json.argtypes = [c_void_p, c_void_p]
        self.bc.bsonsearch_project_json.restype = c_void_p

        self.bc.bsonsearch_project_bson.argtypes = [c_void_p, c_void_p]
        self.bc.bsonsearch_project_bson.restype = c_void_p

        #standard
        self.bc.bsonsearch_startup.argtypes = []
        self.bc.bsonsearch_startup.restype = c_uint
        self.bc.bsonsearch_shutdown.argtypes = []
        self.bc.bsonsearch_shutdown.restype = c_uint
        self.bc.generate_matcher.argtypes = [c_void_p, c_uint]
        self.bc.generate_matcher.restype = c_void_p
        self.bc.matcher_destroy.argtypes = [c_void_p]
        self.bc.matcher_destroy.restype = c_uint
        self.bc.generate_doc.argtypes = [c_void_p, c_uint]
        self.bc.generate_doc.restype = c_void_p
        self.bc.doc_destroy.argtypes = [c_void_p]
        self.bc.doc_destroy.restype = c_uint
        self.bc.regex_destroy.argtypes = []
        self.bc.regex_destroy.restype = c_uint

        self.bc.matcher_compare.argtypes = [c_void_p, c_void_p,  c_uint]
        self.bc.matcher_compare.restype = c_uint

        self.bc.matcher_compare_doc.argtypes = [c_void_p, c_void_p]
        self.bc.matcher_compare_doc.restype = c_uint

        #self.bc.bsonsearch_bson_as_pystring.argtypes = [c_void_p]
        #self.bc.bsonsearch_bson_as_pystring.restype = py_object






        self.matchers = {} #keys = string, value = c-pointers
        self.docs = {} #keys = string, value = c-pointers

    def __enter__(self):
        return self.startup()

    def __exit__(self, exc_type, exc_value, traceback):
        response = self.shutdown()
        assert(response == 0)
        return

    def startup(self):
        response = self.bc.bsonsearch_startup()
        assert(response == 0)
        return self


    def shutdown(self):
        self.destroy_regexes()
        self.destroy_matcher(self.matchers)
        self.destroy_doc(self.docs)
        response = self.bc.bsonsearch_shutdown()
        return response

    def destroy_matcher(self, matcher_id):
        if isinstance(matcher_id, list) or isinstance(matcher_id, dict):
            matcher_ids = [matcher_id_ for matcher_id_ in matcher_id]
            [self.destroy_matcher(matcher_ids_) for matcher_ids_ in matcher_ids]
        elif isinstance(matcher_id, basestring):
            if matcher_id not in self.matchers:
                return True #let caller believe it's been deleted.
            if matcher_id in self.matchers:
                self.bc.matcher_destroy(self.matchers[matcher_id])
                del(self.matchers[matcher_id])

    def generate_matcher(self, spec):
        if isinstance(spec, dict):
            encoded_spec = bson.BSON.encode(spec)
        elif isinstance(spec, basestring):
            encoded_spec = spec
        elif spec is None:
            raise ValueError("SPEC must not be empty")
        else:
            raise ValueError("SPEC must be instance of DICT or bson string")
        matcher_id = md5(encoded_spec).hexdigest()
        if matcher_id not in self.matchers:
            self.matchers[matcher_id] = self.bc.generate_matcher(encoded_spec,
                                                                 len(encoded_spec))
        return matcher_id



    def destroy_doc(self, doc_id):
        if isinstance(doc_id, list) or isinstance(doc_id, dict):
            doc_ids = [doc_id_ for doc_id_ in doc_id]
            [self.destroy_doc(doc_ids_) for doc_ids_ in doc_ids]
        elif isinstance(doc_id, basestring):
            if doc_id not in self.docs:
                return True
            if doc_id in self.docs:
                self.bc.doc_destroy(self.docs[doc_id])
                del(self.docs[doc_id])

    def destroy_regexes(self,):
        self.bc.regex_destroy()

    def generate_doc(self, doc):
        if isinstance(doc, dict):
            encoded_doc = bson.BSON.encode(doc)
        elif isinstance(doc, basestring):
            encoded_doc = doc
        elif doc is None:
            raise ValueError("DOC must not be empty")
        else:
            raise ValueError("DOC must be instance of DICT or bson string")
        doc_id = md5(encoded_doc).hexdigest()
        if doc_id not in self.docs:
            self.docs[doc_id] = self.bc.generate_doc(encoded_doc,
                                                     len(encoded_doc))
        return doc_id


    def match_doc(self, matcher_id, doc_id):
        matcher  = self.matchers[matcher_id] #pointer
        document = self.docs[doc_id] #pointer
        ismatch = self.bc.matcher_compare_doc(matcher, document)
        return bool(ismatch)


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



    def project_bson(self, matcher_id, doc_id):
        '''

        :param matcher_id: <basestring> id for the matcher pointer
        :param doc_id:
        :return: <basestring> bson representation of the requested projection
        '''
        matcher  = self.matchers[matcher_id] #pointer
        document = self.docs[doc_id] #pointer
        projection_pointer = self.bc.bsonsearch_project_bson(matcher, document) #void_p
        bson_str = bsonhelper.bson_as_string(projection_pointer)
        return bson_str

    def project_bson_as_dict(self, matcher_id, doc_id):
        return bson.BSON.decode(bson.BSON(self.project_bson(matcher_id, doc_id)))

    def project_json_as_dict(self, matcher_id, doc_id, loads_function=None):
        if callable(loads_function):
            return loads_function(self.project_json(matcher_id, doc_id))
        return loads(self.project_json(matcher_id, doc_id))

    def project_json(self, matcher_id, doc_id):
        '''
        This function is used to project JSON fields into a CSV style object.

        :param matcher_id:
        :param doc_id:
        :return: <basestring> json representation of the requested projection
        '''
        matcher  = self.matchers[matcher_id] #pointer
        document = self.docs[doc_id] #pointer
        projection_pointer = self.bc.bsonsearch_project_json(matcher, document) #void_p
        projection_str_p = c_cast(projection_pointer, c_char_p)
        projection_value = projection_str_p.value
        self.bc.bsonsearch_free_project_str(projection_pointer)
        return projection_value

    def explode_namespace(self, prefix_len, namespace, doc_id):
        '''
        DEPRECATED!!  Start throwing warning soon.

        :param prefix_len:
        :param namespace:
        :param doc_id:
        :return:
        '''
        try:
            document = self.docs[doc_id]
        except KeyError:
            raise KeyError("destroy_doc was already called on document")
        curpos = prefix_len
        curpos = namespace.find(".",curpos+1)
        if curpos > 0:
            at_end = False
            current_ns = namespace[:curpos]
        else:
            #mongo-c-driver handles items and lists at the end of namespace
            #yield here and let the driver sort it out.
            current_ns =  namespace
            yield current_ns
            raise StopIteration
        #mongo c driver cannot figure out lists embedded in the namespace
        array_len = self.bc.get_array_len(document, current_ns, len(current_ns))
        if not array_len:
            #not a list, move on to the next item in the namespace
            next_ns_prefix = "%s" %(current_ns)
            next_ns = "%s%s" %(next_ns_prefix, namespace[curpos:])
            for yield_ns in self.explode_namespace(len(next_ns_prefix), next_ns, doc_id):
                yield yield_ns
        for x in xrange(array_len):
            #embedded list within the namespace
            #need generate all the prefixes because libbson bson_iter_find_descendant needs it.
            next_ns_prefix = "%s.%s" %(current_ns,x)
            if at_end:
                next_ns = "%s%s" %(next_ns_prefix, "")
            else:
                next_ns = "%s%s" %(next_ns_prefix, namespace[curpos:])
            for yield_ns in self.explode_namespace(len(next_ns_prefix), next_ns, doc_id):
                yield yield_ns
        return

    def convert_to_or(self, ns_list, value):
        query_list = []
        for ns in ns_list:
            query_list.append({ns:value})
        if query_list:
            query = { "$or": query_list }
        else:
            query = None
        return query

    def convert_to_and(self, spec, doc_id):
        and_list = []
        query = {"$and":and_list}
        for key, value in spec.items():
            ns_list = [x for x in self.explode_namespace(0, key, doc_id)]
            query_or = self.convert_to_or(ns_list, value)
            if query_or:
                and_list.append(query_or)
        if not and_list:
            query=None
        return query


#list_of_tuples = unroll("", [], highly_embedded_dict)
def unroll(current_key, output_map, entry, keys_to_append=None):
    '''

    :param current_key:
    :param output_map:
    :param entry:
    :param keys_to_append:
    :return:
    '''
    def unroll_dict(current_key, output_map, entry, keys_to_append=None):
        for key, value in entry.items():
            unroll(".".join([current_key, key]).lstrip("."),
                   output_map,
                   value,
                   keys_to_append=keys_to_append)

    def unroll_list(current_key, output_map, entry, keys_to_append=None):
        for item in entry:
            unroll(current_key,
                   output_map,
                   item,
                   keys_to_append=keys_to_append)

    if isinstance(entry, dict):
        unroll_dict(current_key, output_map, entry, keys_to_append=keys_to_append)
    elif isinstance(entry, list):
        unroll_list(current_key, output_map, entry, keys_to_append=keys_to_append)
    else: #not iterable
        if not keys_to_append or current_key in keys_to_append:
            output_map.append((current_key, entry))
    return output_map

