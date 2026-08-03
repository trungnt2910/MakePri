#include <chrono>
#include <functional>
#include <thread>

#include <processthreadsapi.h>
#include <synchapi.h>

extern "C" DWORD WINAPI GetCurrentThreadId() { return static_cast<DWORD>(std::hash<std::thread::id>()(std::this_thread::get_id())); }

extern "C" void WINAPI Sleep(const DWORD milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
