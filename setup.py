#!/usr/bin/python3

from setuptools import setup, Extension


module1 = Extension('bsonsearch.bsonhelper',
                    sources=['bsonsearch/bsonhelpermodule.c'],
                    libraries=['bson-1.0'],
                    include_dirs=["/usr/include/libbson-1.0/"])

setup(
    name='bsonsearch',
    version='1.17.4',
    description='compare bson documents',
    maintainer="Dan Bauman",
    packages=["bsonsearch"],
    ext_modules=[module1]
)