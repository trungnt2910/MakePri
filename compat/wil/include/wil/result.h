#pragma once

#include <exception>

#include <wil/result_macros.h>

namespace wil
{
template<typename Callable>
HRESULT ResultFromException(Callable&& callable) noexcept
{
    try
    {
        callable();
        return S_OK;
    }
    catch (HRESULT result)
    {
        return result;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }
}
} // namespace wil
