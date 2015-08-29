#include "node.h"
#include "instruction.h"

static void instruction_dealloc(PyObject *self)
{
   custom_log("started freeing Instruction");
   Py_XDECREF(((Node*)self)->instructions);
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
  PyObject* operation,source,dest,code;
   static char *kwlist[] = {"operation", "src", "dest","code", NULL};

  if(PyArg_ParseTupleAndKeywords(args,kwds,"|SSSS",kwlist,&operation,&source,&dest,&code))
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


static char* convert_location_str(LocationType loc_type,union Location location)
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



static PyObject * instruction_str (PyObject* self) {

  char* results = convert_instruction_str( (Instruction*)self);
  PyObject* output = PyString_FromFormat("%s",results);
  free(results);
  return output;
}

char* convert_instruction_str(Instruction* instruction)
{
  char* src_str = convert_location_str(instruction->src_type,instruction->src);
  char* dest_str = convert_location_str(instruction->dest_type,instruction->dest);
  
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
    instruction_str,                                        /*tp_str*/
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