#include "_ctest_Test.h"

void _ctest_Test_print(PikaObj *self, char* name){
    Arg* arg = obj_getArg(self, name);
    if(ARG_TYPE_INT == arg_getType(arg)){
        int v = arg_getInt(arg);
        printf("int: %d\n", v);
    }
    if(ARG_TYPE_FLOAT == arg_getType(arg)){
        float v = arg_getFloat(arg);
        printf("float: %f\n", v);
    }
    if(ARG_TYPE_STRING == arg_getType(arg)){
        char* v = arg_getStr(arg);
        printf("str: %s\n", v);
    }
}
