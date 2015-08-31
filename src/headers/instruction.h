#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <Python.h>
#include <stdbool.h>
#include "util.h"
#include "node.h"

typedef enum {
  NONE,

  MOV,
  SAV,
  SWP,
  SUB,
  ADD,
  NOP,
  NEG,
  JEZ,
  JNZ,
  JGZ,
  JLZ,
  JRO,
  OUT,
  JMP,     

  JMP_FLAG //flags the location for jump to
} Operation;

typedef enum {
  NUMBER,
  ADDRESS,
  NAME
} FieldType;

typedef enum { UP, RIGHT, DOWN, LEFT, NIL, ACC, ANY, LAST } LocationDirection;

typedef struct _Name
{
  char* name;
  int ip;

}Name;

union Field {
  short number;
  LocationDirection direction;
  Name name_field;
} _Field;



typedef struct _Instruction {
  PyObject_HEAD 
  
  Operation operation;

  FieldType first_type;
  union Field first;

  FieldType second_type;
  union Field second;
} Instruction;

/** 
* converts an instruction to a char*.
* some description.
*/
char* convert_instruction_str(Instruction* instruction);

PyObject* create_instruction_instance();
void init_instruction_module(PyObject* module);

bool parse_line(Instruction* instruction, char* input);
void parse_instruction_single(Instruction* instruction,Operation op, char* field_one);
void parse_instruction_two(Instruction* instruction, Operation op, char* field_one,char* field_two);


#endif
