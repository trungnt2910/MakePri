#pragma once

namespace wil
{
class TraceLoggingProvider
{};
} // namespace wil

#define PDT_ProductAndServicePerformance 0
#define IMPLEMENT_TRACELOGGING_CLASS(className, providerName, providerId) public:
#define DEFINE_COMPLIANT_MEASURES_EVENT(eventName, category) \
    static void eventName() noexcept {}
#define DEFINE_COMPLIANT_TELEMETRY_EVENT_PARAM3(eventName, category, type1, name1, type2, name2, type3, name3) \
    static void eventName(type1, type2, type3) noexcept {}
#define DEFINE_COMPLIANT_TELEMETRY_EVENT_PARAM4(eventName, category, type1, name1, type2, name2, type3, name3, type4, name4) \
    static void eventName(type1, type2, type3, type4) noexcept {}
#define DEFINE_COMPLIANT_MEASURES_EVENT_PARAM3 DEFINE_COMPLIANT_TELEMETRY_EVENT_PARAM3
#define DEFINE_COMPLIANT_MEASURES_EVENT_PARAM4 DEFINE_COMPLIANT_TELEMETRY_EVENT_PARAM4
