#if __has_include_next(<windows.h>)
#include_next <windows.h>
#else

#pragma once

// Windows SDK umbrella headers used by this codebase.

#include <sal.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <winnls.h>
#include <winreg.h>
#include <rpc.h>
#include <objbase.h>
#include <winerror.h>
#include <consoleapi.h>

#endif // __has_include_next(<windows.h>)
