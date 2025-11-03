"""
wrapper for bsonsearch
"""

from .matcher_module import Matcher, Document, Utils


try:
    import yara
    import io
    import bson

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
    import bson
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
    from bson.binary import Binary
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
