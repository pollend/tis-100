#include <Python.h>
#include "structmember.h"

#include <assert.h>
#include <string.h>

#include "node.h"
#include "constants.h"
#include "input_code.h"
#include "util.h"
#include "program.h"

#include "curses.h"


staticforward PyTypeObject program_type;


static Node *create_node(Program *p) {
  PyObject* node = node_init();
  PyList_Append(p->nodes, node);
  return (Node*)node;
}

static PyObject* program_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Program* self;
  self = (Program *)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->nodes =   Py_BuildValue("[]");
    if( self->nodes == NULL)
    {
      Py_DECREF(self);
      return NULL;
    }

    self->active_nodes  =   Py_BuildValue("[]");
    if( self->active_nodes == NULL)
    {
      Py_DECREF(self);
      return NULL;
    }
  }
  return (PyObject*)self;
}


static int  program_init(Program *self, PyObject *args, PyObject *kwds) {
  custom_log("creating program");

  /*for (int i=0; i<PROGRAM_NODES; i++) {
    Node *n = create_node(p);
    n->visible = TRUE;
    n->number = i;
  }*/

  // Link all the nodes up
  /*Node **nodes = p->nodes_by_index;
  for (int i=0; i<4; i++) {
    if (i < 3) {
      nodes[i]->ports[RIGHT] = nodes[i+1];
      nodes[i+4]->ports[RIGHT] = nodes[i+5];
      nodes[i+8]->ports[RIGHT] = nodes[i+9];
      nodes[i+1]->ports[LEFT] = nodes[i];
      nodes[i+5]->ports[LEFT] = nodes[i+4];
      nodes[i+9]->ports[LEFT] = nodes[i+8];
    }
    nodes[i]->ports[DOWN] = nodes[i+4];
    nodes[i+4]->ports[DOWN] = nodes[i+8];
    nodes[i+4]->ports[UP] = nodes[i];
    nodes[i+8]->ports[UP] = nodes[i+4];
  }*/
  return 0;
}



/*PyObject* py_program_init(PyObject * self, PyObject * args)
{
  if (!PyArg_ParseTuple(args, ""))
   return NULL;

  return program_init();
  
}*/
/*
PyObject* py_program_init(PyObject * self, PyObject * args)
{
  if (!PyArg_ParseTuple(args, ""))
   return NULL;

  return program_init();
  
}*/

/*void program_clean(Program *p) {
  node_list_clean(p->active_nodes, FALSE);
  node_list_clean(p->nodes, TRUE);
  p->nodes = NULL;
}*/

int program_tick(const Program *p) {
  int all_blocked = TRUE;

  for (int i = 0; i < PyList_Size(p->active_nodes); ++i)
  {
    Node *n = (Node*)PyList_GetItem(p->active_nodes,i);
    node_tick(n);
    all_blocked &= n->blocked;
  }


  return all_blocked;
}

static char *read_line(PyObject* file) {
  PyObject* pyline = PyFile_GetLine(file,0);
  char* line= NULL;
  
  if(PyErr_ExceptionMatches(PyExc_EOFError))
      return NULL;

  if (!PyArg_ParseTuple(pyline, "s", &line)) 
    return NULL;

  return line;
}





static Node *create_input_node(Program *p, PyObject* file) {
  Node *n = create_node(p);

  char *line = NULL;
  line = read_line(file);
  Node *below =NULL;// p->nodes_by_index[atoi(line)];

  n->ports[DOWN] = below;
  below->ports[UP] = n;

  line = read_line(file);
  while (line[0] != '*') {
    Instruction *i = node_create_instruction(n, MOV);
    i->src_type = NUMBER;
    i->src.number = atoi(line);
    i->dest_type = ADDRESS;
    i->dest.direction = DOWN;
    line = read_line(file);
  }

  Instruction *i = node_create_instruction(n, JRO);
  i->src_type = NUMBER;
  i->src.number = 0;

  return n;
}

static Node *create_output_node(Program *p, PyObject* file) {
  Node *n = create_node(p);


  /*char *line = NULL;
  line = read_line(file);
  Node *above =NULL;// p->nodes_by_index[atoi(line)];

  Instruction *i = node_create_instruction(n, MOV);
  i->src_type = ADDRESS;
  i->src.direction = UP;
  i->dest_type = ADDRESS;
  i->dest.direction = ACC;

  node_create_instruction(n, OUT);

  n->ports[UP] = above;
  above->ports[DOWN] = n;*/

  return n;
}

PyObject* program_load_system(PyObject * self, PyObject * args) {
  PyObject* file = NULL;

  if (!PyArg_ParseTuple(args, "O",file))
   return NULL;


  char * line = NULL;
  while (true) {
    line = read_line(file);
    if(line == NULL)
      return NULL;

    Node *n = NULL;
    if (strncmp(line, "input-top", 9) == 0) {
      n = create_input_node((Program*)self, file);
    } else if (strncmp(line, "output-bottom", 12) == 0) {
      n = create_output_node((Program*)self, file);
    }
    if (n) {
      PyList_Append((PyObject*)((Program*)self)->active_nodes,(PyObject*)n);
    }
  }

}

PyObject* program_load_code(PyObject * self, PyObject * args) {

  PyObject* file = NULL;

  if (!PyArg_ParseTuple(args, "O",file))
   return NULL;

  InputCode all_input[PROGRAM_NODES];
  for (int i=0; i<PROGRAM_NODES; i++) {
    input_code_init(&all_input[i]);
  }

   char * line = NULL;
   while (true) {

    line = read_line(file);
    if(line == NULL)
      break;
    // ignore after comment, ignore debug
    char *c = line;
    while (*c != '\0') {
      if (*c == '#') {
        *c = '\0';
        break;
      } else if (*c == '!') {
        *c = ' ';
      }
      c++;
    }
    char *trimmed = trim_whitespace(line);

    if (strlen(trimmed) > 0) {
      int index = 0;
      if (line[0] == '@') {
        index = atoi(trimmed+1);
      } else {
        input_code_addline(&all_input[index], trimmed);
      }
    }
    return NULL;
  }

  for (int i=0; i<PROGRAM_NODES; i++) {
    Node *n = NULL;//((Program*)self)->nodes_by_index[i];
    node_parse_code(n, &all_input[n->number]);
    input_code_clean(&all_input[n->number]);

    if (n->instruction_count > 0) {
       PyList_Append((PyObject*)((Program*)self)->active_nodes,(PyObject*)n);
    }
  }

  return NULL;
}


static void program_decalloc(PyObject *self)
{
  custom_log("started freeing Program");
  Py_XDECREF(((Program*)self)->nodes);
  Py_XDECREF(((Program*)self)->active_nodes);
   self->ob_type->tp_free(self);
   custom_log("finished freeing Program");
  
}

/* Method table */
static PyMethodDef program_method[] = {
  {"LoadSystem", (PyCFunction)program_load_system, METH_VARARGS,"loads states"},
  {"LoadCode", (PyCFunction)program_load_code, METH_VARARGS,"lodas code"},
  {NULL},
};

static PyMemberDef program_members[] = {
    {"nodes", T_OBJECT_EX, offsetof(Program, nodes), 0, "first name"},
    {NULL}  /* Sentinel */
};


  static PyTypeObject program_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "program",                            /*tp_name*/
    sizeof(Program),                            /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)program_decalloc,                /*tp_dealloc*/
    0,                                        /*tp_print*/
    0,                                        /*tp_getattr*/
    0,                                        /*tp_setattr*/
    0,                                        /*tp_compare*/
    0,                                        /*tp_repr*/
    0,                                        /*tp_as_number*/
    0,                                        /*tp_as_sequence*/
    0,                                        /*tp_as_mapping*/
    0,                                        /*tp_hash */
    0,                                        /*tp_call*/
    0,                                        /*tp_str*/
    0,                                        /*tp_getattro*/
    0,                                        /*tp_setattro*/
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "program objects",                        /*tp_doc */
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    program_method,                            /*tp_methods*/
    program_members,                            /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)program_init,                     /*tp_init*/
    0,                                        /*tp_alloc*/
    program_new,                                /*tp_new*/
  };



void init_program_module(PyObject* module)
{
    if (PyType_Ready(&program_type) < 0)
    {
      custom_log("not ready");
    return;
    }


   Py_INCREF(&program_type);
   PyModule_AddObject(module, "Program", (PyObject *)&program_type);
}