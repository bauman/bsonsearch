#!/usr/bin/python

#building requires update to mongo-c-driver/libbson.  Probably have to remove/build/update on build machine

#BUILDING libbsoncompare:
#  1. update version number in bsoncompare.spec to match
#  2. rpmbuild -ba --sign bsoncompare.spec


#BUILDING python-bsonsearch:
#   1. update version number below to rev that matches the targeted mongo-c-drver/libbson/libbsoncompare
#   2. update version number in python-bsonsearch.spec that matches
#   3. python setup.py bdist_rpm
#   4. ln -s the tarball over to build area
#   5. rpmbuild -ba --sign python-bsonsearch.spec to add the appropriate requires to the rpm

from distutils.core import setup, Extension


module1 = Extension('bsonhelper', #TODO: Figure out how to deploy this lib inside bsonsearch package
                    sources = ['bsonsearch/bsonhelpermodule.c'],
                    libraries=['bson-1.0'],
                    include_dirs=["/usr/include/libbson-1.0/"])

setup (name = 'python-bsonsearch',
        version = '1.3.3',
        description = 'compare bson documents',
        maintainer = "Dan Bauman",
        packages=["bsonsearch"],
        ext_modules = [module1]
       )