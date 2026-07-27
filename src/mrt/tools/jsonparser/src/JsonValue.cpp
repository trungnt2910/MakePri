#include "StdAfx.h"

#include <Json.h>

CJsonValue::CJsonValue() : _iLineNumber(0), _iColumnNumber(0), _pObjectValue(nullptr), _pArrayValue(nullptr) {}

CJsonValue::~CJsonValue()
{
    if (_pObjectValue != nullptr)
    {
        delete _pObjectValue;
    }
    if (_pArrayValue != nullptr)
    {
        delete _pArrayValue;
    }
}

CJsonArray::~CJsonArray()
{
    while (!_valueArray.empty())
    {
        CJsonValue* const value = _valueArray.front();
        if (value != nullptr)
        {
            delete value;
        }
        _valueArray.pop_front();
    }
}

bool CJsonObject::IsNameInMap(const wchar_t* const name)
{
    const std::wstring value(name);
    return _objectValueMap.find(value) != _objectValueMap.end();
}

CJsonObject::~CJsonObject()
{
    for (auto iterator = _objectValueMap.begin(); iterator != _objectValueMap.end(); ++iterator)
    {
        if (iterator->second != nullptr)
        {
            delete iterator->second;
        }
    }
    _objectValueMap.clear();
}
