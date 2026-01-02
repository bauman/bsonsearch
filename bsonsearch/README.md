# CTYPES wrapper of bsoncompare library

This is the "old" ctypes wrapper

`python -m build`

# This requires the following installed on the system
- the bson library (from mongo-c-driver)
- the bsoncompare library
- pcre2 library 
- duktape library (optional) 
- discodb library (optional)

# Development Testing

```shell
 cmake-build-db % export  DYLD_LIBRARY_PATH=$(pwd)/usr/lib:$(pwd)
```

# Production

The production wheels should have everything included via cibw
