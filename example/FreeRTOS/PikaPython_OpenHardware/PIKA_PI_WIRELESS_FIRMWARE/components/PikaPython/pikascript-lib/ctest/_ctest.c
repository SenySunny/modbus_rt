#include "_ctest.h"
#include <stdio.h>

pika_float _ctest_add(PikaObj *self, int a, int b, pika_float c){
    return a + b + c;
}

void _ctest_test_list(PikaObj *self, PikaObj* l){
    int len = pikaList_getSize(l);
    for(int i = 0; i < len; i++){
        if (ARG_TYPE_INT ==  pikaList_getType(l, i)){
            int v = pikaList_getInt(l, i);
            printf("%d\n", v);
        }
        if (ARG_TYPE_FLOAT ==  pikaList_getType(l, i)){
            pika_float v = pikaList_getFloat(l, i);
            printf("%f\n", v);
        }
    }
}

int32_t _test_dict_foreach(PikaObj* self,
                                               Arg* keyEach,
                                               Arg* valEach,
                                               void* context){
    char* key_str = arg_getStr(keyEach);
    if (ARG_TYPE_INT == arg_getType(valEach)){
        int key_int = arg_getInt(valEach);
        printf("key_str=%s, key_int=%d\n", key_str, key_int);
    }
    if (ARG_TYPE_FLOAT == arg_getType(valEach)){
        float key_float = arg_getFloat(valEach);
        printf("key_str=%s, key_float=%f\n", key_str, key_float);
    }
    return 0;
}

void _ctest_test_dict(PikaObj *self, PikaObj* d){
    /* 假设已经知道了key的名字 */
    int a = pikaDict_getInt(d, "a");
    float b = pikaDict_getFloat(d, "b");
    printf("a=%d, b=%f\n", a, b);
    /* 如果不知道,对所有的成员进行遍历 */
    pikaDict_forEach(d, _test_dict_foreach, NULL);
}

PikaObj* _ctest_list_return(PikaObj *self){
    /* v1.12.5 */
    
    /* 创建 list */
    PikaObj* mylist = New_PikaList();
    pikaList_append(mylist, arg_newInt(1)); // int
    pikaList_append(mylist, arg_newFloat(2.0)); // float
    pikaList_append(mylist, arg_newStr("3")); // str
    return mylist;
}

PikaObj* _ctest_dict_return(PikaObj *self){
    /* 创建 dict */
    PikaObj* mydict = New_PikaDict();
    pikaDict_set(mydict, "a", arg_newInt(1));
    pikaDict_set(mydict, "b", arg_newFloat(2.0));
    return mydict;
}

Arg* _ctest_test_any(PikaObj *self, Arg* a){
    if (ARG_TYPE_INT == arg_getType(a)){
        int v = arg_getInt(a);
        printf("int: %d\n", v);
    }
    if (ARG_TYPE_FLOAT == arg_getType(a)){
        float v = arg_getFloat(a);
        printf("float: %f\n", v);
    }
    if (ARG_TYPE_STRING == arg_getType(a)){
        char* v = arg_getStr(a);
        printf("str: %s\n", v);
    }
    return arg_newInt(0);
}

void _ctest_test_args(PikaObj *self, PikaTuple* _args){
    int len = pikaTuple_getSize(_args);
    for(int i = 0; i < len; i++){
        if(ARG_TYPE_INT == pikaTuple_getType(_args, i)){
            int v = pikaTuple_getInt(_args, i);
            printf("int: %d\n", v);
        }
        if(ARG_TYPE_FLOAT == pikaTuple_getType(_args, i)){
            float v = pikaTuple_getFloat(_args, i);
            printf("float: %f\n", v);
        }
        if(ARG_TYPE_STRING == pikaTuple_getType(_args, i)){
            char* v = pikaTuple_getStr(_args, i);
            printf("str: %s\n", v);
        }
    }
}

void _ctest_test_kwargs(PikaObj *self, PikaDict* _kwargs){
    printf("kwargs:\n");
    printf("a: %lld\n", pikaDict_getInt(_kwargs, "a"));
    printf("b: %f\n", pikaDict_getFloat(_kwargs, "b"));
}

Arg* _ctest_get_module_var(PikaObj *self, char* name){
    /* 全局函数的 self 指向的是当前模块 */
    return obj_getArg(self, name); // 得到当前模块的一个变量
}
