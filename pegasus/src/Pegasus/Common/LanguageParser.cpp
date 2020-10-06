//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/LanguageParser.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <cstring>

#ifdef PEGASUS_HAS_ICU
# include <unicode/locid.h>
#endif

PEGASUS_NAMESPACE_BEGIN

static char LANGUAGE_TAG_SEPARATOR_CHAR = '-';

AcceptLanguageList LanguageParser::parseAcceptLanguageHeader(
    const String& acceptLanguageHeader)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseAcceptLanguageHeader");

    AcceptLanguageList acceptLanguages;

    try
    {
        Array<String> languageElements;
        LanguageParser::_parseLanguageHeader(
            acceptLanguageHeader,
            languageElements);

        for (Uint32 i = 0; i < languageElements.size(); i++)
        {
            String languageTagString;
            Real32 qualityValue;
            LanguageParser::_parseAcceptLanguageElement(
                languageElements[i], languageTagString, qualityValue);
            acceptLanguages.insert(
                LanguageTag(languageTagString), qualityValue);
        }
    }
    catch (Exception& e)
    {
        throw InvalidAcceptLanguageHeader(e.getMessage());
    }

    PEG_METHOD_EXIT();
    return acceptLanguages;
}

ContentLanguageList LanguageParser::parseContentLanguageHeader(
    const String& contentLanguageHeader)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseContentLanguageHeader");

    ContentLanguageList contentLanguages;

    try
    {
        Array<String> languageElements;
        LanguageParser::_parseLanguageHeader(
            contentLanguageHeader,
            languageElements);

        for (Uint32 i = 0; i < languageElements.size(); i++)
        {
            contentLanguages.append(LanguageTag(languageElements[i]));
        }
    }
    catch (Exception& e)
    {
        throw InvalidContentLanguageHeader(e.getMessage());
    }

    PEG_METHOD_EXIT();
    return contentLanguages;
}

void LanguageParser::parseLanguageTag(
    const String& languageTagString,
    String& language,
    String& country,
    String& variant)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::parseLanguageTag");

    language.clear();
    country.clear();
    variant.clear();

    if (languageTagString == "*")
    {
        // Parsing and validation is complete
        PEG_METHOD_EXIT();
        return;
    }

    Boolean isStandardFormat = true;    // RFC 3066 (ISO 639, ISO 3166)
    Array<String> subtags;

    _parseLanguageSubtags(subtags, languageTagString);

    // _parseLanguageSubtags() always returns at least one subtag.
    PEGASUS_ASSERT(subtags.size() > 0);

    // Validate the primary subtag.
    // Given a languageTagString "en-US-mn" the language is "en".

    language = subtags[0];

    if ((language == "i") || (language == "x"))
    {
        // These primary tags are allowed, but are not ISO 639 compliant
        isStandardFormat = false;
        language.clear();
    }
    else if ((language.size() != 2) && (language.size() != 3))
    {
        // Except for "i" and "x", primary tags must be 2 or 3 characters,
        // according to RFC 3066.

        // Do not localize this message; it could cause recursion.
        PEG_METHOD_EXIT();
        throw Exception(Formatter::format(
            "Invalid language tag \"$0\".",
            languageTagString));
    }

    if (subtags.size() == 1)
    {
        // If only the primary subtag is present, we are done!
        PEG_METHOD_EXIT();
        return;
    }

    // Validate the second subtag.
    // Given a languageTagString "en-US-mn" the country is "US".

    if (subtags[1].size() == 1)
    {
        // The second subtag may not be a single character according to
        // RFC 3066.

        // Do not localize this message; it could cause recursion.
        PEG_METHOD_EXIT();
        throw Exception(Formatter::format(
            "Invalid language tag \"$0\".",
            languageTagString));
    }

    if (isStandardFormat)
    {
        Uint32 variantIndex = 1;

        if (subtags[1].size() == 2)
        {
            country = subtags[1];
            variantIndex = 2;
        }

        Uint32 numSubtags = subtags.size();

        if (variantIndex < numSubtags)
        {
            variant = subtags[variantIndex++];

            while (variantIndex < numSubtags)
            {
                variant.append(LANGUAGE_TAG_SEPARATOR_CHAR);
                variant.append(subtags[variantIndex++]);
            }
        }
    }

    PEG_METHOD_EXIT();
}

void LanguageParser::validateQualityValue(Real32 quality)
{
    if ((quality > 1.0) || (quality < 0.0))
    {
        MessageLoaderParms parms(
            "Common.LanguageParser.INVALID_QUALITY_VALUE",
            "AcceptLanguage contains an invalid quality value");
        throw InvalidAcceptLanguageHeader(MessageLoader::getMessage(parms));
    }
}

String LanguageParser::buildAcceptLanguageHeader(
    const AcceptLanguageList& acceptLanguages)
{
    String alString;
    Uint32 numAcceptLanguages = acceptLanguages.size();

    for (Uint32 i = 0; i < numAcceptLanguages; i++)
    {
        alString.append(acceptLanguages.getLanguageTag(i).toString());

        Real32 q = acceptLanguages.getQualityValue(i);
        if (q != 1.0)
        {
            char qValueString[6];
            sprintf(qValueString, "%4.3f", q);
            alString.append(";q=");
            alString.append(qValueString);
        }

        if (i < numAcceptLanguages - 1)
        {
            alString.append(",");
        }
    }

    return alString;
}

String LanguageParser::buildContentLanguageHeader(
    const ContentLanguageList& contentLanguages)
{
    String clString;
    Uint32 numContentLanguages = contentLanguages.size();

    for (Uint32 i = 0; i < numContentLanguages; i++)
    {
        clString.append(contentLanguages.getLanguageTag(i).toString());

        if (i < numContentLanguages - 1)
        {
            clString.append(",");
        }
    }

    return clString;
}

#ifdef PEGASUS_HAS_ICU
String& LanguageParser::convertLocaleIdToLanguageTag(String& localeId)
{
    static char LOCALE_ID_SEPARATOR_CHAR = '_';
    Uint32 index = 0;
    while ((index = localeId.find(index, LOCALE_ID_SEPARATOR_CHAR)) !=
                PEG_NOT_FOUND)
    {
        localeId[index] = LANGUAGE_TAG_SEPARATOR_CHAR;
    }
    return localeId;
}
#endif

AcceptLanguageList LanguageParser::getDefaultAcceptLanguages()
{
#if defined(PEGASUS_HAS_MESSAGES) && defined(PEGASUS_HAS_ICU)
    Locale default_loc = Locale::getDefault();

    String localeId = default_loc.getName();

    try
    {
        return LanguageParser::parseAcceptLanguageHeader(
            convertLocaleIdToLanguageTag(localeId));
    }
    catch (const InvalidAcceptLanguageHeader& e)
    {
        Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            MessageLoaderParms(
                "src.Server.cimserver.FAILED_TO_SET_PROCESS_LOCALE",
                "Cannot convert the system process locale into a valid "
                    "Accept-Language format."));
        Logger::put(Logger::ERROR_LOG, System::CIMSERVER, Logger::SEVERE,
            e.getMessage());
        AcceptLanguageList al;
        al.insert(LanguageTag("*"), 1);
        return al;
    }
#else
    return AcceptLanguageList();
#endif
}

void LanguageParser::_parseLanguageHeader(
    const String& languageHeaderValue,
    Array<String>& languageElements)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::_parseLanguageHeader");

    languageElements.clear();
    String element;

    for (Uint32 i=0, len=languageHeaderValue.size(); i<len; i++)
    {
        Char16 nextChar = languageHeaderValue[i];

        if (isascii(nextChar) && isspace(nextChar))
        {
            // Ignore whitespace
        }
        else if (nextChar == '(')
        {
            // Ignore comments
            while (i < len)
            {
                // Search for the closing parenthesis
                if (languageHeaderValue[i] == ')')
                {
                    break;
                }

                // Skip over escape characters
                if (languageHeaderValue[i] == '\\')
                {
                    i++;
                }

                i++;
            }

            // Check for a missing closing parenthesis
            if (i >= len)
            {
                MessageLoaderParms parms(
                    "Common.LanguageParser.DOES_NOT_CONTAIN_TERMINATING",
                    "Closing \")\" character is missing.");
                throw Exception(MessageLoader::getMessage(parms));
            }
        }
        else if (nextChar == ',')
        {
            // Check for the end of the element
            languageElements.append(element);
            element.clear();
        }
        else
        {
            // Unescape an escape character
            if ((nextChar == '\\') && (i < len-1))
            {
                nextChar = languageHeaderValue[++i];
            }

            // Include this character in the value
            element.append(nextChar);
        }
    }

    // Include the last element in the languageElements array
    languageElements.append(element);

    PEG_METHOD_EXIT();
}

void LanguageParser::_parseAcceptLanguageElement(
    const String& acceptLanguageElement,
    String& languageTag,
    Real32& quality)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::_parseAcceptLanguageElement");

    // look for ';' in acceptLanguageElement, that means we have a
    // quality value to capture.  If not present, we only have a language tag.

    Uint32 semicolonIndex = acceptLanguageElement.find(";");
    if (semicolonIndex != PEG_NOT_FOUND)
    {
        // Separate the language tag and quality value

        String qualityString =
            acceptLanguageElement.subString(semicolonIndex+1);
        languageTag = acceptLanguageElement.subString(0, semicolonIndex);

        // Parse the quality value

        char dummyChar;
        int scanfConversions = sscanf(
            qualityString.getCString(),
            "q=%f%c", &quality, &dummyChar);

        if ((scanfConversions != 1) ||
            (qualityString.size() > 7))
        {
            MessageLoaderParms parms(
                "Common.LanguageParser.INVALID_QUALITY_VALUE",
                "AcceptLanguage contains an invalid quality value");
            PEG_METHOD_EXIT();
            throw Exception(MessageLoader::getMessage(parms));
        }
    }
    else
    {
        languageTag = acceptLanguageElement;
        quality = 1.0;
    }

    PEG_METHOD_EXIT();
}

void LanguageParser::_parseLanguageSubtags(
    Array<String>& subtags,
    const String& languageTagString)
{
    PEG_METHOD_ENTER(TRC_L10N, "LanguageParser::_parseLanguageSubtags");

    // Parse the language tag into subtags

    Uint32 subtagIndex = 0;
    Uint32 separatorIndex;
    while ((separatorIndex = languageTagString.find(
                subtagIndex, LANGUAGE_TAG_SEPARATOR_CHAR)) != PEG_NOT_FOUND)
    {
        subtags.append(languageTagString.subString(
            subtagIndex, separatorIndex - subtagIndex));
        subtagIndex = separatorIndex + 1;
    }
    subtags.append(languageTagString.subString(subtagIndex));

    // Validate the syntax of each of the subtags

    for (Uint32 i = 0, n = subtags.size(); i < n; i++)
    {
        if (((i == 0) && !_isValidPrimarySubtagSyntax(subtags[i])) ||
            ((i > 0) && !_isValidSubtagSyntax(subtags[i])))
        {
            // throw Exception(MessageLoader::getMessage(parms));
            // do not localize message, requires a language tag for this
            // localization can cause recursion here
            // MessageLoaderParms::toString adds 5kb static code size, Do NOT
            // include in non-debug builds
#ifdef PEGASUS_DEBUG
            MessageLoaderParms parms(
                "Common.LanguageParser.MALFORMED_LANGUAGE_TAG",
                "Malformed language tag \"$0\".", languageTagString);
            PEG_METHOD_EXIT();
            throw Exception(parms.toString());
#else
            String malFormed("Malformed language tag:");
            malFormed.append(languageTagString);
            PEG_METHOD_EXIT();
            throw Exception(malFormed);
#endif
        }
    }

    PEG_METHOD_EXIT();
}

Boolean LanguageParser::_isValidPrimarySubtagSyntax(const String& subtag)
{
    if ((subtag.size() == 0) || (subtag.size() > 8))
    {
        return false;
    }

    for (Uint32 i = 0, n = subtag.size(); i < n; i++)
    {
        if (!(isascii(subtag[i]) && isalpha(subtag[i])))
        {
            return false;
        }
    }

    return true;
}

Boolean LanguageParser::_isValidSubtagSyntax(const String& subtag)
{
    if ((subtag.size() == 0) || (subtag.size() > 8))
    {
        return false;
    }

    for (Uint32 i = 0, n = subtag.size(); i < n; i++)
    {
        if (!(isascii(subtag[i]) && isalnum(subtag[i])))
        {
            return false;
        }
    }

    return true;
}

PEGASUS_NAMESPACE_END
