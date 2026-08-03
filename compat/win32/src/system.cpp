#include <fileapi.h>
#include <winbase.h>

extern "C" UINT WINAPI GetDriveTypeW(LPCWSTR) { return DRIVE_FIXED; }
