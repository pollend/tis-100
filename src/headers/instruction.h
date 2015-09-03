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
  ADDRESS,
  NAME
} FieldType;

typedef enum { UP, RIGHT, DOWN, LEFT, NIL, ACC, ANY, LAST ,NUMBER} LocationDirection;

typedef struct _Name
{
  char* name;
  int ip;
}Name;

typedef struct  _Address
{
  short number;
  LocationDirection direction;
}Address;

typedef union _Field {
  Address address; 
  Name name;
} Field;



typedef struct _Instruction {
  PyObject_HEAD 
  
  Operation operation;

  int number_fields;
  FieldType* field_types;
  union _Field* fields;

} Instruction;

/** 
* converts an instruction to a char*.
* some description.
*/
char* convert_instruction_str(Instruction* instruction);

PyObject* create_instruction_instance();
void init_instruction_module(PyObject* module);

bool parse_line(Instruction* instruction, char* input);

void set_field_type(int index,Instruction* inst,FieldType type);
void create_fields(int number_fields,Instruction* inst);
Field* get_field(int index);

#endif
