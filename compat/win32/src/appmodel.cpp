#include <appmodel.h>

extern "C" LONG WINAPI GetCurrentPackageFullName(UINT32*, PWSTR) { return APPMODEL_ERROR_NO_PACKAGE; }
