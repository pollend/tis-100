#include <Python.h>

#include "util.h"
#include "program.h"
#include "output.h"

#include "tis.h"
#include "program.h"
#include "node.h"
#include "instruction.h"
/*static PyObject* tis_100_process(PyObject *self, PyObject *args)
{
  PyObject* inputs;
  PyObject* outputs;

  int * layout;
  int layout_entries;

  const char ** code; 
  int code_entries;


  if (!PyArg_ParseTuple(args, "OO(i:)#(s:)#", 
    &inputs,&outputs,&layout,&layout_entries,
    &code,&code_entries))
    return NULL;
  return NULL;
}*/



/*static PyMethodDef TisMethods[] =
{
  {"program",py_program_init,METH_VARARGS},
  {NULL, NULL, 0, NULL}
};*/

void inittis(void)
{
  PyObject*m =  Py_InitModule3("tis", NULL,"program module");
  init_node_module(m);
  init_program_module(m);
  init_instruction_module(m);
}
