#include <winerror.h>
#include <winreg.h>

extern "C" LSTATUS WINAPI RegGetValueW(HKEY, LPCWSTR, LPCWSTR, DWORD, LPDWORD, PVOID, LPDWORD) { return ERROR_FILE_NOT_FOUND; }
