#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <Python.h>

#include "node.h"

typedef struct _Program {
  PyObject_HEAD

  PyObject *nodes;
  PyObject *active_nodes;
} Program;

PyObject* program_load_system(PyObject * self, PyObject * args);
PyObject* program_load_code(PyObject * self, PyObject * args);

int program_tick(const Program *p);
void program_clean(Program *p);


void init_program_module(PyObject* module);

#endif
