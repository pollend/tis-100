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

  int cycle_count;

  struct _Node *output_port, *last;
  short output_value;

  struct _Node *ports[4];
} Node;

typedef struct _ReadResult {
  int blocked;
  short value;
} ReadResult;

void node_up_cycle(Node* n);
void node_tick(Node *n);

int node_write(Node *n, LocationDirection dir, short value);
void node_advance(Node *n);

void node_parse(Node* node,char* input);

void append_node(LocationDirection direction, Node* from, Node* to);
void append_instruction(Node *n,Instruction* instruction);

void init_node_module(PyObject* module);
PyObject* create_node_instance();

#endif
