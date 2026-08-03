#include <cstdlib>

#include <securitybaseapi.h>

extern "C" PVOID WINAPI FreeSid(PSID sid)
{
    std::free(sid);
    return nullptr;
}
