#include "StdAfx.h"

#include <DefStatus.h>

namespace Microsoft::Resources
{
bool Def_HrFailed0(const HRESULT result, IDefStatus* const status)
{
    if (SUCCEEDED(result))
    {
        return false;
    }
    if (status != nullptr)
    {
        status->SetError(result, L"" __FILE__, 229, L"", 0);
    }
    return true;
}
} // namespace Microsoft::Resources
