#if __has_include_next(<wrl/client.h>)
#include_next <wrl/client.h>
#else

#pragma once

#include <objbase.h>

namespace Microsoft::WRL
{

template<typename Interface>
class ComPtr
{
public:
    ComPtr() = default;
    ComPtr(const ComPtr&) = delete;
    ComPtr& operator=(const ComPtr&) = delete;
    ~ComPtr() { Reset(); }

    Interface* Get() const noexcept { return value_; }
    Interface* operator->() const noexcept { return value_; }
    Interface** operator&() noexcept
    {
        Reset();
        return &value_;
    }
    explicit operator bool() const noexcept { return value_ != nullptr; }
    bool operator==(std::nullptr_t) const noexcept { return value_ == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return value_ != nullptr; }

    template<typename Other>
    HRESULT As(Other** result) const
    {
        if (value_ == nullptr || result == nullptr)
        {
            return E_POINTER;
        }
        return value_->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(result));
    }

    void Reset() noexcept
    {
        if (value_ != nullptr)
        {
            value_->Release();
            value_ = nullptr;
        }
    }

private:
    Interface* value_ {};
};

} // namespace Microsoft::WRL

#endif // __has_include_next(<wrl/client.h>)
