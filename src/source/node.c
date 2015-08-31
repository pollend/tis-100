#include <Python.h>

#include "node.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "util.h"



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
      if (port && inst->operation == MOV && inst->first_type == ADDRESS &&
          (inst->first.direction == ANY ||
           port->ports[inst->first.direction] == n)) {
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

ReadResult node_read(Node *n, FieldType type, union Field where) {
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
            //default:
        //        raise_error("unhandled direction");
    }
  }

  return res;
}

static inline Instruction* create_instruction(char* line)
{
  Instruction* instance = (Instruction*)create_instruction_instance();
  parse_line(instance,line);
  return instance;
}

void append_instruction(Node *n,Instruction* instruction)
{
  PyList_Append((PyObject*)n->instructions,(PyObject*)instruction);
}

void node_parse(Node* node,char* input)
{
  char* data = strdup(input);
  char* line;

  while((line = strsep(&data,"\n")) != NULL)
  {
    char* c = line;

    while (*c != '\0') { 
        if (*c == '#') {
          break;
        } else if (*c == '!') {
          c++;
          PyList_Append((PyObject*)node->instructions,(PyObject*)create_instruction(c));
          break;
        }
        else if(*c == ':')
        {
         char* inst_one = malloc((c-line) +2);
         char* inst_two = malloc(strlen(c) +1);
         
         inst_one = memcpy(inst_one,line,(c-line) +2);
         inst_one[(c-line) +2] = '\0';

         inst_two = memcpy(inst_two,line,strlen(c) +1);

         PyList_Append((PyObject*)node->instructions, (PyObject*)create_instruction(inst_one));
         PyList_Append((PyObject*)node->instructions, (PyObject*)create_instruction(inst_two));
         free(inst_one);
         free(inst_two);

         break;

        }
        c++;
      }
  }
  free(data);
 
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
      read = node_read(n, i->first_type, i->first);
      if (read.blocked) return;
      blocked = node_write(n, i->second.direction, read.value);
      if (blocked) return;
      break;
    case ADD:
      read = node_read(n, i->first_type, i->first);
      if (read.blocked) return;

      n->acc += read.value;
      if (n->acc > MAX_ACC) n->acc = MAX_ACC;
      if (n->acc < MIN_ACC) n->acc = MIN_ACC;
      break;
    case SUB:
      read = node_read(n, i->first_type, i->first);
      if (read.blocked) return;

      n->acc -= read.value;
      if (n->acc > MAX_ACC) n->acc = MAX_ACC;
      if (n->acc < MIN_ACC) n->acc = MIN_ACC;
      break;
    case JMP:
      node_set_ip(n, i->first.number);
      return;
    case JRO:
      node_set_ip(n, n->ip + i->first.number);
      return;
    case JEZ:
      if (n->acc == 0) {
        node_set_ip(n, i->first.number);
        return;
      }
      break;
    case JGZ:
      if (n->acc > 0) {
        node_set_ip(n, i->first.number);
        return;
      }
      break;
    case JLZ:
      if (n->acc < 0) {
        node_set_ip(n, i->first.number);
        return;
      }
      break;
    case JNZ:
      if (n->acc != 0) {
        node_set_ip(n, i->first.number);
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
    case NONE:
    case JMP_FLAG:
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

void append_node(LocationDirection direction, Node* from, Node* to)
{
  switch(direction)
  {
    case UP:
      from->ports[UP] = to;
      to->ports[DOWN] = from;
    break;
    case RIGHT:
     from->ports[RIGHT] = to;
      to->ports[LEFT] = from;
    break;
    case DOWN:
     from->ports[DOWN] = to;
      to->ports[UP] = from;
    break;
    case LEFT:
     from->ports[LEFT] = to;
      to->ports[RIGHT] = from;
    break;
    default:
    break;
  }

}


/*Node*/
static int py_node_init(Node *self, PyObject *args, PyObject *kwds)  {
  return 0;
}




static PyObject* py_node_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  custom_log("creating node");
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
  custom_log("finished creating node");

  return (PyObject*)self;

}



static PyObject* py_node_parse(PyObject* self,PyObject* args)
{
  char* input = NULL;
  if(!PyArg_ParseTuple(args,"s",&input))
    return NULL;

  node_parse((Node*)self,input);

  Py_RETURN_NONE;
}


static void py_node_dealloc(PyObject *self)
{
   custom_log("started freeing Node");
   Py_XDECREF(((Node*)self)->instructions);
   self->ob_type->tp_free(self);
   custom_log("finished freeing Node");
}



static PyObject * py_node_str (PyObject* self) {
  Node* node = (Node*)self;

  int num_instructions = PyList_Size(node->instructions);
  char* final = NULL;
  for (int i = 0; i < num_instructions; ++i)
  {

    char* line = convert_instruction_str((Instruction*)PyList_GET_ITEM(node->instructions,i));

    if(final == NULL)
    {
      final = malloc(strlen(line)+1);
      strcpy(final,line);
    }
    else
    {
      final = realloc(final, strlen(final) + strlen(line)+2);
      strcat(final,"\n");
     strcat(final,line);
    }
    

    free(line);
  }


  PyObject* out = PyString_FromFormat("%s",final);
  free(final);
  return out;
 
}

/* Method table */
static PyMethodDef py_node_method[] = {
  {"Parse", (PyCFunction)py_node_parse, METH_VARARGS,"Parse Code"},
  {NULL},
};

static PyMemberDef py_node_members[] = {
    {NULL}  /* Sentinel */
};

static PyTypeObject node_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "node",                            /*tp_name*/
    sizeof(Node),                            /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)py_node_dealloc,                /*tp_dealloc*/
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
    py_node_str,                                        /*tp_str*/
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
    py_node_method,                            /*tp_methods*/
    py_node_members,                            /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)py_node_init,                     /*tp_init*/
    0,                                        /*tp_alloc*/
    py_node_new,                                /*tp_new*/
};



void init_node_module(PyObject* module)
{
    if (PyType_Ready(&node_type) < 0)
    {
      custom_log("not ready");
      return;
    }

   Py_INCREF(&node_type);
   PyModule_AddObject(module, "Node", (PyObject *)&node_type);
}

PyObject* create_node_instance()
{
   return  PyObject_CallObject((PyObject *)&node_type,NULL);
}