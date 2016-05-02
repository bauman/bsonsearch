/*
 * Copyright (c) 2016 Bauman
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Python.h>
#include <bson.h>


/*
 *--------------------------------------------------------------------------
 *
 * bson_as_string --
 *
 *      this function converts a bson_object (bson_t) to a Python String
 *      (http://api.mongodb.org/libbson/current/bson_t.html)
 *
 *      bson_t has embedded null characters, so you need to use the
 *             bson_get_data in coordination with the len to assemble
 *             the string, otherwise it will get truncated
 *
 * Requires:
 *     the first (and only) argument must be a 64 bit pointer to a bson_t
 *
 *     This library will segfault otherwise
 *
 *
 * Returns:
 *      PyObject<string (s#)>: if the requirements are met
 *
 *      NULL if the requirements are not met
 *
 * Notes:
 *      None
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
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