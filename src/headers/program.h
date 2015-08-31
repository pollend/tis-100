#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <Python.h>

#include "node.h"

typedef struct _Program {
  PyObject_HEAD

  PyObject *nodes;
  PyObject *active_nodes;
} Program;

PyObject* create_program_instance();

void program_clean(Program *p);

void init_program_module(PyObject* module);

#endif
