#ifndef _NODE_H
#define _NODE_H

#include <Python.h>
#include "structmember.h"
#include "instruction.h"
#include <stdbool.h>

#include "constants.h"
#include "input_code.h"

typedef struct _Node {
  PyObject_HEAD 

  bool visible;
  bool blocked;

  unsigned int ip;
  unsigned int number;

  PyObject *instructions; //list of Instruction

  short acc;
  short bak;

  struct _Node *output_port, *last;
  short output_value;

  struct _Node *ports[4];
} Node;

typedef struct _ReadResult {
  int blocked;
  short value;
} ReadResult;


void node_clean(Node *n);

void node_parse_code(Node *n, InputCode *ic);
void node_parse_line(Node *n, InputCode *ic, const char *line);
void node_tick(Node *n);
ReadResult node_read(Node *n, LocationType lt, union Location where);
int node_write(Node *n, LocationDirection dir, short value);
void node_advance(Node *n);
Instruction *node_create_instruction(Node *n, Operation op);

void init_node_module(PyObject* module);
PyObject* create_instruction_instance();
PyObject* create_node_instance();
#endif
