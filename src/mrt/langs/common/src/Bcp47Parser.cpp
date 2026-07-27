#include "StdAfx.h"

#include <Language.h>

namespace Windows::Internal
{
namespace
{
constexpr std::uint8_t TagParsed = 0x01;
constexpr std::uint8_t TagWellFormed = 0x04;
constexpr std::uint8_t TagValid = 0x08;
constexpr std::uint8_t TagGrandfathered = 0x10;
constexpr std::uint8_t TagPseudo = 0x20;
constexpr std::uint8_t TagPrivateUse = 0x40;

constexpr std::uint32_t MaximumTagLength = 84;
} // namespace

bool CLanguage::ParseTag(const wchar_t* const tag)
{
    if ((m_subtags.flags & TagParsed) == 0)
    {
        std::size_t tagLength;
        const HRESULT lengthResult = StringCchLengthW(tag, MaximumTagLength + 1, &tagLength);
        if (FAILED(lengthResult))
        {
            tagLength = MaximumTagLength + 2;
        }

        Bcp47SubtagField currentSubtag {};

        struct ParserStateMachine
        {
            ParserStateMachine(Bcp47TagSubtagsInfo* const target, Bcp47SubtagField* const currentSubtag, const std::uint32_t tagLength) :
                target(target), currentSubtag(currentSubtag), state(0), valid(true), done(false)
            {
                target->flags &= ~TagValid;
                target->flags &= ~TagPseudo;
                target->flags &= ~TagPrivateUse;
                target->flags |= TagParsed;
                target->flags &= ~TagWellFormed;
                target->subtagsMap = BSF_FULL_TAG;
                target->subtags[7].offset = 0;

                if ((tagLength < 2) || (tagLength >= 85))
                {
                    valid = false;
                    done = true;
                    target->subtags[7].length = 0;
                }
                else
                {
                    target->subtags[7].length = static_cast<std::uint8_t>(tagLength);
                }
            }

            bool operator()(const bool isValid, const std::uint32_t subtagIndex, const std::uint32_t nextState)
            {
                if (valid)
                {
                    const auto subtagFlag = 1U << subtagIndex;
                    if ((target->subtagsMap & subtagFlag) != 0)
                    {
                        target->subtags[subtagIndex].length += currentSubtag->length + 1;
                    }
                    else
                    {
                        target->subtagsMap = static_cast<BCP47_SUBTAG_FLAGS>(target->subtagsMap | subtagFlag);
                        target->subtags[subtagIndex] = *currentSubtag;
                    }

                    state = nextState;
                    valid = isValid;
                }

                return isValid;
            }

            ~ParserStateMachine()
            {
                if (valid && ((state != 5) || (target->subtags[5].length > 3)) && ((state != 6) || (target->subtags[6].length > 2)))
                {
                    target->flags |= TagWellFormed;
                }
                else
                {
                    target->flags &= ~TagWellFormed;
                }
            }

            Bcp47TagSubtagsInfo* target;
            Bcp47SubtagField* currentSubtag;
            std::uint32_t state;
            bool valid;
            bool done;
        } parser(&m_subtags, &currentSubtag, static_cast<std::uint32_t>(tagLength));

        if (SUCCEEDED(lengthResult))
        {
            static constexpr std::uint32_t StateTransitions[] = {
                0, 9, 1, 1, 4, 0,  0,  0, 0, 9, 6, 2, 4, 8,  8,  8,  8,  9,  6,  7,  4,  8,  8,  8,  8,  9,  6,  7,  5,
                8, 8, 8, 8, 9, 12, 12, 5, 8, 8, 8, 8, 9, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
            };

            std::uint8_t alphaCharacters = 0;
            std::uint8_t digitCharacters = 0;
            bool hasExtlang = false;
            bool hasExtension = false;
            std::uint8_t characterIndex = 0;

            do
            {
                if (!parser.valid || parser.done)
                {
                    break;
                }

                const wchar_t character = tag[characterIndex];
                if (static_cast<std::uint16_t>((character | 0x20) - L'a') <= 25)
                {
                    if (currentSubtag.length == 0)
                    {
                        currentSubtag.offset = characterIndex;
                    }

                    ++currentSubtag.length;
                    ++alphaCharacters;
                }
                else if (static_cast<std::uint16_t>(character - L'0') <= 9)
                {
                    if (currentSubtag.length == 0)
                    {
                        currentSubtag.offset = characterIndex;
                    }

                    ++currentSubtag.length;
                    ++digitCharacters;
                }
                else if (((character == L'-') || (character == L'\0')) && (currentSubtag.length != 0) && (currentSubtag.length <= 8))
                {
                    const auto action = StateTransitions[(parser.state * 8) + currentSubtag.length];
                    switch (action)
                    {
                    case 0:
                        parser.valid = false;
                        parser.done = true;
                        break;
                    case 1:
                        parser(digitCharacters == 0, 0, 1);
                        break;
                    case 2:
                        if (digitCharacters != 0)
                        {
                            if (alphaCharacters != 0)
                            {
                                parser.valid = false;
                                parser.done = true;
                            }
                            else
                            {
                                parser(true, 3, 4);
                            }
                        }
                        else if (hasExtlang)
                        {
                            parser.valid = false;
                            parser.done = true;
                        }
                        else
                        {
                            parser(true, 1, 1);
                            hasExtlang = true;
                        }
                        break;
                    case 3:
                        m_subtags.flags |= TagGrandfathered;
                        parser(digitCharacters == 0, 0, 2);
                        break;
                    case 4:
                        if (digitCharacters != 0)
                        {
                            parser(static_cast<std::uint16_t>(tag[currentSubtag.offset] - L'0') <= 9, 4, 4);
                        }
                        else
                        {
                            parser(true, 3, 2);
                        }
                        break;
                    case 5:
                        parser(static_cast<std::uint16_t>(tag[currentSubtag.offset] - L'0') <= 9, 4, 4);
                        break;
                    case 6:
                        if (digitCharacters != 0)
                        {
                            parser.valid = false;
                            parser.done = true;
                        }
                        else
                        {
                            parser(true, 3, 4);
                        }
                        break;
                    case 7:
                        if (alphaCharacters != 0)
                        {
                            parser.valid = false;
                            parser.done = true;
                        }
                        else
                        {
                            parser(true, 3, 4);
                        }
                        break;
                    case 8:
                        parser(true, 4, 4);
                        break;
                    case 9:
                        if (hasExtension)
                        {
                            parser.valid = false;
                            parser.done = true;
                        }
                        else if ((tag[currentSubtag.offset] == L'x') || (tag[currentSubtag.offset] == L'X'))
                        {
                            parser(true, 6, 6);
                        }
                        else if (currentSubtag.offset != 0)
                        {
                            hasExtension = true;
                            parser(true, 5, 5);
                        }
                        else
                        {
                            parser.done = true;
                            if ((m_subtags.flags & TagGrandfathered) == 0)
                            {
                                parser.valid = false;
                            }
                        }
                        break;
                    case 10:
                        hasExtension = false;
                        parser(true, 5, 5);
                        break;
                    case 11:
                        m_subtags.flags |= TagPrivateUse;
                        parser(true, 6, 6);
                        break;
                    case 12:
                        parser.done = true;
                        if ((m_subtags.flags & TagGrandfathered) == 0)
                        {
                            parser.valid = false;
                        }
                        break;
                    default:
                        break;
                    }

                    digitCharacters = 0;
                    alphaCharacters = 0;
                    currentSubtag.length = 0;
                    currentSubtag.offset = 0;
                }
                else
                {
                    parser.valid = false;
                    parser.done = true;
                }

                ++characterIndex;
            } while (characterIndex <= tagLength);
        }
    }

    return (m_subtags.flags & TagWellFormed) != 0;
}
} // namespace Windows::Internal
