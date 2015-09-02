#include "node.h"
#include "instruction.h"
#include "util.h"

#include <stddef.h>
#include <string.h>





/*
Instruction *node_create_instruction(Node *n, Operation op) {
  assert(n->instruction_count < MAX_INSTRUCTIONS);

  Instruction* i = (Instruction*)create_instruction_instance();
  i->operation = op;
  PyList_Append(n->instructions,(PyObject*)i);
  return i;
}*/

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

static void parse_field(char *s, Instruction* inst, int index) {
  if(inst->field_types[index] == ADDRESS)
  {
      if (strcmp(s, "UP") == 0) {
      inst->fields[index].address.direction = UP;
    } else if (strcmp(s, "DOWN") == 0) {
      inst->fields[index].address.direction = DOWN;
    } else if (strcmp(s, "LEFT") == 0) {
      inst->fields[index].address.direction = LEFT;
    } else if (strcmp(s, "RIGHT") == 0) {
      inst->fields[index].address.direction = RIGHT;
    } else if (strcmp(s, "ACC") == 0) {
      inst->fields[index].address.direction = ACC;
    } else if (strcmp(s, "NIL") == 0) {
      inst->fields[index].address.direction = NIL;
    } else if (strcmp(s, "ANY") == 0) {
      inst->fields[index].address.direction = ANY;
    } else if (strcmp(s, "LAST") == 0) {
      inst->fields[index].address.direction = LAST;
    } else {

      inst->fields[index].address.direction = NUMBER;
      inst->fields[index].address.number = atoi(s);
    }

  }
  else
  {
      if(s[strlen(s)-1] == ':')
      {
        char* output = strdup(s);
        output[strlen(output)-1] = '\0';
        inst->fields[index].name.name =output;
      }
      else
      {
       inst->fields[index].name.name = strdup(s);
      }
      inst->fields[index].name.ip = -1;
  }

}

static inline void create_fields(int number_fields,Instruction* inst)
{
  inst->fields = malloc(sizeof(Field) * number_fields);
  inst->field_types = malloc(sizeof(FieldType) * number_fields);
  inst->number_fields = number_fields;

}

static inline void set_field_type(int index,Instruction* inst,FieldType type)
{
  inst->field_types[index] = type;
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
        instruction->operation = NONE;
      break;

      case JMP_FLAG:
       instruction->operation = op;
       create_fields(1,instruction);
       set_field_type(0,instruction,NAME);
       parse_field(operator,instruction,0);
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
      case JRO:
       instruction->operation = op;
        create_fields(1,instruction);
        set_field_type(0,instruction,ADDRESS);
        parse_field(strsep(&ptr," "),instruction,0);
      break;

      case JEZ:
      case JMP:
      case JNZ:
      case JGZ:
      case JLZ:
       instruction->operation = op;

       create_fields(1,instruction);

       set_field_type(0,instruction,NAME);
       parse_field(strsep(&ptr," "),instruction,0);

      break;
      
      
      case MOV:
       instruction->operation = op;
        
        create_fields(2,instruction);

        set_field_type(0,instruction,ADDRESS);
        parse_field(strsep(&ptr," "),instruction,0);


        set_field_type(1,instruction,ADDRESS);
        parse_field(strsep(&ptr," "),instruction,1);

      break;
      
    };
    free(line);
  
  return true;
}

 /*Instruction to string*/
static char** convert_fields_str(Instruction* inst)
{
  char** items = NULL;
  items = malloc(sizeof(char*) * inst->number_fields);

  for (int i = 0; i < inst->number_fields; ++i)
  {
    switch(inst->field_types[i])
    {
      case ADDRESS:
        switch(inst->fields[i].address.direction)
        {
           case UP:
           items[i] =  strdup("UP");
           break;
          case RIGHT:
           items[i] = strdup("RIGHT");
           break;
          case DOWN:
           items[i] = strdup("DOWN");
           break;
          case LEFT:
           items[i] = strdup("LEFT");
           break;
          case NIL:
           items[i] = strdup("NIL");
           break;
          case ACC:
           items[i] = strdup("ACC");
           break;
          case ANY:
           items[i] = strdup("ANY");
           break;
          case LAST:
           items[i] = strdup("LAST");
           break;
          case NUMBER:
           ;
            char* out = (char*)malloc(sizeof(char)*3+1);
            if(sprintf(out,"%u",inst->fields[i].address.number))
            {
              items[i] =  (char*)out;
            }
           break;
        }
      break;
      case NAME:
        items[i] =strdup( inst->fields[i].name.name);
      break;

    }
  }

 
  return items;
}



char* convert_instruction_str(Instruction* instruction)
{
  char** field_str = convert_fields_str(instruction);
  
  char* out =malloc(sizeof(char) * 60);
  switch(instruction->operation)
  {
    //CALL <LABEL> <LABEL>
    case MOV:
    sprintf(out,"MOV %s %s",field_str[0],field_str[1]);
    break;

    //CALL
    case SAV:
       sprintf(out,"SAV");
    break;
    case SWP:
      sprintf(out,"SWP");
    break;
    case NOP:
      sprintf(out,"NOP");
    break;
    case NEG:
      sprintf(out,"NEG");
    break;

    //CALL <LABEL>
    case SUB:
      sprintf(out,"SUB %s",field_str[0]);
    break;
    case ADD:
      sprintf(out,"ADD %s",field_str[0]);
      //out = combine_str("ADD ", dest_str);
    break;
    case JEZ:    
      sprintf(out,"JEZ %s",field_str[0]);
    break;
    case JMP:
      sprintf(out,"JMP %s",field_str[0]);
    break;
    case JNZ:
      sprintf(out,"JNZ %s",field_str[0]);
    break;
    case JGZ:
      sprintf(out,"JGZ %s",field_str[0]);
    break;
    case JLZ:
     sprintf(out,"JLZ %s",field_str[0]);
    break;
    case JMP_FLAG:
    sprintf(out,"%s:",field_str[0]);
    break;
    case NONE:
    sprintf(out," ");
    break;
  }

    for (int i = 0; i < instruction->number_fields; ++i)
    {
      free(field_str[i]);
    }
    free(field_str);
  return out;
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