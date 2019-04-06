#include <Python.h>

extern void* JInit();
extern int JFree(void* runtime);
extern void JSMX(void* runtime,
                 void* output,
                 void* wd,
                 void* input,
                 void* unused,
                 int opts);
extern int JDo(void* runtime, unsigned char* code);

typedef struct {
  PyObject_HEAD;
  void* runtime;
  PyObject* output_cb;
} j_runtime;

static j_runtime** all_runtimes = NULL;
static int num_runtimes = 0;

static void j_callback_output(void* runtime, int type, const char* str) {
  for (int i = 0; i < num_runtimes; ++i) {
    if (all_runtimes[i]->runtime == runtime) {
      j_runtime* rt = all_runtimes[i];
      if (rt->output_cb) {
        PyObject* args = Py_BuildValue("is", type, str);
        PyObject_CallObject(rt->output_cb, args);
      }
      return;
    }
  }
}

static void* j_callback_input(void* runtime, char* str) {
  return "";
}

static void j_runtime_dealloc(j_runtime* self) {
  JFree(self->runtime);
  Py_XDECREF(self->output_cb);

  for (int i = 0; i < num_runtimes; ++i) {
    if (all_runtimes[i] == self) {
      all_runtimes[i] = all_runtimes[num_runtimes - 1];
      break;
    }
  }
  num_runtimes -= 1;

  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* j_runtime_new(PyTypeObject* type,
                               PyObject* args,
                               PyObject* kwds) {
  j_runtime* self = (j_runtime*)type->tp_alloc(type, 0);
  self->runtime = JInit();
  self->output_cb = NULL;
  JSMX(self->runtime, j_callback_output, NULL, j_callback_input, NULL, 0);

  all_runtimes = realloc(all_runtimes, sizeof(j_runtime*) * (num_runtimes + 1));
  all_runtimes[num_runtimes++] = self;

  return (PyObject*)self;
}

static int j_runtime_init(j_runtime* self, PyObject* args, PyObject* kwds) {
  return 0;
}

static PyObject* j_runtime_do(j_runtime* self, PyObject* args, PyObject* kwds) {
  unsigned char* code;
  if (!PyArg_ParseTuple(args, "s", &code)) {
    return NULL;
  }
  int res = JDo(self->runtime, code);
  return PyLong_FromLong((long)res);
}

static PyObject* j_runtime_set_output(j_runtime* self,
                                      PyObject* args,
                                      PyObject* kwds) {
  PyObject* fn;
  if (!PyArg_ParseTuple(args, "O", &fn)) {
    return NULL;
  }
  if (!PyCallable_Check(fn)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be callable");
    return NULL;
  }
  Py_XDECREF(self->output_cb);
  Py_XINCREF(fn);
  self->output_cb = fn;
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef j_runtime_methods[] = {
    {"do", (PyCFunction)j_runtime_do, METH_VARARGS,
     "Evaluate a piece of J source code."},
    {"set_output", (PyCFunction)j_runtime_set_output, METH_VARARGS,
     "Set the source code output callback."},
    {NULL},
};

static PyTypeObject j_runtime_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pyj.Runtime",
    .tp_doc = "An instance of the J language runtime.",
    .tp_basicsize = sizeof(j_runtime),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = j_runtime_new,
    .tp_init = (initproc)j_runtime_init,
    .tp_dealloc = (destructor)j_runtime_dealloc,
    .tp_methods = j_runtime_methods,
};

static PyModuleDef pyj_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pyj",
    .m_doc = "Bindings for the J language engine.",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_pyj(void) {
  PyObject* m;
  if (PyType_Ready(&j_runtime_type) < 0) {
    return NULL;
  }

  m = PyModule_Create(&pyj_module);
  if (m == NULL) {
    return NULL;
  }

  Py_INCREF(&j_runtime_type);
  PyModule_AddObject(m, "Runtime", (PyObject*)&j_runtime_type);
  return m;
}