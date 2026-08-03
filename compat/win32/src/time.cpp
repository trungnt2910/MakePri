#include <chrono>
#include <cstring>
#include <ctime>

#include <errhandlingapi.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>
#include <winerror.h>

extern "C" void WINAPI GetSystemTime(LPSYSTEMTIME const systemTime)
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t value = std::chrono::system_clock::to_time_t(now);
    const std::tm* const utc = std::gmtime(&value);
    if (utc == nullptr)
    {
        std::memset(systemTime, 0, sizeof(*systemTime));
        return;
    }
    systemTime->wYear = static_cast<WORD>(utc->tm_year + 1900);
    systemTime->wMonth = static_cast<WORD>(utc->tm_mon + 1);
    systemTime->wDayOfWeek = static_cast<WORD>(utc->tm_wday);
    systemTime->wDay = static_cast<WORD>(utc->tm_mday);
    systemTime->wHour = static_cast<WORD>(utc->tm_hour);
    systemTime->wMinute = static_cast<WORD>(utc->tm_min);
    systemTime->wSecond = static_cast<WORD>(utc->tm_sec);
    systemTime->wMilliseconds =
        static_cast<WORD>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
}

extern "C" BOOL WINAPI SystemTimeToFileTime(const SYSTEMTIME* const systemTime, FILETIME* const fileTime)
{
    using namespace std::chrono;
    const year_month_day date(year(systemTime->wYear), month(systemTime->wMonth), day(systemTime->wDay));
    if (!date.ok())
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    constexpr std::intmax_t FileTimeTicksPerSecond = 10'000'000;
    using FileTimeDuration = duration<std::int64_t, std::ratio<1, FileTimeTicksPerSecond>>;
    constexpr sys_days WindowsEpoch = year(1601) / January / 1;
    constexpr sys_days UnixEpoch = year(1970) / January / 1;
    constexpr std::uint64_t WindowsEpochOffset =
        static_cast<std::uint64_t>(duration_cast<FileTimeDuration>(UnixEpoch - WindowsEpoch).count());
    const auto sinceUnix = sys_days(date) - UnixEpoch + hours(systemTime->wHour) + minutes(systemTime->wMinute) +
                           seconds(systemTime->wSecond) + milliseconds(systemTime->wMilliseconds);
    const std::uint64_t ticks = WindowsEpochOffset + static_cast<std::uint64_t>(duration_cast<FileTimeDuration>(sinceUnix).count());
    fileTime->dwLowDateTime = static_cast<DWORD>(ticks);
    fileTime->dwHighDateTime = static_cast<DWORD>(ticks >> 32);
    return TRUE;
}
