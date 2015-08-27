#include <Python.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "node.h"

staticforward PyTypeObject node_type;

/*PyObject *py_node_init(PyObject *self, PyObject *args) {

}*/


void node_clean(Node *n) { free(n->instructions); }

Instruction *node_create_instruction(Node *n, Operation op) {
  //assert(n->instruction_count < MAX_INSTRUCTIONS);
  //Instruction *i = &n->instructions[n->instruction_count++];
  //i->operation = op;
  //return i;
  return NULL;
}

static void parse_location(const char *s, union Location *loc,LocationType *type) {
  //  if (!s) { raise_error("no source was found"); }

  if (strcmp(s, "UP") == 0) {
    *type = ADDRESS;
    (*loc).direction = UP;
  } else if (strcmp(s, "DOWN") == 0) {
    *type = ADDRESS;
    (*loc).direction = DOWN;
  } else if (strcmp(s, "LEFT") == 0) {
    *type = ADDRESS;
    (*loc).direction = LEFT;
  } else if (strcmp(s, "RIGHT") == 0) {
    *type = ADDRESS;
    (*loc).direction = RIGHT;
  } else if (strcmp(s, "ACC") == 0) {
    *type = ADDRESS;
    (*loc).direction = ACC;
  } else if (strcmp(s, "NIL") == 0) {
    *type = ADDRESS;
    (*loc).direction = NIL;
  } else if (strcmp(s, "ANY") == 0) {
    *type = ADDRESS;
    (*loc).direction = ANY;
  } else if (strcmp(s, "LAST") == 0) {
    *type = ADDRESS;
    (*loc).direction = LAST;
  } else {
    *type = NUMBER;
    (*loc).number = atoi(s);
  }
}

static void parse_mov(Node *n, const char *s) {
  const int len = strlen(s + 4);
  char *rem = (char *)malloc(sizeof(char) * (len + 1));
  strcpy(rem, s + 4);

  Instruction *i = node_create_instruction(n, MOV);
  parse_location(strtok(rem, " ,"), &i->src, &i->src_type);
  parse_location(strtok(NULL, " ,\n"), &i->dest, &i->dest_type);

  free(rem);
}

static void parse_onearg(Node *n, InputCode *ic, const char *s, Operation op) {
  const int len = strlen(s + 4);
  char *rem = (char *)malloc(sizeof(char) * (len + 1));
  strcpy(rem, s + 4);

  Instruction *ins = node_create_instruction(n, op);

  switch (op) {
    case JEZ:
    case JMP:
    case JNZ:
    case JGZ:
    case JLZ:
      for (int i = 0; i < ic->label_count; i++) {
        const char *label = ic->labels[i];
        if (strcmp(label, rem) == 0) {
          ins->src_type = NUMBER;
          ins->src.number = ic->label_address[i];
          goto finally;
        }
      }
    default:
      parse_location(rem, &ins->src, &ins->src_type);
  }
finally:
  free(rem);
}

void node_parse_code(Node *n, InputCode *ic) {
  // First let's find the labels
  for (int i = 0; i < ic->line_count; i++) {
    char *line = ic->lines[i];

    // Look for a label
    char *c = line;
    while (*c != '\0') {
      if (*c == ':') {
        int length = (c - line);
        char *label = (char *)malloc(sizeof(char) * (length + 1));
        strncpy(label, line, length);
        label[length] = '\0';

        int idx = ic->label_count;
        ic->labels[idx] = label;
        ic->label_address[idx] = i;
        ic->label_count++;

        // Remove the label from the code
        char *rem = trim_whitespace(c + 1);

        // We need something to jump to, so NOP for now
        // TODO: compress empty lines and jump to the next instruction
        if (!strlen(rem)) {
          rem = "NOP";
        }

        char *new_line = (char *)malloc(sizeof(char) * strlen(rem));
        strcpy(new_line, rem);

        free(line);
        line = new_line;
        ic->lines[i] = new_line;
      }
      c++;
    }
  }

  for (int i = 0; i < ic->line_count; i++) {
    node_parse_line(n, ic, ic->lines[i]);
  }
}

void node_parse_line(Node *n, InputCode *ic, const char *s) {
  assert(n);
  assert(s);
  assert(strlen(s) > 2);

  char ins[4];
  strncpy(ins, s, 3);
  ins[3] = '\0';

  if (strcmp(ins, "MOV") == 0) {
    parse_mov(n, s);
  } else if (strcmp(ins, "SUB") == 0) {
    parse_onearg(n, ic, s, SUB);
  } else if (strcmp(ins, "ADD") == 0) {
    parse_onearg(n, ic, s, ADD);
  } else if (strcmp(ins, "JEZ") == 0) {
    parse_onearg(n, ic, s, JEZ);
  } else if (strcmp(ins, "JMP") == 0) {
    parse_onearg(n, ic, s, JMP);
  } else if (strcmp(ins, "JNZ") == 0) {
    parse_onearg(n, ic, s, JNZ);
  } else if (strcmp(ins, "JGZ") == 0) {
    parse_onearg(n, ic, s, JGZ);
  } else if (strcmp(ins, "JLZ") == 0) {
    parse_onearg(n, ic, s, JLZ);
  } else if (strcmp(ins, "JRO") == 0) {
    parse_onearg(n, ic, s, JRO);
  } else if (strcmp(ins, "SAV") == 0) {
    node_create_instruction(n, SAV);
  } else if (strcmp(ins, "SWP") == 0) {
    node_create_instruction(n, SWP);
  } else if (strcmp(ins, "NOP") == 0) {
    node_create_instruction(n, NOP);
  } else if (strcmp(ins, "NEG") == 0) {
    node_create_instruction(n, NEG);
  } else if (strcmp(ins, "OUT") == 0) {
    node_create_instruction(n, OUT);
  } else {
    //    raise_error("Don't understand instruction [%s]", ins);
  }
}

static inline void node_set_ip(Node *n, short new_val) {
  if (new_val >= PyList_Size(n->instructions) || new_val < 0) new_val = 0;
  n->ip = new_val;
}

static inline Node *node_get_input_port(Node *n, int direction) {
  if (direction == ANY) {
    LocationDirection dirs[] = {LEFT, RIGHT, UP, DOWN};
    for (int i = 0; i < 4; i++) {
      Node *port = n->ports[dirs[i]];
      if (port && port->output_port == n) {
        return port;
      }
    }
    return NULL;
  } else if (direction == LAST) {
    return n->last;
  } else {
    return n->ports[direction];
  }
}

static inline Node *node_get_output_port(Node *n, int direction) {
  if (direction == ANY) {
    LocationDirection dirs[] = {UP, LEFT, RIGHT, DOWN};
    for (int i = 0; i < 4; i++) {
      Node *port = n->ports[dirs[i]];
      Instruction *inst = (Instruction*)PyList_GetItem(port->instructions,n->ip);
      if (port && inst->operation == MOV && inst->src_type == ADDRESS &&
          (inst->src.direction == ANY ||
           port->ports[inst->src.direction] == n)) {
        return port;
      }
    }
    return NULL;
  } else if (direction == LAST) {
    return n->last;
  } else {
    return n->ports[direction];
  }
}

ReadResult node_read(Node *n, LocationType type, union Location where) {
  ReadResult res;
  res.blocked = 0;

  if (n->output_port) {
    return res;
  }

  if (type == NUMBER) {
    res.value = where.number;
  } else {
    Node *read_from;
    switch (where.direction) {
      case NIL:
        res.value = 0;
        break;
      case ACC:
        res.value = n->acc;
        break;
      case UP:
      case RIGHT:
      case DOWN:
      case LEFT:
      case ANY:
      case LAST:
        read_from = node_get_input_port(n, where.direction);
        if (read_from && read_from->output_port == n) {
          res.value = read_from->output_value;
          res.blocked = 0;

          read_from->output_value = 0;
          read_from->output_port = NULL;
          node_advance(read_from);
          if (where.direction == ANY) n->last = read_from;
        } else if (read_from == NULL && where.direction == LAST) {
          res.value = 0;
        } else {
          res.blocked = 1;
        }
        break;
        //    default:

        //        raise_error("unhandled direction");
    }
  }

  return res;
}

int node_write(Node *n, LocationDirection dir, short value) {
  Node *dest;
  switch (dir) {
    case ACC:
      n->acc = value;
      break;
    case UP:
    case RIGHT:
    case DOWN:
    case LEFT:
    case ANY:
    case LAST:
      dest = node_get_output_port(n, dir);
      if (dest && n->output_port == NULL) {
        n->output_port = dest;
        n->output_value = value;
        if (dir == ANY) n->last = dest;
      }
      return 1;
      break;
    case NIL:
      break;
      //  return NULL;
      //  raise_error("Can't write to %d", dir);
      //    default:
      //      raise_error("don't know how to write %d", dir);
  }

  // not blocked
  return 0;
}

void node_advance(Node *n) { node_set_ip(n, n->ip + 1); }

void node_tick(Node *n) {
  n->blocked = TRUE;

  Instruction *i = (Instruction*)PyList_GetItem(n->instructions,n->ip);
  short tmp;
  ReadResult read;

  int blocked;

  switch (i->operation) {
    case MOV:
      read = node_read(n, i->src_type, i->src);
      if (read.blocked) return;
      blocked = node_write(n, i->dest.direction, read.value);
      if (blocked) return;
      break;
    case ADD:
      read = node_read(n, i->src_type, i->src);
      if (read.blocked) return;

      n->acc += read.value;
      if (n->acc > MAX_ACC) n->acc = MAX_ACC;
      if (n->acc < MIN_ACC) n->acc = MIN_ACC;
      break;
    case SUB:
      read = node_read(n, i->src_type, i->src);
      if (read.blocked) return;

      n->acc -= read.value;
      if (n->acc > MAX_ACC) n->acc = MAX_ACC;
      if (n->acc < MIN_ACC) n->acc = MIN_ACC;
      break;
    case JMP:
      node_set_ip(n, i->src.number);
      return;
    case JRO:
      node_set_ip(n, n->ip + i->src.number);
      return;
    case JEZ:
      if (n->acc == 0) {
        node_set_ip(n, i->src.number);
        return;
      }
      break;
    case JGZ:
      if (n->acc > 0) {
        node_set_ip(n, i->src.number);
        return;
      }
      break;
    case JLZ:
      if (n->acc < 0) {
        node_set_ip(n, i->src.number);
        return;
      }
      break;
    case JNZ:
      if (n->acc != 0) {
        node_set_ip(n, i->src.number);
        return;
      }
      break;
    case SWP:
      tmp = n->bak;
      n->bak = n->acc;
      n->acc = tmp;
      break;
    case SAV:
      n->bak = n->acc;
      break;
    case NEG:
      n->acc = n->acc * -1;
      break;
    case NOP:
      break;
    case OUT:
#ifndef RICH_OUTPUT
      printf("%d\n", n->acc);
#endif
      break;
      //    default:
      //      raise_error("ERROR: DIDN'T HANDLE op\n");
  }
  n->blocked = FALSE;
  node_advance(n);
}



/*Instructions*/


static void instruction_dealloc(PyObject *self)
{
   custom_log("started freeing Instruction");
   self->ob_type->tp_free(self);
   custom_log("finished freeing Instruction");

}

/* Method table */
static PyMethodDef instruction_method[] = {
  {NULL},
};

static PyMemberDef instruction_members[] = {
    {NULL}  /* Sentinel */
};

static int instruction_init(Instruction *self, PyObject *args, PyObject *kwds)
{
  PyObject* code;
  if(PyArg_ParseTuple(args,"s",&code))
  {

  }
  return 0;
}

static PyObject* instruction_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Instruction* self;
  self = (Instruction*)type->tp_alloc(type,0);

  return (PyObject*)self;  
}


static PyTypeObject instruction_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "instruction",                            /*tp_name*/
    sizeof(Instruction),                      /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)instruction_dealloc,         /*tp_dealloc*/
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
    "node objects",                           /*tp_doc */
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    instruction_method,                       /*tp_methods*/
    instruction_members,                      /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)instruction_init,               /*tp_init*/
    0,                                        /*tp_alloc*/
    instruction_new,                          /*tp_new*/
};

/*Node*/
static int node_init(Node *self, PyObject *args, PyObject *kwds)  {

  return 0;
}

static PyObject* node_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Node* self;
  self = (Node*)type->tp_alloc(type,0);
  if(self != NULL)
  {
    self->instructions  = Py_BuildValue("[]");
    if(self->instructions == NULL)
    {
       Py_DECREF(self);
      return NULL;
    }
    self->acc = 0;
    self->bak= 0;
    self->ip = 0;
    self->output_value = 0;

    self->visible = FALSE;
    self->blocked = FALSE;

    self->output_port = NULL;
    self->last = NULL;

    self->ports[0] = NULL;
    self->ports[1] = NULL;
    self->ports[2] = NULL;
    self->ports[3] = NULL;
    

  }


  return (PyObject*)self;

}


static void node_dealloc(PyObject *self)
{
    custom_log("started freeing Node");
  Py_XDECREF(((Node*)self)->instructions);
   self->ob_type->tp_free(self);
   custom_log("finished freeing Node");
}

/* Method table */
static PyMethodDef node_method[] = {
  {NULL},
};

static PyMemberDef node_members[] = {
    {NULL}  /* Sentinel */
};



static PyTypeObject node_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "node",                            /*tp_name*/
    sizeof(Node),                            /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)node_dealloc,                /*tp_dealloc*/
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
    "node objects",                        /*tp_doc */
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    node_method,                            /*tp_methods*/
    node_members,                            /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)node_init,                     /*tp_init*/
    0,                                        /*tp_alloc*/
    node_new,                                /*tp_new*/
};



void init_node_module(PyObject* module)
{
    if (PyType_Ready(&node_type) < 0)
    {
      custom_log("not ready");
      return;
    }

   Py_INCREF(&node_type);
   PyModule_AddObject(module, "Program", (PyObject *)&node_type);

   
   if (PyType_Ready(&instruction_type) < 0)
   {
      custom_log("not ready");
      return;
   }

   Py_INCREF(&instruction_type);
   PyModule_AddObject(module, "Program", (PyObject *)&instruction_type);
}


PyObject* create_instruction_instance()
{

  PyObject* obj = _PyObject_New(&instruction_type);
  return PyObject_Init(obj,&instruction_type);
}

PyObject* create_node_instance()
{
    PyObject* obj = _PyObject_New(&node_type);
  return PyObject_Init(obj,&node_type);

}