/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-07     lyon       the first version
 */
#include <rtthread.h>
#include "pika_config.h"
#include <stdio.h>
#include "pikaScript.h"
#include "PikaVM.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "unistd.h"

#define DBG_TAG    "PikaScript"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

PikaObj *pikaMain = NULL;

//void pika_vm_exit_await(void) {
//    pika_vm_exit();
//    while (1) {
//        pika_platform_thread_yield();
//        if (_VMEvent_getVMCnt() == 0) {
//            return;
//        }
//    }
//}
//
//void pika_exit() {
//    if(NULL != pikaMain) {
//        rt_kprintf("pika exit.\n");
//        pika_vm_exit_await();
//
////      关闭pikaPythonShell线程操作
//
//        rt_kprintf("pika exit ok.\n");
//        pikaMain = NULL;
//    }
//}

#ifdef FINSH_USING_MSH
#include <finsh.h>
static int pika_main(int argc, char *argv[]){
    if(argc == 1)
    {
        pikaMain = pikaPythonInit();
    }
    else if(argc == 2)
    {
        Args buffs = { 0 };
        pikaMain = newRootObj("pikaMain", New_PikaMain);
        char* path = argv[1];
        char* filename = strsPathGetFileName(&buffs, path);
        size_t filename_size = strGetSize(filename);
        if (0 == strncmp(filename + filename_size - 3, ".py", 3))
        {
            Args buffs = { 0 };
            char* pwd = strsPathGetFolder(&buffs, path);
            char* dir_path = strsPathJoin(&buffs, pwd, "pikascript-api");
            mkdir((const char *)dir_path, 0775);
            pikaVM_runFile(pikaMain, path);
        }
        else if(0 == strncmp(filename + filename_size - 5, ".py.a", 5))
        {
            obj_linkLibraryFile(pikaMain, path);
            obj_runModule(pikaMain, "main");
        }
        strsDeinit(&buffs);
    }
    pikaPythonShell(pikaMain);
    obj_deinit(pikaMain);
    pikaMain = NULL;
    return 0;
}
MSH_CMD_EXPORT_ALIAS(pika_main, pikapython, run PikaScript);
#endif
