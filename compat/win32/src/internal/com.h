#pragma once

#include <atomic>
#include <span>

#include <windows.h>

namespace win32_compat
{

class ComReferenceCounted
{
protected:
    ULONG AddReference() { return ++references_; }
    ULONG ReleaseReference()
    {
        const ULONG result = --references_;
        if (result == 0)
        {
            delete this;
        }
        return result;
    }
    virtual ~ComReferenceCounted() = default;

private:
    std::atomic<ULONG> references_ {1};
};

using ComFactory = HRESULT (*)(REFIID interfaceId, void** object);

struct ComClassRegistration
{
    const CLSID* classId;
    ComFactory factory;
};

void RegisterComClasses(std::span<const ComClassRegistration> classes);

} // namespace win32_compat
