#!/bin/bash

python setup.py bdist_rpm
rpmbuild -ba --sign ~/rpmbuild/SPECS/bsoncompare.spec
rpmbuild -ba --sign ~/rpmbuild/SPECS/python-bsonsearch.spec


