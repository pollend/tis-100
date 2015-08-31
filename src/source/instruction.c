#include "node.h"
#include "instruction.h"
#include "util.h"

#include <stddef.h>
#include <string.h>


/*Instruction to string*/
static char* convert_location_str(FieldType loc_type,union Field location)
{
  if(loc_type == NUMBER)
  {

    char* out = (char*)malloc(sizeof(char)*3+1);
    if(sprintf(out,"%u",location.number))
    {
      return (char*)out;
    }
  }
  else
  {
    switch(location.direction)
    {
      case UP:
       return str_dump("UP");
      case RIGHT:
       return str_dump("RIGHT");
      case DOWN:
       return str_dump("DOWN");
      case LEFT:
       return str_dump("LEFT");
      case NIL:
       return str_dump("NIL");
      case ACC:
       return str_dump("ACC");
      case ANY:
       return str_dump("ANY");
      case LAST:
       return str_dump("LAST");
    }
  }

  return str_dump("");
}


char* convert_instruction_str(Instruction* instruction)
{
  char* src_str = convert_location_str(instruction->first_type,instruction->first);
  char* dest_str = convert_location_str(instruction->second_type,instruction->second);
  
  char* combined = (char*)malloc(2+strlen(src_str)+ strlen(dest_str));
  strcpy(combined,src_str);
  strcat(combined, " ");
  strcat(combined, dest_str);


  char* out = NULL;

  switch(instruction->operation)
  {
    //CALL <LABEL> <LABEL>
    case MOV:
      out = combine_str("MOV ", combined);
    break;

    //CALL
    case SAV:
       out = str_dump("SAV");
    break;
    case SWP:
      out = str_dump("SWP");
    break;
    case NOP:
      out = str_dump("NOP");
    break;
    case NEG:
      out = str_dump("NEG");
    break;

    //CALL <LABEL>
    case SUB:
      out = combine_str("SUB ", dest_str);
      
    break;
    case ADD:
      out = combine_str("ADD ", dest_str);
    break;
    case JEZ:
      
      out = combine_str("JEZ ", dest_str);
    break;
    case JMP:
      out = combine_str("JMP ", dest_str);
    break;
    case JNZ:
      out = combine_str("JNZ ", dest_str);
    break;
    case JGZ:
      out = combine_str("JGZ ", dest_str);
    break;
    case JLZ:
      out = combine_str("SUB ", dest_str);
    break;
    break;
    default:

    break;
  }

  free(combined);
  free(src_str);
  free(dest_str);
  return out;
}

/*
Instruction *node_create_instruction(Node *n, Operation op) {
  assert(n->instruction_count < MAX_INSTRUCTIONS);

  Instruction* i = (Instruction*)create_instruction_instance();
  i->operation = op;
  PyList_Append(n->instructions,(PyObject*)i);
  return i;
}*/

static void parse_field(char *s, union Field *field,FieldType *type) {
  if (strcmp(s, "UP") == 0) {
    *type = ADDRESS;
    (*field).direction = UP;
  } else if (strcmp(s, "DOWN") == 0) {
    *type = ADDRESS;
    (*field).direction = DOWN;
  } else if (strcmp(s, "LEFT") == 0) {
    *type = ADDRESS;
    (*field).direction = LEFT;
  } else if (strcmp(s, "RIGHT") == 0) {
    *type = ADDRESS;
    (*field).direction = RIGHT;
  } else if (strcmp(s, "ACC") == 0) {
    *type = ADDRESS;
    (*field).direction = ACC;
  } else if (strcmp(s, "NIL") == 0) {
    *type = ADDRESS;
    (*field).direction = NIL;
  } else if (strcmp(s, "ANY") == 0) {
    *type = ADDRESS;
    (*field).direction = ANY;
  } else if (strcmp(s, "LAST") == 0) {
    *type = ADDRESS;
    (*field).direction = LAST;
  } else {
    int value = atoi(s);
    if(value == 0)
    {
       *type = NUMBER;
      (*field).number = atoi(s);
    }
    else
    {

      *type = NAME;
      (*field).name_field.name = str_dump(s);
      (*field).name_field.ip = 0;
    }
  }
}

static Operation parse_operation(const char *input)
{
  if(strcmp(input,"MOV") == 0)
    return MOV;
  else if(strcmp(input,"SAV") == 0)
    return SAV;
  else if(strcmp(input,"SWP") == 0)
    return SWP;
  else if(strcmp(input,"SUB") == 0)
    return SUB;
  else if(strcmp(input,"ADD") == 0)
    return ADD;
  else if(strcmp(input,"NOP") == 0)
    return NOP;
  else if(strcmp(input,"NEG") == 0)
    return NEG;
  else if(strcmp(input,"JEZ") == 0)
    return JEZ;
  else if(strcmp(input,"JNZ") == 0)
    return JNZ;
  else if(strcmp(input,"JGZ") == 0)
    return JGZ;
  else if(strcmp(input,"JLZ") == 0)
    return JLZ;
  else if(strcmp(input,"JRO") == 0)
    return JRO;
  else if(strcmp(input,"OUT") == 0)
    return OUT;
  else if(strcmp(input,"JMP") == 0)
    return JMP;
  else if(input[strlen(input)-1] == ':')
    return JMP_FLAG;
  return NONE;
    
}

bool parse_line(Instruction* instruction, char* input)
{
  char* line = strdup(input);
  char* ptr = line;

  char* operator =  strsep(&ptr," ");
  Operation op = parse_operation(operator);
  switch(op)
  {
    case NONE:
     // free(line);
     // return false;
    break;

    case JMP_FLAG:
     instruction->operation = op;

    break;

    case SAV:
    case SWP:
    case NOP:
    case NEG:
    case OUT:
    instruction->operation = op;
    break;

    case SUB:
    case ADD:
    case JEZ:
    case JMP:
    case JNZ:
    case JGZ:
    case JLZ:
    case JRO:
     instruction->operation = op;
     parse_field(strsep(&ptr," "), &instruction->first,&instruction->first_type);
    break;
    
    
    case MOV:
     instruction->operation = op;

      parse_field(strsep(&ptr," "), &instruction->first,&instruction->first_type);
      parse_field(strsep(&ptr," "), &instruction->second,&instruction->second_type);
    break;
    
  };
  free(line);
  return true;
}

void parse_instruction_single(Instruction* instruction,Operation op, char* field_one)
{
  instruction->operation = op;
  parse_field(field_one, &instruction->first,&instruction->first_type);
}

void parse_instruction_two(Instruction* instruction, Operation op, char* field_one,char* field_two)
{
  instruction->operation = op;
  parse_field(field_one, &instruction->first,&instruction->first_type);
  parse_field(field_two, &instruction->second,&instruction->second_type);

}



static PyObject* py_instruction_parse_line(PyObject* self,PyObject* args)
{

  char* input = NULL;
  if(!PyArg_ParseTuple(args,"s",&input))
    return NULL;

  parse_line((Instruction*)self,input);

  Py_RETURN_NONE;
}

static PyObject * py_instruction_str (PyObject* self) {

  char* results = convert_instruction_str( (Instruction*)self);
  PyObject* output = PyString_FromFormat("%s",results);
  free(results);
  return output;
}


static void py_instruction_dealloc(PyObject *self)
{
   custom_log("started freeing Instruction");
   Py_XDECREF(((Node*)self)->instructions);
   self->ob_type->tp_free(self);
   custom_log("finished freeing Instruction");

}

static int py_instruction_init(Instruction *self, PyObject *args, PyObject *kwds)
{
  char* input = NULL;
  if(!PyArg_ParseTuple(args,"|s",&input))
    return NULL;

  return 0;
}

static PyObject* py_instruction_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Instruction* self;
  self = (Instruction*)type->tp_alloc(type,0);

  return (PyObject*)self;  
}


/* Method table */
static PyMethodDef py_instruction_method[] = {
  {"parse instruction",(PyCFunction)py_instruction_parse_line,METH_VARARGS,"parse single line"},
  {NULL},
  
};

static PyMemberDef py_instruction_members[] = {
    {NULL}  /* Sentinel */
};


static PyTypeObject instruction_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "instruction",                            /*tp_name*/
    sizeof(Instruction),                      /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)py_instruction_dealloc,         /*tp_dealloc*/
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
    py_instruction_str,                                        /*tp_str*/
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
    py_instruction_method,                       /*tp_methods*/
    py_instruction_members,                      /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)py_instruction_init,               /*tp_init*/
    0,                                        /*tp_alloc*/
    py_instruction_new,                          /*tp_new*/
};


PyObject* create_instruction_instance()
{
   return PyObject_CallObject((PyObject *)&instruction_type,NULL);
}

void init_instruction_module(PyObject* module)
{
    if (PyType_Ready(&instruction_type) < 0)
   {
      custom_log("not ready");
      return;
   }

   Py_INCREF(&instruction_type);
   PyModule_AddObject(module, "Instruction", (PyObject *)&instruction_type);

}