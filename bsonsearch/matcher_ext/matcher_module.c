#define WITH_MODULES
#define WITH_UTILS
#define WITH_PROJECTION
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "bsoncompare.h"

typedef struct {
    PyObject_HEAD
    Py_ssize_t value;
} Utils;

typedef struct {
    PyObject_HEAD
    mongoc_matcher_t *matcher;
    const char *json;
    Py_ssize_t value;
} Matcher;

typedef struct {
    PyObject_HEAD
    bson_t  *document;
    const char *json;
    Py_ssize_t value;
} Document;


// ---- UTILS FUNCTIONS ----
static PyObject *
Utils_startup(Utils *self, PyObject *args, PyObject *kwds)
{
    int result = bsonsearch_startup();
    return PyLong_FromLong(result);
}

static PyObject *
Utils_shutdown(Utils *self, PyObject *args, PyObject *kwds)
{
    int result = bsonsearch_shutdown();
    return PyLong_FromLong(result);
}

static PyObject *
Utils_regex_destroy(Utils *self, PyObject *args, PyObject *kwds)
{
    int result = regex_destroy();
    return PyLong_FromLong(result);
}


static PyObject *
Utils_haversine_distance(Utils *self, PyObject *args, PyObject *kwds)
{
    double lon1;
    double lat1;
    double lon2;
    double lat2;
    if (!PyArg_ParseTuple(args, "dddd", &lon1, &lat1, &lon2, &lat2)) {
        return NULL; // Error occurred, exception already set
    }
    double result = bsonsearch_haversine_distance(
        lon1, lat1, lon2, lat2
    );
    return PyFloat_FromDouble(result);
}


static PyObject *
Utils_haversine_distance_degrees(Utils *self, PyObject *args, PyObject *kwds)
{
    double lon1;
    double lat1;
    double lon2;
    double lat2;
    if (!PyArg_ParseTuple(args, "dddd", &lon1, &lat1, &lon2, &lat2)) {
        return NULL; // Error occurred, exception already set
    }
    double result = bsonsearch_haversine_distance_degrees(
        lon1, lat1, lon2, lat2
    );
    return PyFloat_FromDouble(result);
}

static PyObject *
Utils_crossarc_degrees(Utils *self, PyObject *args, PyObject *kwds)
{
    double lon1;
    double lat1;
    double lon2;
    double lat2;
    double lon3;
    double lat3;
    if (!PyArg_ParseTuple(args, "dddddd", &lon1, &lat1, &lon2, &lat2, &lon3, &lat3)) {
        return NULL; // Error occurred, exception already set
    }
    double result = bsonsearch_get_crossarc_degrees(
        lon1, lat1, lon2, lat2, lon3, lat3
    );
    return PyFloat_FromDouble(result);
}

static PyObject *
Utils_get_value(Utils *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromLong(self->value);
}

// Method definitions for Matcher
static PyMethodDef Utils_methods[] = {
    {"get_value", (PyCFunction)Utils_get_value, METH_NOARGS, "Return the value of the Document instance."},
    {"regex_destroy", (PyCFunction)Utils_regex_destroy, METH_VARARGS, "Frees internal regex cache"},
    {"startup", (PyCFunction)Utils_startup, METH_VARARGS, "Prep the module internally"},
    {"shutdown", (PyCFunction)Utils_shutdown, METH_VARARGS, "Cleanup the module internally"},
    {"haversine_distance", (PyCFunction)Utils_haversine_distance, METH_VARARGS, "Haversine distance using radians"},
    {"haversine_distance_degrees", (PyCFunction)Utils_haversine_distance_degrees, METH_VARARGS, "Haversine distance using degrees"},
    {"crossarc_degrees", (PyCFunction)Utils_crossarc_degrees, METH_VARARGS, "Crossarc"},
    {NULL}  /* Sentinel */
};

// Matcher_new: Constructor (allocates and initializes the C struct)
static PyObject *
Utils_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Utils *self;
    self = (Utils *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->value = 0; // Default value
    }
    return (PyObject *)self;
}

// Matcher_init: Initializer (sets instance attributes)
static int
Utils_init(Utils *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

// Matcher_dealloc: Destructor (frees resources)
static void
Utils_dealloc(Document *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);
}


// 6. Create the PyTypeObject
static PyTypeObject UtilsClassType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bsonsearch.matcher_module.Utils",
    .tp_doc = "matchermodule",
    .tp_basicsize = sizeof(Utils),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Utils_new, // Use the custom tp_new
    .tp_init = (initproc)Utils_init,
    .tp_dealloc = (destructor)Utils_dealloc,
    .tp_methods = Utils_methods,
};

//  ----  Document FUNCTIONS ----

static PyObject *
Document_get_value(Document *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromLong(self->value);
}

// Method definitions for Matcher
static PyMethodDef Document_methods[] = {
    {"get_value", (PyCFunction)Document_get_value, METH_NOARGS,
        "Return the value of the Document instance."},
    {NULL}  /* Sentinel */
};

// Matcher_new: Constructor (allocates and initializes the C struct)
static PyObject *
Document_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Document *self;
    self = (Document *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->value = 0; // Default value
    }
    return (PyObject *)self;
}

// Matcher_init: Initializer (sets instance attributes)
static int
Document_init(Document *self, PyObject *args, PyObject *kwds)
{
    if (!PyArg_ParseTuple(args, "s#", &self->json, &self->value)) {
        return -1;
    }
    self->document = generate_doc_from_json((const uint8_t *)self->json, (uint32_t)self->value);
    if (self->document == NULL) {
        return -2;
    }
    return 0;
}

// Matcher_dealloc: Destructor (frees resources)
static void
Document_dealloc(Document *self)
{
    doc_destroy(self->document);
    Py_TYPE(self)->tp_free((PyObject *)self);
}


// 6. Create the PyTypeObject
static PyTypeObject DocumentClassType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bsonsearch.matcher_module.Document",
    .tp_doc = "matchermodule",
    .tp_basicsize = sizeof(Matcher),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Document_new, // Use the custom tp_new
    .tp_init = (initproc)Document_init,
    .tp_dealloc = (destructor)Document_dealloc,
    .tp_methods = Document_methods,
};




//  ----  MATCHER FUNCTIONS ----
static PyObject *
Matcher_get_value(Matcher *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromLong(self->value);
}

static PyObject *
Matcher_match_json(Matcher *self, PyObject *args, PyObject *kwds)
{
    const char *c_string_ptr;
    Py_ssize_t c_string_len;
    if (!PyArg_ParseTuple(args, "s#", &c_string_ptr, &c_string_len)) {
        return NULL;
    }
    bson_t *doc = generate_doc_from_json((const uint8_t*)c_string_ptr, c_string_len);
    if (!doc) {
        return NULL;
    }
    long result = matcher_compare_doc(self->matcher, doc);
    doc_destroy(doc);
    return PyBool_FromLong(result);
}

static PyObject *
Matcher_match_doc(Matcher *self, PyObject *args, PyObject *kwds)
{
    PyObject *my_obj;
    if (!PyArg_ParseTuple(args, "O!", &DocumentClassType, &my_obj)) {
        return NULL; // PyArg_ParseTuple handles the error
    }
    Document *document = (Document *)my_obj;
    long result = matcher_compare_doc(self->matcher, document->document);
    return PyBool_FromLong(result);
}

static PyObject *
Matcher_project_json(Matcher *self, PyObject *args, PyObject *kwds)
{
    PyObject *my_obj;
    if (!PyArg_ParseTuple(args, "O!", &DocumentClassType, &my_obj)) {
        return NULL; // PyArg_ParseTuple handles the error
    }
    Document *document = (Document *)my_obj;
    char *result = bsonsearch_project_json(self->matcher, document->document);
    PyObject *returnable =  Py_BuildValue("s", result);
    bson_free(result);
    return returnable;
}



// Method definitions for Matcher
static PyMethodDef Matcher_methods[] = {
    {"get_value", (PyCFunction)Matcher_get_value, METH_NOARGS,
        "Return the value of the Matcher instance."},
    {"match_json", (PyCFunction)Matcher_match_json, METH_VARARGS, "Returns whether the doc matches the spec"},
    {"match_doc", (PyCFunction)Matcher_match_doc, METH_VARARGS, "Returns whether the doc matches the spec"},
    {"project_json", (PyCFunction)Matcher_project_json, METH_VARARGS, "projects data into a json document"},
    {NULL}  /* Sentinel */
};

// Matcher_new: Constructor (allocates and initializes the C struct)
static PyObject *
Matcher_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Matcher *self;
    self = (Matcher *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->value = 0; // Default value
    }
    return (PyObject *)self;
}

// Matcher_init: Initializer (sets instance attributes)
static int
Matcher_init(Matcher *self, PyObject *args, PyObject *kwds)
{
    if (!PyArg_ParseTuple(args, "s#", &self->json, &self->value)) {
        return -1;
    }
    self->matcher = generate_matcher_from_json((const uint8_t *)self->json, (uint32_t)self->value);
    if (self->matcher == NULL) {
        return -2;
    }
    return 0;
}

// Matcher_dealloc: Destructor (frees resources)
static void
Matcher_dealloc(Matcher *self)
{
    matcher_destroy(self->matcher);
    Py_TYPE(self)->tp_free((PyObject *)self);
}


// 6. Create the PyTypeObject
static PyTypeObject MatcherClassType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bsonsearch.matcher_module.Matcher",
    .tp_doc = "matchermodule",
    .tp_basicsize = sizeof(Matcher),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Matcher_new, // Use the custom tp_new
    .tp_init = (initproc)Matcher_init,
    .tp_dealloc = (destructor)Matcher_dealloc,
    .tp_methods = Matcher_methods,
};






//  ----  Module FUNCTIONS  (unlikely to need editing below here) ----


static struct PyModuleDef matcher_module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "bsonsearch.matcher_module",
    .m_doc = "The module.",
    .m_size = -1
};


// Module initialization function
PyMODINIT_FUNC
PyInit_matcher_module(void)
{
    PyObject *m;

    if (PyType_Ready(&MatcherClassType) < 0)
        return NULL;

    if (PyType_Ready(&DocumentClassType) < 0)
        return NULL;

    if (PyType_Ready(&UtilsClassType) < 0)
        return NULL;

    m = PyModule_Create(&matcher_module_def);
    if (m == NULL)
        return NULL;

    Py_INCREF(&MatcherClassType);
    if (PyModule_AddObject(m, "Matcher", (PyObject *)&MatcherClassType) < 0) {
        Py_DECREF(&MatcherClassType);
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&DocumentClassType);
    if (PyModule_AddObject(m, "Document", (PyObject *)&DocumentClassType) < 0) {
        Py_DECREF(&DocumentClassType);
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&UtilsClassType);
    if (PyModule_AddObject(m, "Utils", (PyObject *)&UtilsClassType) < 0) {
        Py_DECREF(&UtilsClassType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
