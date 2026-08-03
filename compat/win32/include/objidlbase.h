#if __has_include_next(<objidlbase.h>)
#include_next <objidlbase.h>
#else

#pragma once

#include <unknwn.h>
#include <wtypes.h>

struct ISequentialStream : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Read(void* data, ULONG bytes, ULONG* bytesRead) = 0;
    virtual HRESULT STDMETHODCALLTYPE Write(const void* data, ULONG bytes, ULONG* bytesWritten) = 0;
};

struct IStream;
using LPSTREAM = IStream*;

struct STATSTG
{
    LPOLESTR pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
};

enum STGTY
{
    STGTY_STORAGE = 1,
    STGTY_STREAM = 2,
    STGTY_LOCKBYTES = 3,
    STGTY_PROPERTY = 4,
};

enum STREAM_SEEK
{
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR = 1,
    STREAM_SEEK_END = 2,
};

#pragma push_macro("LOCK_WRITE")
#undef LOCK_WRITE

enum LOCKTYPE
{
    LOCK_WRITE = 1,
    LOCK_EXCLUSIVE = 2,
    LOCK_ONLYONCE = 4,
};

#pragma pop_macro("LOCK_WRITE")

struct IStream : ISequentialStream
{
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* newPosition) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER newSize) = 0;
    virtual HRESULT STDMETHODCALLTYPE
    CopyTo(IStream* stream, ULARGE_INTEGER bytes, ULARGE_INTEGER* bytesRead, ULARGE_INTEGER* bytesWritten) = 0;
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Revert() = 0;
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER offset, ULARGE_INTEGER bytes, DWORD lockType) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER offset, ULARGE_INTEGER bytes, DWORD lockType) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* status, DWORD flags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream** stream) = 0;
};

DEFINE_GUID(IID_ISequentialStream, 0x0c733a30, 0x2a1c, 0x11ce, 0xad, 0xe5, 0x00, 0xaa, 0x00, 0x44, 0x77, 0x3d);
DEFINE_GUID(IID_IStream, 0x0000000c, 0x0000, 0x0000, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#endif // __has_include_next(<objidlbase.h>)
