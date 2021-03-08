"""
thin ctypes wrapper to libbsoncompare

can work with either the full version or the lite version
"""
import ctypes
from ctypes import c_void_p, c_double, c_char_p, c_uint, c_bool, py_object
from ctypes import cast as c_cast
from ctypes.util import find_library
from hashlib import md5
import bson
from bson.json_util import loads
from bson.binary import Binary

hash_fn = md5

try:
    from bsonsearch import bsonhelper
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

    def YARA_SOURCE_STR(source):
        assert isinstance(source, bytes)
        return {"$yara": {"source": source}}

except ImportError:
    yara = None


try:
    import discodb
    DISCO_VALUE_ONLY = "$valueonly"
    DISCO_KEY_EXISTS = "$keyexists"
    DISCO_VALUE_IS = "$valueis"
    DISCO_CNF_QUERY = "$Q"
    DISCO_DDB_FILE = "$ddb"

    def DISCO_VALUE_IS_CONFIG(**kwargs):
        spec = {
            kwargs["ns"]: {
                "$module": {
                    "name": "disco",
                    "config": {
                        DISCO_VALUE_IS: kwargs['value'],
                        DISCO_DDB_FILE: kwargs["ddb"]
                    }
                }
            }
        }
        return spec

    def DISCO_VALUE_ONLY_CONFIG(**kwargs):
        spec = {
            kwargs["ns"]: {
                "$module": {
                    "name": "disco",
                    "config": {
                        DISCO_VALUE_ONLY: kwargs['value'],
                        DISCO_DDB_FILE: kwargs["ddb"]
                    }
                }
            }
        }
        return spec

    def DISCO_KEY_EXISTS_CONFIG(**kwargs):
        spec = {
            kwargs["ns"]: {
                "$module": {
                    "name": "disco",
                    "config": {
                        DISCO_KEY_EXISTS: 0,
                        DISCO_DDB_FILE: kwargs["ddb"]
                    }
                }
            }
        }
        return spec


    def DISCO_QUERY_CONFIG(**kwargs):
        config = bson.son.SON()
        config['$ddb'] = kwargs["ddb"]
        config[DISCO_CNF_QUERY] = discodb.Q.parse(kwargs["cnf"]).deploy()
        config['precache'] = kwargs.get("precache", False)
        spec = {
            kwargs["ns"]: {
                "$module": {
                    "name": "disco",
                    "config": config
                }
            }
        }
        return spec

    def DISCODB_CNF_TO_DICT(cnf_string):
        return discodb.Q.parse(cnf_string).deploy()
except ImportError:
    discodb = None


try:
    import IPy
    import struct
    def pack_ip(ip_string):
        '''

        :param ip_string: String representation of ipv4 or ipv6 address ("127.0.0.1" or "::1"
        :return: Binary encapsulated and packed 16 byte integer
        '''
        ip = IPy.IP(ip_string)
        ip_int = ip.int()
        ip_bin = Binary(struct.pack(">QQ", ip_int/(2**64), ip_int%(2**64)), 0x80+ip.version())
        return ip_bin

    def ip_inrange_query(namespace, ip_string, netmask):
        """
        builds the $inIPRange
        :param namespace:
        :param ip_string:
        :param netmask:
        :return:
        """
        assert namespace
        ip_bin = pack_ip(ip_string)
        nm_bin = pack_ip(netmask)
        assert ip_bin.subtype == nm_bin.subtype
        return {namespace: {"$inIPrange": [ip_bin, nm_bin]}}

    def ip_inrangeset_query(namespace, list_of_ip_netmask_tuples):
        """

        :param namespace:
        :param list_of_ip_netmask_tuples: [(ip1,mask1), (ip2,mask2)...]
        :return:dict
        """
        setlist = []
        assert namespace
        for ip_string, netmask in list_of_ip_netmask_tuples:
            ip_bin = pack_ip(ip_string)
            nm_bin = pack_ip(netmask)
            setlist.append([ip_bin, nm_bin])
        assert ip_bin.subtype == nm_bin.subtype
        return {namespace: {"$inIPrangeset": setlist}}
except ImportError:
    pack_ip = None


class bsoncompare:
    """
    main class that wraps the libbsoncompare library

    this should be thread safe to generate matchers and pass the matcher ids to other threads.
    """
    def __init__(self):
        self.CAPABILITY = 0
        lib_path = find_library("bsoncompare")
        if not lib_path:
            lib_path = "libbsoncompare.so"
        try:
            self.bc = ctypes.CDLL(lib_path)  # libbsoncompare rpm.
            self.CAPABILITY = 15
        except OSError:
            lib_path = find_library("bsoncomparelite")
            if not lib_path:
                lib_path = "libbsoncomparelite.so"
            self.bc = ctypes.CDLL(lib_path)  # libbsoncompare rpm.

        self.bc.bsonsearch_capability.argtypes = []
        self.bc.bsonsearch_capability.restype = c_uint

        # standard
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

        self.bc.matcher_compare.argtypes = [c_void_p, c_void_p, c_uint]
        self.bc.matcher_compare.restype = c_uint

        self.bc.matcher_compare_doc.argtypes = [c_void_p, c_void_p]
        self.bc.matcher_compare_doc.restype = c_uint

        self.CAPABILITY = self.bc.bsonsearch_capability()
        if self.CAPABILITY > 8:
            # utils
            self.bc.bsonsearch_haversine_distance.argtypes = [
                c_double, c_double, c_double, c_double
            ]
            self.bc.bsonsearch_haversine_distance.restype = c_double
            self.bc.bsonsearch_haversine_distance_degrees.argtypes = [
                c_double, c_double, c_double, c_double
            ]
            self.bc.bsonsearch_haversine_distance_degrees.restype = c_double
            self.bc.bsonsearch_get_crossarc_degrees.argtypes = [
                c_double, c_double, c_double, c_double, c_double, c_double
            ]
            self.bc.bsonsearch_get_crossarc_degrees.restype = c_double

            self.bc.bsonsearch_free_project_str.argtypes = [c_void_p]
            self.bc.bsonsearch_free_project_str.restype = c_uint

            self.bc.bsonsearch_project_json.argtypes = [c_void_p, c_void_p]
            self.bc.bsonsearch_project_json.restype = c_void_p

            self.bc.bsonsearch_project_bson.argtypes = [c_void_p, c_void_p]
            self.bc.bsonsearch_project_bson.restype = c_void_p

            try:
                self.bc.bsonsearch_yara_gte1_hit_raw.argtypes = [
                    c_void_p, c_char_p, c_uint
                ]
                self.bc.bsonsearch_yara_gte1_hit_raw.restype = c_bool
            except AttributeError:
                pass

        self.matchers = {}  # keys = string, value = c-pointers
        self.docs = {}  # keys = string, value = c-pointers

    def __enter__(self):
        return self.startup()

    def __exit__(self, exc_type, exc_value, traceback):
        self.shutdown()

    def startup(self):
        """
        initializes the libbsoncompare library
        which initializes all linked libraries
        should be called once by the main thread at startup

        :return:
        """
        self.bc.bsonsearch_startup()
        return self

    def shutdown(self):
        """
        finalizes the libbsoncompare library
        which finalizes all linked libraries
        should be called once by the main thread at the end
        :return:
        """
        self.destroy_regexes()
        self.destroy_matcher(self.matchers)
        self.destroy_doc(self.docs)
        response = self.bc.bsonsearch_shutdown()
        return response

    def destroy_matcher(self, matcher_id):
        """
        free the underlying bsoncompare memory allocated
        to hold this matcher
        :param matcher_id:
        :return:
        """
        if isinstance(matcher_id, (list, dict)):
            # duplicate the list because can't delete while iterating
            matcher_ids = [matcher_id_ for matcher_id_ in matcher_id]
            for matcher_ids_ in matcher_ids:
                self.destroy_matcher(matcher_ids_)
        elif isinstance(matcher_id, (str, bytes)):
            if matcher_id not in self.matchers:
                return True  # let caller believe it's been deleted.
            if matcher_id in self.matchers:
                if self.matchers[matcher_id]:
                    self.bc.matcher_destroy(self.matchers[matcher_id])
                del self.matchers[matcher_id]
                return True

    def generate_matcher(self, spec):
        """
        creates a matcher object.
        Matchers are thread safe internal representations of the spec.
        Generate the matcher from spec and can use the reference to the matcher in parallel threads.

        :param spec:
        :return:
        """
        if isinstance(spec, dict):
            encoded_spec = bson.BSON.encode(spec)
        elif isinstance(spec, bytes):
            encoded_spec = spec
        elif spec is None:
            raise ValueError("SPEC must not be empty")
        else:
            raise ValueError("SPEC must be instance of DICT or bson string")
        matcher_id = hash_fn(encoded_spec).hexdigest()
        if matcher_id not in self.matchers:
            self.matchers[matcher_id] = self.bc.generate_matcher(
                encoded_spec,
                len(encoded_spec)
            )
            if not self.matchers[matcher_id]:
                del self.matchers[matcher_id]
                raise ValueError("Invalid SPEC")
        return matcher_id

    def destroy_doc(self, doc_id):
        """

        :param doc_id:
        :return:
        """
        if isinstance(doc_id, (list, dict)):
            # duplicate the list because can't delete while iterating
            doc_ids = [doc_id_ for doc_id_ in doc_id]
            for doc_ids_ in doc_ids:
                self.destroy_doc(doc_ids_)
        elif isinstance(doc_id, bytes):
            if doc_id not in self.docs:
                return True
            if doc_id in self.docs:
                if self.docs[doc_id]:
                    self.bc.doc_destroy(self.docs[doc_id])
                del self.docs[doc_id]
                return True

    def destroy_regexes(self,):
        """
        regular expressions are cached internally
        and may need freed every so often
        if they are not reused

        :return:
        """
        self.bc.regex_destroy()

    def generate_doc(self, doc):
        """
        creates the internal representation of a document

        :param doc:
        :return:
        """
        if isinstance(doc, dict):
            encoded_doc = bson.BSON.encode(doc)
        elif isinstance(doc, bytes):
            encoded_doc = doc
        elif doc is None:
            raise ValueError("DOC must not be empty")
        else:
            raise ValueError("DOC must be instance of DICT or bson string")
        doc_id = hash_fn(encoded_doc).hexdigest()
        if doc_id not in self.docs:
            self.docs[doc_id] = self.bc.generate_doc(
                encoded_doc,
                len(encoded_doc)
            )
            if not self.docs[doc_id]:
                del self.docs[doc_id]
                raise ValueError("Invalid Document")
        return doc_id

    def match_doc(self, matcher_id, doc_id):
        """
        Takes a matcher created by generate_matcher
              and a doc created by generate_doc

              returns whether the document matches the spec

        :param matcher_id:
        :param doc_id:
        :return: bool
        """
        matcher = self.matchers[matcher_id]  # pointer
        document = self.docs[doc_id]  # pointer
        if matcher and document:
            ismatch = self.bc.matcher_compare_doc(matcher, document)
        else:
            raise ValueError("Unable to retrieve handles")
        return bool(ismatch)

    def match(self, matcher_id, document):
        """
        Takes a matcher created by generate_matcher
              and a doc that is a dict or bson.binary.Binary

              returns whether the document matches the spec
        :param matcher_id:
        :param document:
        :return:
        """
        if isinstance(matcher_id, dict):
            matcher_id = self.generate_matcher(matcher_id)
        matcher = self.matchers[matcher_id]
        if isinstance(document, dict):
            encoded_document = bson.BSON.encode(document)
        elif isinstance(document, bytes):
            encoded_document = document
        else:
            raise ValueError
        if matcher and encoded_document:
            ismatch = self.bc.matcher_compare(
                matcher,
                encoded_document,
                len(encoded_document)
            )
        else:
            raise ValueError("Unable to retrieve handles")
        return bool(ismatch)

    def project_bson(self, matcher_id, doc_id):
        """
        creates a new bytes object representing one document

        projected based on the matcher

        :param matcher_id: <bytes> id for the matcher pointer
        :param doc_id:
        :return: <bytes> bson representation of the requested projection
        """
        matcher = self.matchers[matcher_id]  # pointer
        document = self.docs[doc_id]  # pointer
        if matcher and document:
            projection_pointer = self.bc.bsonsearch_project_bson(matcher, document)  # void_p
            if projection_pointer:
                bson_str = bsonhelper.bson_as_string(projection_pointer)
            else:
                raise ValueError("Projection Failed")
        else:
            raise ValueError("Unable to retrieve handles")
        return bson_str

    def project_bson_as_dict(self, matcher_id, doc_id):
        """
        wraps the project_bson call and
        converts the document to a native python dict
        :param matcher_id:
        :param doc_id:
        :return: <dict>
        """
        return bson.BSON.decode(bson.BSON(self.project_bson(matcher_id, doc_id)))

    def project_json_as_dict(self, matcher_id, doc_id, loads_function=None):
        if callable(loads_function):
            return loads_function(self.project_json(matcher_id, doc_id))
        return loads(self.project_json(matcher_id, doc_id))

    def project_json(self, matcher_id, doc_id):
        """
        This function is used to project JSON fields into a CSV style object.

        :param matcher_id:
        :param doc_id:
        :return: <bytes> json representation of the requested projection
        """
        matcher = self.matchers[matcher_id]  # pointer
        document = self.docs[doc_id]  # pointer
        projection_pointer = self.bc.bsonsearch_project_json(matcher, document)  # void_p
        projection_str_p = c_cast(projection_pointer, c_char_p)
        projection_value = projection_str_p.value
        self.bc.bsonsearch_free_project_str(projection_pointer)
        return projection_value

    def explode_namespace(self, prefix_len, namespace, doc_id):
        """
        DEPRECATED!!  Start throwing warning soon.

        :param prefix_len:
        :param namespace:
        :param doc_id:
        :return:
        """
        try:
            document = self.docs[doc_id]
        except KeyError:
            raise KeyError("destroy_doc was already called on document")
        curpos = prefix_len
        curpos = namespace.find(".", curpos+1)
        if curpos > 0:
            at_end = False
            current_ns = namespace[:curpos]
        else:
            # mongo-c-driver handles items and lists at the end of namespace
            # yield here and let the driver sort it out.
            current_ns = namespace
            yield current_ns
            return
        # mongo c driver cannot figure out lists embedded in the namespace
        array_len = self.bc.get_array_len(document, current_ns, len(current_ns))
        if not array_len:
            # not a list, move on to the next item in the namespace
            next_ns_prefix = "%s" %(current_ns)
            next_ns = "%s%s" %(next_ns_prefix, namespace[curpos:])
            for yield_ns in self.explode_namespace(len(next_ns_prefix), next_ns, doc_id):
                yield yield_ns
        for _ in range(array_len):
            # embedded list within the namespace
            # need generate all the prefixes because libbson bson_iter_find_descendant needs it.
            next_ns_prefix = "%s.%s" %(current_ns, _)
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
            query_list.append({ns: value})
        if query_list:
            query = {"$or": query_list}
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
            query = None
        return query


# list_of_tuples = unroll("", [], highly_embedded_dict)
def unroll(current_key, output_map, entry, keys_to_append=None):
    """

    :param current_key:
    :param output_map:
    :param entry:
    :param keys_to_append:
    :return:
    """
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
    else:  # not iterable
        if not keys_to_append or current_key in keys_to_append:
            output_map.append((current_key, entry))
    return output_map
