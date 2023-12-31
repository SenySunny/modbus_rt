/*
 * [Warning!] This file is auto-generated by pika compiler.
 * Do not edit it manually.
 * The source code is *.pyi file.
 * More details: 
 * English Doc:
 * https://pikadoc-en.readthedocs.io/en/latest/PikaScript%20%E6%A8%A1%E5%9D%97%E6%A6%82%E8%BF%B0.html
 * Chinese Doc:
 * http://pikapython.com/doc/PikaScript%20%E6%A8%A1%E5%9D%97%E6%A6%82%E8%BF%B0.html
 */

#include "PikaMain.h"
#include <stdio.h>
#include <stdlib.h>

volatile PikaObj *__pikaMain;
PikaObj *pikaPythonInit(void){
    pika_platform_printf("======[pikapython packages installed]======\r\n");
    pika_printVersion();
    pika_platform_printf("PikaStdLib==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("_thread==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("json==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("math==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("os==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("random==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("socket==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("time==29474235ca6e78b0f579c90d4b2389cab5a7b0ea\r\n");
    pika_platform_printf("===========================================\r\n");
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    __pikaMain = pikaMain;
    extern unsigned char pikaModules_py_a[];
    obj_linkLibrary(pikaMain, pikaModules_py_a);
#if PIKA_INIT_STRING_ENABLE
    obj_run(pikaMain,
            "import PikaStdLib\n"
            "import os\n"
            "import modbus_rt\n"
            "import modbus_rt_defines as cst\n"
            "print('hello pikapython!')\n"
            "\n");
#else 
    obj_runModule((PikaObj*)pikaMain, "main");
#endif
    return pikaMain;
}

