#include "StdAfx.h"

#include <IFsIndexer.h>

namespace Microsoft::Resources::Indexers
{

IFormatSpecificIndexer* WINAPI CFsiFactory::s_GetIndexer(const wchar_t* const type, IDefStatusEx* const status, FSIList_Group* const group)
{
    IFormatSpecificIndexer* indexer = nullptr;
    *group = FSI_MainGroup;
    if (type != nullptr)
    {
        if (DefString_CompareWithOptions(type, L"resfiles", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CResFilesIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"folder", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CFolderIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"resw", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CResxIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"resjson", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CResJsonIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"priinfo", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CPriInfoIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"pri", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CPriFileIndexer();
        }
        else if (DefString_CompareWithOptions(type, L"embedfiles", DefCompare_CaseInsensitive) == 0)
        {
            indexer = new (std::nothrow) CEmbedFilesIndexer();
            *group = FSI_FinalGroup;
        }
    }

    if (indexer == nullptr)
    {
        status->SetError(E_DEF_FSI_INSTANTIATION_FAILED, type);
    }
    return indexer;
}

} // namespace Microsoft::Resources::Indexers
