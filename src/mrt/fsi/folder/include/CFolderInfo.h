#pragma once

#include <cstdint>

#include <windows.h>

#include <string>
#include <vector>

namespace Microsoft::Resources
{
class IDefStatusEx;
}

namespace Microsoft::Resources::Indexers
{
class IIndexOptions;

enum class FOLDER_FILE_ATTRIBUTES : std::uint32_t
{
    SPECIAL_NONE = 0,
    JUNCTION_POINT = 0x01,
    DFS = 0x02,
    SYMBOLIC_LINK = 0x04,
    REMOTE_STORAGE = 0x08,
    VOLUME_MOUNT = 0x10,
    REPARSE_POINT = 0x20,
    HIDDEN_FILE = 0x40,
};

class CFileInfo
{
public:
    HRESULT Set(const wchar_t* pFilePath, DWORD ulAttribute, DWORD ulReserved, IDefStatusEx* pStatus);

    std::wstring _wszFullFilePath;
    DWORD _ulFileAttribute {};
    DWORD _ulReserved {};
};

class CFolderInfo
{
public:
    struct FILE_ATTRIUBTE
    {
        std::wstring wszFileName;
        DWORD ulAttribute {};
        DWORD ulReserved {};
    };

    CFolderInfo();
    ~CFolderInfo();

    HRESULT Set(const wchar_t* pFolder, IDefStatusEx* pStatus);
    HRESULT Set(const wchar_t* pFolder, DWORD ulAttribute, DWORD ulReserved, IDefStatusEx* pStatus);
    HRESULT TraverseChildren();

    static HRESULT IsSpecialFileOrFolderToIgnore(
        DWORD attributes,
        DWORD reserved,
        const IIndexOptions* options,
        FOLDER_FILE_ATTRIBUTES* specialAttributes);

    HRESULT GetCFolderInfo(std::uint32_t index, CFolderInfo* pCFolderInfo, IDefStatusEx* pStatus) const;
    HRESULT GetCFileInfo(std::uint32_t index, CFileInfo* pCFileInfo, IDefStatusEx* pStatus) const;

    std::wstring _wszFolder;
    std::wstring _wszFolderAbsolutePath;
    std::vector<FILE_ATTRIUBTE>* _pFolderList;
    std::vector<FILE_ATTRIUBTE>* _pFileList;
    DWORD _ulFolderAttribute;
    DWORD _ulReserved;
};
} // namespace Microsoft::Resources::Indexers
