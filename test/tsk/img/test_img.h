#ifndef TEST_IMG_H
#define TEST_IMG_H

#include "tsk/base/tsk_os_cpp.h"        // needed for TSK_TSTRING


#ifdef TSK_WIN32_WIDE
inline std::wstring prepend_test_data_dir(const wchar_t* relative_path) {
    const wchar_t* base = _wgetenv(L"srcdir");
    if (!base) throw std::runtime_error("srcdir");
    std::wstring full = base;
    full += L"/test/data/";
    full += relative_path;
    return full;
}
#else
inline std::string prepend_test_data_dir(const char* relative_path) {
    const char* base = getenv("srcdir");
    if (!base) throw std::runtime_error("srcdir");
    std::string full = base;
    full += "/test/data/";
    full += relative_path;
    return full;
}
#endif

#ifdef TSK_WIN32
inline void fix_slashes_for_windows(TSK_TSTRING &path) {
    std::replace(path.begin(), path.end(), _TSK_T('/'), _TSK_T('\\'));
}
#else
inline void fix_slashes_for_windows(TSK_TSTRING &) {}
#endif
#endif
