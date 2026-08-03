#include <algorithm>
#include <cstdlib>
#include <mutex>
#include <vector>

#include <combaseapi.h>

#include "internal/com.h"

namespace
{
std::vector<win32_compat::ComClassRegistration>& RegisteredClasses()
{
    static std::vector<win32_compat::ComClassRegistration> classes;
    return classes;
}

std::mutex& RegistrationMutex()
{
    static std::mutex mutex;
    return mutex;
}
} // namespace

void win32_compat::RegisterComClasses(const std::span<const ComClassRegistration> classes)
{
    std::scoped_lock lock(RegistrationMutex());
    for (const ComClassRegistration& registration : classes)
    {
        auto& registered = RegisteredClasses();
        const auto match = std::find_if(registered.begin(), registered.end(), [&](const ComClassRegistration& existing) {
            return *existing.classId == *registration.classId;
        });
        if (match == registered.end())
        {
            registered.push_back(registration);
        }
        else
        {
            *match = registration;
        }
    }
}

extern "C" LPVOID WINAPI CoTaskMemAlloc(const SIZE_T bytes) { return std::malloc(bytes); }
extern "C" void WINAPI CoTaskMemFree(LPVOID const memory) { std::free(memory); }

extern "C" HRESULT WINAPI CoInitializeEx(LPVOID, DWORD) { return S_OK; }
extern "C" void WINAPI CoUninitialize() {}

extern "C" HRESULT WINAPI CoCreateInstance(REFCLSID classId, IUnknown*, DWORD, REFIID interfaceId, LPVOID* const object)
{
    if (object == nullptr)
    {
        return E_POINTER;
    }
    *object = nullptr;

    win32_compat::ComFactory factory = nullptr;
    {
        std::scoped_lock lock(RegistrationMutex());
        const auto& registered = RegisteredClasses();
        const auto match = std::find_if(registered.begin(), registered.end(), [&](const win32_compat::ComClassRegistration& entry) {
            return *entry.classId == classId;
        });
        if (match != registered.end())
        {
            factory = match->factory;
        }
    }

    return factory == nullptr ? CLASS_E_CLASSNOTAVAILABLE : factory(interfaceId, object);
}
