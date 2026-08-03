#if __has_include_next(<comutil.h>)
#include_next <comutil.h>
#else

#pragma once

#include <oleauto.h>

class _bstr_t
{
public:
    explicit _bstr_t(const wchar_t* value) : value_(SysAllocString(value)) {}
    _bstr_t(const _bstr_t& other) : value_(SysAllocString(other.value_)) {}
    ~_bstr_t() { SysFreeString(value_); }
    operator BSTR() const { return value_; }

private:
    BSTR value_;
};

class _variant_t : public VARIANT
{
public:
    _variant_t() { VariantInit(this); }
    ~_variant_t() { VariantClear(this); }
    operator LONG() const { return lVal; }
    operator bool() const { return vt == VT_BOOL ? boolVal != VARIANT_FALSE : lVal != 0; }
    operator double() const { return vt == VT_R8 ? dblVal : static_cast<double>(lVal); }
};

#endif // __has_include_next(<comutil.h>)
