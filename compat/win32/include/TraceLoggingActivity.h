#if __has_include_next(<TraceLoggingActivity.h>)
#include_next <TraceLoggingActivity.h>
#else

#pragma once

#include <TraceLoggingProvider.h>

#define _tlgActivityDecl(activity)
#define _tlgActivityRef(activity) (activity)

template<typename DerivedType, auto keyword, auto level>
class _TlgActivityBase
{
public:
    static constexpr auto Keyword = keyword;
    static constexpr auto Level = level;

    _TlgActivityBase() = default;
    _TlgActivityBase(_TlgActivityBase&&) = default;
    _TlgActivityBase& operator=(_TlgActivityBase&&) = default;

    bool IsStarted() const { return false; }

    decltype(nullptr) Id() const { return nullptr; }

    decltype(nullptr) zInternalRelatedId() const { return nullptr; }

    void zInternalStart() {}

    void zInternalStop() {}

protected:
    template<typename RelatedId>
    void SetRelatedId(const RelatedId&)
    {}

    decltype(nullptr) GetRelatedId() const { return nullptr; }

    void PushThreadActivityId() {}

    void PopThreadActivityId() {}
};

#endif // __has_include_next(<TraceLoggingActivity.h>)
