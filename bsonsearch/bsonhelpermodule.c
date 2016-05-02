#include <Python.h>
#include <bson.h>

static PyObject*
bson_as_string(PyObject* self, PyObject* args)
{
    bson_t * bson_object;

    if (PyArg_ParseTuple(args, "L", &bson_object)){
        const uint8_t *doc_bson = bson_get_data(bson_object);
        PyObject * result =  Py_BuildValue("s#", doc_bson, bson_object->len);
        return result;
    }
    return NULL;
}

static PyMethodDef BsonHelperMethods[] =
{
     {"bson_as_string", bson_as_string, METH_VARARGS, "Convert bson_t to python string"},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initbsonhelper(void)
{
     (void) Py_InitModule("bsonhelper", BsonHelperMethods);
}