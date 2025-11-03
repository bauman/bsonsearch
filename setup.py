from setuptools import setup, Extension
from os import environ

include_dirs = [
    f"/usr/include/bson-{environ.get('BSON_VERSION', '2.1.2')}/",
    f"/opt/homebrew/include/bson-{environ.get('BSON_VERSION', '2.1.2')}/"
]
include_dirs.extend(environ.get("INCLUDE_DIR", ".").split(":"))
lib_dirs = []
lib_dirs.extend(environ.get("LIB_DIR", ".").split(":"))
setup_args = dict(
    ext_modules = [
        Extension('bsonsearch.bsonhelper',
                  sources=['bsonsearch/bsonhelpermodule.c'],
                  libraries=['bson2'],
                  library_dirs=lib_dirs,
                  include_dirs=include_dirs
        ),
        Extension('bsonsearch.matcher_module',
                  sources=['bsonsearch/matcher_ext/matcher_module.c'],
                  libraries=['bson2', 'bsonsearch'],
                  library_dirs=lib_dirs,
                  include_dirs=include_dirs
        )
    ]
)
setup(**setup_args)
