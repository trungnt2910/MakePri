#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

#include <oleauto.h>

#include <uni_algo/conv.h>

#include "internal/strings.h"

namespace
{
struct BstrHeader
{
    UINT byteLength;
};

BstrHeader* BstrAllocation(BSTR const string)
{
    return reinterpret_cast<BstrHeader*>(reinterpret_cast<BYTE*>(string) - sizeof(BstrHeader));
}
} // namespace

extern "C" BSTR WINAPI SysAllocString(const wchar_t* const source)
{
    return source == nullptr ? nullptr : SysAllocStringLen(source, static_cast<UINT>(win32_compat::WideView(source).size()));
}

extern "C" BSTR WINAPI SysAllocStringLen(const wchar_t* const source, const UINT length)
{
    auto* const allocation =
        static_cast<BstrHeader*>(std::malloc(sizeof(BstrHeader) + (static_cast<std::size_t>(length) + 1) * sizeof(char16_t)));
    if (allocation == nullptr)
    {
        return nullptr;
    }
    allocation->byteLength = length * sizeof(char16_t);
    auto* const string = reinterpret_cast<char16_t*>(allocation + 1);
    if (source != nullptr)
    {
        std::copy_n(reinterpret_cast<const char16_t*>(source), length, string);
    }
    else
    {
        std::fill_n(string, length, u'\0');
    }
    string[length] = u'\0';
    return reinterpret_cast<BSTR>(string);
}

extern "C" void WINAPI SysFreeString(BSTR const string)
{
    if (string != nullptr)
    {
        std::free(BstrAllocation(string));
    }
}

extern "C" UINT WINAPI SysStringLen(BSTR const string)
{
    return string == nullptr ? 0 : BstrAllocation(string)->byteLength / sizeof(char16_t);
}

extern "C" void WINAPI VariantInit(VARIANT* const value)
{
    if (value != nullptr)
    {
        std::memset(value, 0, sizeof(*value));
    }
}

extern "C" HRESULT WINAPI VariantClear(VARIANT* const value)
{
    if (value == nullptr)
    {
        return E_INVALIDARG;
    }
    if (value->vt == VT_BSTR)
    {
        SysFreeString(value->bstrVal);
    }
    else if (value->vt == VT_UNKNOWN && value->punkVal != nullptr)
    {
        value->punkVal->Release();
    }
    else if (value->vt == VT_DISPATCH && value->pdispVal != nullptr)
    {
        value->pdispVal->Release();
    }
    VariantInit(value);
    return S_OK;
}

extern "C" HRESULT WINAPI VariantChangeTypeEx(VARIANT* const destination, VARIANT* const source, DWORD, USHORT, const VARTYPE type)
{
    if (destination == nullptr || source == nullptr)
    {
        return E_INVALIDARG;
    }
    if (type != VT_R8 || source->vt != VT_BSTR)
    {
        return E_NOTIMPL;
    }

    const std::string value = una::utf16to8<char16_t, char>(win32_compat::WideView(source->bstrVal));
    const double converted = std::strtod(value.c_str(), nullptr);
    if (destination != source)
    {
        VariantClear(destination);
    }
    destination->vt = VT_R8;
    destination->dblVal = converted;
    return S_OK;
}
