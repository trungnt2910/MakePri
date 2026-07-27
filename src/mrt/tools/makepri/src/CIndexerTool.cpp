#include "StdAfx.h"

using Microsoft::Resources::DefStatusEx;
using Microsoft::Resources::Tools::MakePri::FileOperations;
using Microsoft::Resources::Tools::MakePri::InputArgs;
using Microsoft::Resources::Tools::MakePri::Log;
using Microsoft::Resources::Tools::MakePri::ParameterParser;
using Microsoft::Resources::Tools::MakePri::UsageScenario;

int wmain(const int argc, wchar_t** const argv)
{
    InputArgs inputArgs {};
    SetThreadPreferredUILanguages(MUI_CONSOLE_FILTER, nullptr, nullptr);
    std::fflush(stdout);
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    SetConsoleCtrlHandler(MakepriCtrlHandler, TRUE);
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

    const HRESULT initializeResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(initializeResult))
    {
        return initializeResult;
    }
    auto uninitialize = wil::scope_exit([] {
        Log::Flush();
        CoUninitialize();
    });

    int result = 0;
    ParameterParser parser(inputArgs);
    const HRESULT parseResult = parser.Parse(argc, argv);
    if (inputArgs.help)
    {
        Log::PrintUsage(&inputArgs);
        return result;
    }

    if (FAILED(parseResult))
    {
        if (parseResult != E_ABORT)
        {
            Log::PrintUsage(&inputArgs);
        }
        result = parseResult;
        return result;
    }

    if (inputArgs.scenario == UsageScenario::New)
    {
        result = MakePriNewVersionedPack(&inputArgs, UsageScenario::New);
    }
    else if (inputArgs.scenario == UsageScenario::Versioned)
    {
        result = MakePriNewVersionedPack(&inputArgs, UsageScenario::Versioned);
    }
    else if (inputArgs.scenario == UsageScenario::ResourcePack)
    {
        result = MakePriNewVersionedPack(&inputArgs, UsageScenario::ResourcePack);
    }
    else if (inputArgs.scenario == UsageScenario::Dump)
    {
        result = MakePriDumpInternal(&inputArgs, nullptr);
    }
    else if (inputArgs.scenario == UsageScenario::CreateConfig)
    {
        result = MakePriCreateConfigInternal(&inputArgs);
    }

    return result;
}
