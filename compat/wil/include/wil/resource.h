#pragma once

#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

#include <wil/result_macros.h>

namespace wistd = std;

namespace wil
{
template<typename Callable>
class scope_exit_t
{
public:
    explicit scope_exit_t(Callable callable) : m_callable(std::move(callable)) {}
    scope_exit_t(const scope_exit_t&) = delete;
    scope_exit_t& operator=(const scope_exit_t&) = delete;
    scope_exit_t(scope_exit_t&& other) noexcept : m_callable(std::move(other.m_callable)), m_active(other.m_active)
    {
        other.m_active = false;
    }
    ~scope_exit_t()
    {
        if (m_active)
            m_callable();
    }
    void release() noexcept { m_active = false; }

private:
    Callable m_callable;
    bool m_active {true};
};

template<typename Callable>
scope_exit_t<std::decay_t<Callable>> scope_exit(Callable&& callable)
{
    return scope_exit_t<std::decay_t<Callable>>(std::forward<Callable>(callable));
}

template<typename CloseType, CloseType closeFunction, typename Pointer = HANDLE, Pointer invalid = nullptr>
class unique_any
{
public:
    unique_any() noexcept = default;
    explicit unique_any(Pointer value) noexcept : m_value(value) {}
    unique_any(const unique_any&) = delete;
    unique_any& operator=(const unique_any&) = delete;
    unique_any(unique_any&& other) noexcept : m_value(other.release()) {}
    unique_any& operator=(unique_any&& other) noexcept
    {
        reset(other.release());
        return *this;
    }
    ~unique_any() { reset(); }

    Pointer get() const noexcept { return m_value; }
    explicit operator bool() const noexcept { return m_value != invalid; }
    operator Pointer() const noexcept { return m_value; }
    Pointer release() noexcept
    {
        const Pointer value = m_value;
        m_value = invalid;
        return value;
    }
    void reset(Pointer value = invalid) noexcept
    {
        if (m_value != invalid)
            closeFunction(m_value);
        m_value = value;
    }
    Pointer* put() noexcept
    {
        reset();
        return &m_value;
    }
    Pointer* addressof() noexcept { return &m_value; }
    Pointer* operator&() noexcept { return put(); }

private:
    Pointer m_value {invalid};
};

template<typename CloseType, CloseType closeFunction>
using unique_any_handle_null = unique_any<CloseType, closeFunction, HANDLE, nullptr>;

template<typename FunctionType, FunctionType function>
struct function_deleter
{
    template<typename Pointer>
    void operator()(Pointer* pointer) const noexcept
    {
        function(pointer);
    }
};

using unique_handle = unique_any<decltype(&::CloseHandle), ::CloseHandle, HANDLE, nullptr>;
using unique_hfind = unique_any<decltype(&::FindClose), ::FindClose, HANDLE, nullptr>;
using unique_hmodule = unique_any<decltype(&::FreeLibrary), ::FreeLibrary, HMODULE, nullptr>;
using unique_hlocal_security_descriptor = unique_any<decltype(&::LocalFree), ::LocalFree, HLOCAL, nullptr>;
using unique_sid = unique_any<decltype(&::FreeSid), ::FreeSid, PSID, nullptr>;
using unique_cotaskmem_string = std::unique_ptr<wchar_t, decltype(&::CoTaskMemFree)>;

namespace details
{
inline bool IsDebuggerPresent() noexcept { return false; }
} // namespace details
} // namespace wil
