#!/usr/bin/python
from setuptools import setup, find_packages



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
setup(
    name = "python-bsonsearch",
    version = "1.3.0",
    maintainer = "Dan Bauman",
    packages=find_packages())
