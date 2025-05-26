#include "tsk_tempfile.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATH_SEP '/'
#endif

char* tsk_make_tempfile() {
    std::string temp_dir;

#ifdef _WIN32
    const char* env = std::getenv("TEMP");
    if (!env) env = std::getenv("TMP");
    temp_dir = env ? env : ".";
#else
    const char* env = std::getenv("TMPDIR");
    temp_dir = env ? env : "/tmp";
#endif

    std::string filename = "tsk_tempfile_" + std::to_string(std::time(nullptr));
#ifdef _WIN32
    filename += "_" + std::to_string(GetTickCount64());
#else
    filename += "_" + std::to_string(getpid());
#endif
    filename += ".txt";

    std::string full_path = temp_dir + PATH_SEP + filename;

    // Allocate and return a heap copy
    char* c_path = (char*)malloc(full_path.size() + 1);
    std::strcpy(c_path, full_path.c_str());
    return c_path;
}
