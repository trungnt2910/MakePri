#pragma once

#include <windows.h>

#include <cstddef>
#include <cstdint>

#include <list>
#include <map>
#include <string>

class CJSONLexer
{
public:
    enum tagJSON_FLAGS
    {
        JSON_ENABLECOMMENTS = 1,
    };

    enum tagTOKEN
    {
        tokenLSQUARE = 0,
        tokenRSQUARE = 1,
        tokenLBRACE = 2,
        tokenRBRACE = 3,
        tokenCOLON = 4,
        tokenCOMMA = 5,
        tokenINTEGER = 6,
        tokenFLOAT = 7,
        tokenTRUE = 8,
        tokenFALSE = 9,
        tokenNULL = 10,
        tokenSTRING = 11,
        tokenUNKNOWN = 12,
        tokenERROR = 13,
        tokenFAIL = 14,
        tokenEOF = 15,
    };

    struct JsonCharacter
    {
        JsonCharacter& operator++();
        const wchar_t* operator++(int);

        const wchar_t* pszInput {};
        std::int32_t iLineNum {1};
        std::int32_t iColumnNum {};
    };

    CJSONLexer() = default;
    ~CJSONLexer() { delete[] _szYYTextAlloc; }

    HRESULT SetInput(const wchar_t* input, std::uint32_t length);
    tagTOKEN GetToken();
    void GetPositionOfToken(int* line, int* column);
    [[nodiscard]] const wchar_t* SzYYText() const { return _szYYTextAlloc; }

protected:
    tagTOKEN ReadExponent();
    tagTOKEN ReadNumber();
    tagTOKEN ReadString();

    JsonCharacter _jsonInput;
    wchar_t* _szYYTextAlloc {};
    std::size_t _cchYYTextAlloc {};
    wchar_t* _pszYYText {};
    tagTOKEN _token {tagTOKEN::tokenUNKNOWN};
    std::int32_t _iTokenLineNumber {};
    std::int32_t _iTokenColumnNumber {};
    bool _bEnableInlineComments {true};
};

class CJsonArray;
class CJsonObject;

class CJsonValue
{
public:
    enum JsonValueType
    {
        JsonValueType_Null = 0,
        JsonValueType_Boolean = 1,
        JsonValueType_Number = 2,
        JsonValueType_String = 3,
        JsonValueType_Array = 4,
        JsonValueType_Object = 5,
    };

    CJsonValue();
    ~CJsonValue();

    JsonValueType _valueType;
    std::wstring _szStringValue;
    double _doubleValue;
    bool _booleanValue;
    std::int32_t _iLineNumber;
    std::int32_t _iColumnNumber;
    CJsonObject* _pObjectValue;
    CJsonArray* _pArrayValue;
};

class CJsonArray
{
public:
    ~CJsonArray();
    void AddValue(CJsonValue* value) { _valueArray.push_back(value); }

private:
    std::list<CJsonValue*> _valueArray;
};

class CJsonObject
{
public:
    ~CJsonObject();
    [[nodiscard]] std::map<std::wstring, CJsonValue*> GetMap() { return _objectValueMap; }

private:
    friend class CJsonParser;

    HRESULT AddPair(const wchar_t* name, CJsonValue* value)
    {
        if (name == nullptr || value == nullptr)
        {
            return E_INVALIDARG;
        }

        const auto inserted = _objectValueMap.insert(std::pair<const std::wstring, CJsonValue*>(std::wstring(name), value));
        return inserted.second ? S_OK : S_FALSE;
    }

    bool IsNameInMap(const wchar_t* name);

    std::map<std::wstring, CJsonValue*> _objectValueMap;
};

static_assert(sizeof(void*) == 4 ? sizeof(CJsonObject) == 12 : sizeof(CJsonObject) == 24);

class CJsonArray;
class CJsonObject;
class CJsonValue;

class CJsonParser
{
public:
    enum tagJSON_FLAGS
    {
        JSON_ENABLECOMMENTS = 1,
        JSON_ENABLE_RESJSON_CHECKS = 16,
    };

    enum tagJSON_STATE
    {
        UNINITIALIZED = 0,
        INITIALIZED = 1,
    };

    enum tagJSON_ERROR : std::int32_t
    {
        NO_ERR = 0,
        ERR_DUPLICATE = 1,
        ERR_MULTIPLE_OBJECTS = 2,
        ERR_MISSING_LBRACE = 3,
        ERR_MISSING_RBRACE = 4,
        ERR_MISSING_RSQUARE = 5,
        ERR_MISSING_OBJECT_NAME = 6,
        ERR_MISSING_COLON = 7,
        ERR_INVALID_CHARACTER = 8,
        ERR_INVALID_INPUTSTRING = 9,
        ERR_NOT_INITIALIZED = 10,
    };

    CJsonParser() = default;
    ~CJsonParser();

    HRESULT SetInput(const wchar_t* input, std::uint32_t length)
    {
        HRESULT result = _lexJSON.SetInput(input, length);
        if (result == E_INVALIDARG)
        {
            result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _jsonError = ERR_INVALID_INPUTSTRING;
        }
        else if (SUCCEEDED(result))
        {
            _jsonState = INITIALIZED;
        }
        return result;
    }

    HRESULT Parse(CJsonValue* value);
    tagJSON_ERROR GetLastJsonError(int* line, int* column);

private:
    bool _IsValue(CJSONLexer::tagTOKEN token);
    HRESULT _ParseValue(CJsonValue* value);
    HRESULT _ParseArray(CJsonArray* value);
    HRESULT _ParseObject(CJsonObject* value);

    CJSONLexer _lexJSON;
    CJSONLexer::tagTOKEN _tokParse;
    [[maybe_unused]] bool _bEnableInlineComments {};
    tagJSON_ERROR _jsonError {NO_ERR};
    tagJSON_STATE _jsonState {UNINITIALIZED};
    std::uint32_t _ulJsonFlags {JSON_ENABLECOMMENTS | JSON_ENABLE_RESJSON_CHECKS};
};
