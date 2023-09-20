#include "pika_config.h"
#include <stdio.h>
#include "pikaScript.h"
#include "PikaVM.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#elif defined(__linux) || PIKA_LINUX_COMPATIBLE
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "unistd.h"
#endif

int main(int argc, char *argv[])
{
    PikaObj* pikaMain = NULL;
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
#ifdef _WIN32
            mkdir((const char *)dir_path);
#elif defined(__linux) || PIKA_LINUX_COMPATIBLE
            mkdir((const char *)dir_path, 0775);
#endif
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
    return 0;
}
