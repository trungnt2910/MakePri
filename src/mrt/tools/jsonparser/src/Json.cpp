#include "StdAfx.h"

#include <Json.h>

CJSONLexer::JsonCharacter& CJSONLexer::JsonCharacter::operator++()
{
    const wchar_t character = *pszInput;
    if (character == L'\n' || character == L'\r')
    {
        ++iLineNum;
        iColumnNum = 1;
    }
    else
    {
        ++iColumnNum;
    }
    ++pszInput;
    return *this;
}

const wchar_t* CJSONLexer::JsonCharacter::operator++(int)
{
    const wchar_t* const previous = pszInput;
    operator++();
    return previous;
}

CJSONLexer::tagTOKEN CJSONLexer::GetToken()
{
    if (_jsonInput.pszInput == nullptr || _szYYTextAlloc == nullptr)
    {
        _token = tagTOKEN::tokenERROR;
        return _token;
    }
    if (_token == tagTOKEN::tokenERROR || _token == tagTOKEN::tokenFAIL)
    {
        return _token;
    }

    std::size_t tokenLength = 0;
    if (FAILED(StringCchLengthW(_szYYTextAlloc, _cchYYTextAlloc, &tokenLength)))
    {
        _token = tagTOKEN::tokenERROR;
        return _token;
    }
    memset(_szYYTextAlloc, 0, sizeof(wchar_t) * tokenLength);
    _pszYYText = _szYYTextAlloc;

    while (true)
    {
        const wchar_t character = *_jsonInput.pszInput;
        if (character == L'\0')
        {
            _token = tagTOKEN::tokenEOF;
            return _token;
        }
        const wchar_t next = _jsonInput.pszInput[1];
        if (character == L' ' || character == L'\t' || character == L'\n' || character == L'\r')
        {
            ++_jsonInput;
            continue;
        }
        if (character == L'/' && next == L'/' && _bEnableInlineComments)
        {
            while (*_jsonInput.pszInput != L'\0' && *_jsonInput.pszInput != L'\n' && *_jsonInput.pszInput != L'\r')
            {
                ++_jsonInput;
            }
            continue;
        }

        _iTokenLineNumber = _jsonInput.iLineNum;
        _iTokenColumnNumber = _jsonInput.iColumnNum;
        *_pszYYText = character;

        if (isdigit(character))
        {
            if (character != L'0')
            {
                return ReadNumber();
            }
            if (isdigit(next))
            {
                _jsonInput++;
                _token = tagTOKEN::tokenUNKNOWN;
                return _token;
            }
            _jsonInput++;
            _token = tagTOKEN::tokenINTEGER;
            return _token;
        }

        if (isalpha(character))
        {
            do
            {
                *_pszYYText++ = *_jsonInput++;
            } while (*_jsonInput.pszInput != L'\0' && isalpha(*_jsonInput.pszInput));

            if (wcscmp(_szYYTextAlloc, L"false") == 0)
            {
                _token = tagTOKEN::tokenFALSE;
            }
            else if (wcscmp(_szYYTextAlloc, L"true") == 0)
            {
                _token = tagTOKEN::tokenTRUE;
            }
            else if (wcscmp(_szYYTextAlloc, L"null") == 0)
            {
                _token = tagTOKEN::tokenNULL;
            }
            else
            {
                _token = tagTOKEN::tokenUNKNOWN;
            }
            return _token;
        }

        switch (character)
        {
        case L'[':
            ++_jsonInput;
            _token = tagTOKEN::tokenLSQUARE;
            return _token;
        case L']':
            ++_jsonInput;
            _token = tagTOKEN::tokenRSQUARE;
            return _token;
        case L'{':
            ++_jsonInput;
            _token = tagTOKEN::tokenLBRACE;
            return _token;
        case L'}':
            ++_jsonInput;
            _token = tagTOKEN::tokenRBRACE;
            return _token;
        case L':':
            ++_jsonInput;
            _token = tagTOKEN::tokenCOLON;
            return _token;
        case L',':
            ++_jsonInput;
            _token = tagTOKEN::tokenCOMMA;
            return _token;
        case L'"':
            *_pszYYText = L'\0';
            return ReadString();
        case L'-':
            ++_pszYYText;
            ++_jsonInput;
            if (next == L'0' || !isdigit(next))
            {
                _token = tagTOKEN::tokenUNKNOWN;
                return _token;
            }
            return ReadNumber();
        default:
            _jsonInput++;
            _token = tagTOKEN::tokenUNKNOWN;
            return _token;
        }
    }
}

void CJSONLexer::GetPositionOfToken(int* const line, int* const column)
{
    if (line != nullptr)
    {
        *line = static_cast<int>(_iTokenLineNumber);
    }
    if (column != nullptr)
    {
        *column = static_cast<int>(_iTokenColumnNumber);
    }
}

HRESULT CJSONLexer::SetInput(const wchar_t* const input, const std::uint32_t length)
{
    HRESULT result = S_OK;
    _token = tagTOKEN::tokenUNKNOWN;
    if (input == nullptr)
    {
        delete[] _szYYTextAlloc;
        _szYYTextAlloc = nullptr;
        _cchYYTextAlloc = 0;
        _jsonInput.pszInput = nullptr;
        _pszYYText = nullptr;
        return result;
    }

    std::size_t inputLength = 0;
    result = StringCchLengthW(input, length, &inputLength);
    if (SUCCEEDED(result))
    {
        if (inputLength == 0)
        {
            return E_INVALIDARG;
        }
        if (_szYYTextAlloc != nullptr)
        {
            delete[] _szYYTextAlloc;
        }
        _cchYYTextAlloc = inputLength + 1;
        _szYYTextAlloc = new wchar_t[_cchYYTextAlloc];
        if (_szYYTextAlloc == nullptr)
        {
            _cchYYTextAlloc = 0;
            return E_OUTOFMEMORY;
        }
        memset(_szYYTextAlloc, 0, sizeof(wchar_t) * _cchYYTextAlloc);
        _jsonInput.pszInput = input;
        _pszYYText = _szYYTextAlloc;
    }
    return result;
}

CJSONLexer::tagTOKEN CJSONLexer::ReadExponent()
{
    if (_jsonInput.pszInput == nullptr || *_jsonInput.pszInput == L'\0' || (*_jsonInput.pszInput != L'E' && *_jsonInput.pszInput != L'e'))
    {
        _token = tagTOKEN::tokenFAIL;
        return _token;
    }

    const wchar_t next = _jsonInput.pszInput[1];
    if (next == L'+' || next == L'-')
    {
        if (!isdigit(_jsonInput.pszInput[2]))
        {
            _token = tagTOKEN::tokenUNKNOWN;
            return _token;
        }
        *_pszYYText++ = *_jsonInput++;
        do
        {
            *_pszYYText++ = *_jsonInput++;
        } while (*_jsonInput.pszInput != L'\0' && isdigit(*_jsonInput.pszInput));
        _token = tagTOKEN::tokenFLOAT;
        return _token;
    }

    if (isdigit(_jsonInput.pszInput[1]))
    {
        do
        {
            *_pszYYText++ = *_jsonInput++;
        } while (*_jsonInput.pszInput != L'\0' && isdigit(*_jsonInput.pszInput));
        _token = tagTOKEN::tokenFLOAT;
        return _token;
    }

    _token = tagTOKEN::tokenUNKNOWN;
    return _token;
}

CJSONLexer::tagTOKEN CJSONLexer::ReadNumber()
{
    if (_jsonInput.pszInput == nullptr || *_jsonInput.pszInput == L'\0' || *_jsonInput.pszInput == L'0')
    {
        _token = tagTOKEN::tokenFAIL;
        return _token;
    }

    do
    {
        *_pszYYText++ = *_jsonInput++;
    } while (*_jsonInput.pszInput != L'\0' && isdigit(*_jsonInput.pszInput));

    if (*_jsonInput.pszInput == L'.')
    {
        if (!isdigit(_jsonInput.pszInput[1]))
        {
            _token = tagTOKEN::tokenINTEGER;
            return _token;
        }
        do
        {
            *_pszYYText++ = *_jsonInput++;
        } while (*_jsonInput.pszInput != L'\0' && isdigit(*_jsonInput.pszInput));
        if (*_jsonInput.pszInput == L'E' || *_jsonInput.pszInput == L'e')
        {
            ReadExponent();
        }
    }
    else if ((*_jsonInput.pszInput != L'E' && *_jsonInput.pszInput != L'e') || ReadExponent() == tagTOKEN::tokenUNKNOWN)
    {
        _token = tagTOKEN::tokenINTEGER;
        return _token;
    }

    _token = tagTOKEN::tokenFLOAT;
    return _token;
}

CJSONLexer::tagTOKEN CJSONLexer::ReadString()
{
    if (_jsonInput.pszInput == nullptr || *_jsonInput.pszInput == L'\0' || *_jsonInput.pszInput != L'"')
    {
        _token = tagTOKEN::tokenFAIL;
        return _token;
    }

    _jsonInput++;
    while (*_jsonInput.pszInput != L'\0' && *_jsonInput.pszInput != L'"')
    {
        const wchar_t character = *_jsonInput.pszInput;
        if (character == L'\\')
        {
            const wchar_t escaped = _jsonInput.pszInput[1];
            switch (escaped)
            {
            case L'"':
                *_pszYYText = L'"';
                break;
            case L'/':
                *_pszYYText = L'/';
                break;
            case L'\\':
                *_pszYYText = L'\\';
                break;
            case L'b':
                *_pszYYText = L'\b';
                break;
            case L'f':
                *_pszYYText = L'\f';
                break;
            case L'n':
                *_pszYYText = L'\n';
                break;
            case L'r':
                *_pszYYText = L'\r';
                break;
            case L't':
                *_pszYYText = L'\t';
                break;
            case L'u':
            {
                for (std::uint32_t index = 2; index < 6; ++index)
                {
                    if (_jsonInput.pszInput[index] == L'\0' || !isxdigit(_jsonInput.pszInput[index]))
                    {
                        _token = tagTOKEN::tokenERROR;
                        return _token;
                    }
                }
                char hex[] = {
                    static_cast<char>(_jsonInput.pszInput[2]),
                    static_cast<char>(_jsonInput.pszInput[3]),
                    static_cast<char>(_jsonInput.pszInput[4]),
                    static_cast<char>(_jsonInput.pszInput[5]),
                    '\0',
                };
                if (hex[0] == '0' && hex[1] == '0' && hex[2] == '0' && hex[3] == '0')
                {
                    _token = tagTOKEN::tokenERROR;
                    return _token;
                }
                char* end;
                *_pszYYText++ = static_cast<wchar_t>(strtol(hex, &end, 16));
                _jsonInput.pszInput += 6;
                continue;
            }
            case L'x':
            {
                for (std::uint32_t index = 2; index < 4; ++index)
                {
                    if (_jsonInput.pszInput[index] == L'\0' || !isxdigit(_jsonInput.pszInput[index]))
                    {
                        _token = tagTOKEN::tokenERROR;
                        return _token;
                    }
                }
                char hex[] = {
                    static_cast<char>(_jsonInput.pszInput[2]),
                    static_cast<char>(_jsonInput.pszInput[3]),
                    '\0',
                };
                if (hex[0] == '0' && hex[1] == '0')
                {
                    _token = tagTOKEN::tokenERROR;
                    return _token;
                }
                char* end;
                *_pszYYText++ = static_cast<wchar_t>(strtol(hex, &end, 16));
                _jsonInput.pszInput += 4;
                continue;
            }
            default:
                _token = tagTOKEN::tokenERROR;
                return _token;
            }
            ++_pszYYText;
            _jsonInput.pszInput += 2;
        }
        else
        {
            const std::uint32_t value = static_cast<std::uint32_t>(character);
            if (value != 0x20 && value != 0x21 && (value < 0x23 || value > 0x5B) && (value < 0x5D || value > 0x10FFFF))
            {
                _token = tagTOKEN::tokenERROR;
                return _token;
            }
            *_pszYYText++ = *_jsonInput++;
        }
    }

    if (*_jsonInput.pszInput == L'\0')
    {
        _token = tagTOKEN::tokenERROR;
        return _token;
    }
    _jsonInput++;
    _token = tagTOKEN::tokenSTRING;
    return _token;
}

CJsonParser::~CJsonParser() = default;

CJsonParser::tagJSON_ERROR CJsonParser::GetLastJsonError(int* const line, int* const column)
{
    _lexJSON.GetPositionOfToken(line, column);
    return _jsonError;
}

bool CJsonParser::_IsValue(const CJSONLexer::tagTOKEN token)
{
    return token == CJSONLexer::tagTOKEN::tokenLBRACE || token == CJSONLexer::tagTOKEN::tokenLSQUARE ||
           token == CJSONLexer::tagTOKEN::tokenINTEGER || token == CJSONLexer::tagTOKEN::tokenFLOAT ||
           token == CJSONLexer::tagTOKEN::tokenSTRING || token == CJSONLexer::tagTOKEN::tokenFALSE ||
           token == CJSONLexer::tagTOKEN::tokenNULL || token == CJSONLexer::tagTOKEN::tokenTRUE;
}

HRESULT CJsonParser::_ParseArray(CJsonArray* const value)
{
    HRESULT result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    _tokParse = _lexJSON.GetToken();
    if (_tokParse == CJSONLexer::tagTOKEN::tokenRSQUARE)
    {
        return S_OK;
    }
    if (_IsValue(_tokParse))
    {
        while (true)
        {
            CJsonValue* const item = new (std::nothrow) CJsonValue();
            result = _ParseValue(item);
            if (FAILED(result))
            {
                if (item != nullptr)
                {
                    delete item;
                }
            }
            else if (item != nullptr)
            {
                value->AddValue(item);
            }

            _tokParse = _lexJSON.GetToken();
            if (_tokParse != CJSONLexer::tagTOKEN::tokenCOMMA || FAILED(result))
            {
                break;
            }
            _tokParse = _lexJSON.GetToken();
        }
        if (_tokParse != CJSONLexer::tagTOKEN::tokenRSQUARE && SUCCEEDED(result))
        {
            _jsonError = ERR_MISSING_RSQUARE;
            result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    else
    {
        _jsonError = ERR_INVALID_CHARACTER;
    }
    return result;
}

HRESULT CJsonParser::_ParseObject(CJsonObject* const value)
{
    _tokParse = _lexJSON.GetToken();
    if (_tokParse == CJSONLexer::tagTOKEN::tokenRBRACE)
    {
        return S_OK;
    }
    if (!_IsValue(_tokParse))
    {
        _jsonError = ERR_INVALID_CHARACTER;
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    HRESULT result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    while (true)
    {
        if (_tokParse == CJSONLexer::tagTOKEN::tokenCOMMA)
        {
            _tokParse = _lexJSON.GetToken();
        }
        if (_tokParse != CJSONLexer::tagTOKEN::tokenSTRING)
        {
            _jsonError = ERR_MISSING_OBJECT_NAME;
            break;
        }

        std::wstring name(_lexJSON.SzYYText());
        if (value->IsNameInMap(name.c_str()))
        {
            _jsonError = ERR_DUPLICATE;
            result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            break;
        }

        _tokParse = _lexJSON.GetToken();
        if (_tokParse != CJSONLexer::tagTOKEN::tokenCOLON)
        {
            _jsonError = ERR_MISSING_COLON;
            result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            break;
        }

        _tokParse = _lexJSON.GetToken();
        CJsonValue* const item = new (std::nothrow) CJsonValue();
        result = _ParseValue(item);
        if (SUCCEEDED(result))
        {
            result = value->AddPair(name.c_str(), item);
        }
        if (result != S_OK && item != nullptr)
        {
            delete item;
        }

        _tokParse = _lexJSON.GetToken();
        if (_tokParse != CJSONLexer::tagTOKEN::tokenCOMMA || FAILED(result))
        {
            break;
        }
    }

    if (_tokParse != CJSONLexer::tagTOKEN::tokenRBRACE && SUCCEEDED(result))
    {
        _jsonError = ERR_MISSING_RBRACE;
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    return result;
}

HRESULT CJsonParser::_ParseValue(CJsonValue* const value)
{
    HRESULT result = S_OK;
    _lexJSON.GetPositionOfToken(&value->_iLineNumber, &value->_iColumnNumber);
    switch (_tokParse)
    {
    case CJSONLexer::tagTOKEN::tokenLSQUARE:
    {
        CJsonArray* const array = new (std::nothrow) CJsonArray();
        if (array == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        result = _ParseArray(array);
        if (SUCCEEDED(result))
        {
            value->_valueType = CJsonValue::JsonValueType_Array;
            value->_pArrayValue = array;
        }
        else
        {
            delete array;
        }
        break;
    }
    case CJSONLexer::tagTOKEN::tokenLBRACE:
    {
        CJsonObject* const object = new (std::nothrow) CJsonObject();
        if (object == nullptr)
        {
            return E_OUTOFMEMORY;
        }
        result = _ParseObject(object);
        if (SUCCEEDED(result))
        {
            value->_valueType = CJsonValue::JsonValueType_Object;
            value->_pObjectValue = object;
        }
        else
        {
            delete object;
        }
        break;
    }
    case CJSONLexer::tagTOKEN::tokenINTEGER:
    case CJSONLexer::tagTOKEN::tokenFLOAT:
        value->_valueType = CJsonValue::JsonValueType_Number;
        value->_doubleValue = _wtof(_lexJSON.SzYYText());
        break;
    case CJSONLexer::tagTOKEN::tokenTRUE:
    case CJSONLexer::tagTOKEN::tokenFALSE:
        value->_valueType = CJsonValue::JsonValueType_Boolean;
        value->_booleanValue = CompareStringOrdinal(_lexJSON.SzYYText(), -1, L"true", -1, TRUE) == CSTR_EQUAL;
        break;
    case CJSONLexer::tagTOKEN::tokenNULL:
        value->_valueType = CJsonValue::JsonValueType_Null;
        break;
    case CJSONLexer::tagTOKEN::tokenSTRING:
        value->_valueType = CJsonValue::JsonValueType_String;
        value->_szStringValue.assign(_lexJSON.SzYYText());
        break;
    default:
        result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _jsonError = ERR_INVALID_CHARACTER;
        break;
    }
    return result;
}

HRESULT CJsonParser::Parse(CJsonValue* const value)
{
    if (_jsonState != INITIALIZED)
    {
        _jsonError = ERR_NOT_INITIALIZED;
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);
    }

    _tokParse = _lexJSON.GetToken();
    if ((_ulJsonFlags & JSON_ENABLE_RESJSON_CHECKS) != 0 && _tokParse != CJSONLexer::tagTOKEN::tokenLBRACE)
    {
        _jsonError = ERR_MISSING_LBRACE;
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    HRESULT result = _ParseValue(value);
    if (SUCCEEDED(result))
    {
        _tokParse = _lexJSON.GetToken();
        if (_tokParse != CJSONLexer::tagTOKEN::tokenEOF)
        {
            _jsonError = _tokParse == CJSONLexer::tagTOKEN::tokenLBRACE ? ERR_MULTIPLE_OBJECTS : ERR_INVALID_CHARACTER;
            result = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    return result;
}
