#include "StdAfx.h"

#include <ParameterManager.h>

namespace Microsoft::Resources
{
const OptionsBase::OptionSpec OutputOptions::s_options[14] {
    {L"priheader", L"ph", 0x0001, 0x0000, 0x0001},
    {L"indexes", L"ix", 0x0002, 0x0000, 0x0002},
    {L"uris", L"uri", 0x0004, 0x0000, 0x0004},
    {L"qualifierinfo", L"qi", 0x0008, 0x0000, 0x0008},
    {L"resourcemaps", L"rm", 0x0010, 0x0000, 0x0010},
    {L"versioninfo", L"vi", 0x0020, 0x0000, 0x0020},
    {L"emptysubtrees", L"es", 0x0040, 0x0000, 0x0040},
    {L"emptynamedresources", L"enr", 0x0080, 0x0000, 0x0080},
    {L"links", L"l", 0x0100, 0x0000, 0x0100},
    {L"namedresourcedecision", L"nrd", 0x0200, 0x0000, 0x0200},
    {L"candidates", L"c", 0x0400, 0x0000, 0x0400},
    {L"linkedcandidates", L"lc", 0x0800, 0x0800, 0x0000},
    {L"sanitizexml", L"sxml", 0x1000, 0x1000, 0x0000},
    {L"rawlocators", L"rl", 0x2000, 0x2000, 0x0000},
};

OutputOptions::OutputOptions() : OptionsBase(0, 14, s_options, 0, nullptr) {}
} // namespace Microsoft::Resources
