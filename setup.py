from setuptools import setup, Extension
from os import environ

include_dirs = [
    f"/usr/include/bson-{environ.get('BSON_VERSION', '2.2.1')}/",
    f"/usr/local/include/bson-{environ.get('BSON_VERSION', '2.2.1')}/",
    f"/opt/homebrew/include/bson-{environ.get('BSON_VERSION', '2.2.1')}/",
    "/opt/homebrew/include/",
    "/usr/local/"
]
include_dirs.extend(environ.get("INCLUDE_DIR", ".").split(":"))
lib_dirs = ["/opt/homebrew/lib/", "/usr/local/lib/", "/usr/lib64/"]
lib_dirs.extend(environ.get("LIB_DIR", ".").split(":"))
setup_args = dict(
    ext_modules = [
        Extension('bsonsearch.matcher_module',
                  sources=['bsonsearch/matcher_ext/matcher_module.c'],
                  libraries=['bson2', 'bsonsearch'],
                  library_dirs=lib_dirs,
                  include_dirs=include_dirs
        )
    ]
)
setup(**setup_args)
