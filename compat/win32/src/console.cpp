#include <algorithm>
#include <csignal>
#include <iterator>
#include <mutex>
#include <vector>

#include <consoleapi.h>
#include <errhandlingapi.h>
#include <winerror.h>

namespace
{
std::vector<PHANDLER_ROUTINE>& ControlHandlers()
{
    static std::vector<PHANDLER_ROUTINE> handlers;
    return handlers;
}

std::mutex& ControlHandlersMutex()
{
    static std::mutex mutex;
    return mutex;
}

void DispatchControlSignal(const int signal)
{
    constexpr DWORD ControlCEvent = 0;
    constexpr DWORD ControlCloseEvent = 2;
    const DWORD event = signal == SIGINT ? ControlCEvent : ControlCloseEvent;
    std::scoped_lock lock(ControlHandlersMutex());
    for (auto handler = ControlHandlers().rbegin(); handler != ControlHandlers().rend(); ++handler)
    {
        if ((*handler)(event) != FALSE)
        {
            return;
        }
    }
}

bool SetNativeSignalHandlers(void (*handler)(int))
{
    const auto previousInterrupt = std::signal(SIGINT, handler);
    if (previousInterrupt == SIG_ERR)
    {
        return false;
    }
    if (std::signal(SIGTERM, handler) == SIG_ERR)
    {
        std::signal(SIGINT, previousInterrupt);
        return false;
    }
    return true;
}
} // namespace

extern "C" BOOL WINAPI SetConsoleCtrlHandler(const PHANDLER_ROUTINE handler, const BOOL add)
{
    std::scoped_lock lock(ControlHandlersMutex());
    if (handler == nullptr)
    {
        return SetNativeSignalHandlers(add != FALSE ? SIG_IGN : SIG_DFL) ? TRUE : FALSE;
    }

    auto& handlers = ControlHandlers();
    if (add != FALSE)
    {
        handlers.push_back(handler);
        if (handlers.size() == 1 && !SetNativeSignalHandlers(DispatchControlSignal))
        {
            handlers.pop_back();
            return FALSE;
        }
        return TRUE;
    }

    const auto found = std::find(handlers.rbegin(), handlers.rend(), handler);
    if (found == handlers.rend())
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    handlers.erase(std::next(found).base());
    return (!handlers.empty() || SetNativeSignalHandlers(SIG_DFL)) ? TRUE : FALSE;
}
