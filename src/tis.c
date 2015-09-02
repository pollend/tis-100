#include <Python.h>

#include "util.h"
#include "program.h"
#include "output.h"

#include "tis.h"
#include "program.h"
#include "node.h"
#include "instruction.h"

void inittis(void)
{
  PyObject*m =  Py_InitModule3("tis", NULL,"program module");
  init_node_module(m);
  init_program_module(m);
  init_instruction_module(m);
}
