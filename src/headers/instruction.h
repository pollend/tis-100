#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <Python.h>
#include "util.h"
#include "node.h"

typedef enum {
  MOV,
  SAV,
  SWP,
  SUB,
  ADD,
  NOP,
  NEG,
  JEZ,
  JMP,
  JNZ,
  JGZ,
  JLZ,
  JRO,
  OUT
} Operation;

typedef enum {
  NUMBER,
  ADDRESS,
} LocationType;

typedef enum { UP, RIGHT, DOWN, LEFT, NIL, ACC, ANY, LAST } LocationDirection;

union Location {
  short number;
  LocationDirection direction;
} _Location;

typedef struct _Instruction {
  PyObject_HEAD 
  
  Operation operation;
  LocationType src_type;
  union Location src;

  LocationType dest_type;
  union Location dest;
} Instruction;

PyObject* create_instruction_instance();
void init_instruction_module(PyObject* module);
char* convert_instruction_str(Instruction* instruction);

#endif
