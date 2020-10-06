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

#ifndef Pegasus_LanguageParser_h
#define Pegasus_LanguageParser_h

#include <cstdlib>
#include <cctype>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class parses and validates Accept-Language and Content-Language values
    from the respective HTTP headers, and provides utility functions to parse
    and manipulate language information
 */
class PEGASUS_COMMON_LINKAGE LanguageParser
{
public:

    /**
        Builds an AcceptLanguageList object from an HTTP Accept-Language
        header value.
        @param acceptLanguageHeader A String containing an HTTP
        Accept-Language header value from which to build the AcceptLanguageList
        object.
        @exception InvalidAcceptLanguageHeader if a syntax error is
        encountered.
     */
    static AcceptLanguageList parseAcceptLanguageHeader(
        const String& acceptLanguageHeader);

    /**
        Builds a ContentLanguageList object from an HTTP Content-Language
        header value.
        @param contentLanguageHeader A String containing an HTTP
        Content-Language header value from which to build the
        ContentLanguageList object.
        @exception InvalidContentLanguageHeader if a syntax error is
        encountered.
     */
    static ContentLanguageList parseContentLanguageHeader(
        const String& contentLanguageHeader);

    /**
        Validates the syntax of a language tag according to RFC 3066.
        Parses the language tag into language, country, and variant, if
        the language tag is formed using ISO codes (ISO 639 and ISO 3166).
        @param languageTagString The language tag string to parse and
        validate.
        @param language The returned language value parsed from the language
        tag, if applicable.
        @param country The returned country value parsed from the language
        tag, if applicable.
        @param variant The returned language variant value parsed from the
        language tag, if applicable.
        @exception Exception if the language tag is syntactically invalid.
     */
    static void parseLanguageTag(
        const String& languageTagString,
        String& language,
        String& country,
        String& variant);

    /**
        Validates the syntax of an Accept-Language quality value.
        @param quality A Real32 quality value to validate.
        @exception Exception if the quality value is invalid.
     */
    static void validateQualityValue(Real32 quality);

    /**
        Builds an HTTP Accept-Language header value from an AcceptLanguageList
        object.
        @param acceptLanguages The AcceptLanguageList object to be encoded in
        an HTTP Accept-Language header.
     */
    static String buildAcceptLanguageHeader(
        const AcceptLanguageList& acceptLanguages);

    /**
        Builds an HTTP Content-Language header value from a ContentLanguageList
        object.
        @param contentLanguages The ContentLanguageList object to be encoded
        in an HTTP Content-Language header.
     */
    static String buildContentLanguageHeader(
        const ContentLanguageList& contentLanguages);

#ifdef PEGASUS_HAS_ICU
    /**
        Converts a locale ID to language tag syntax by replacing the '_'
        separators with '-' separators.
        @param localeId The locale ID string to convert to language tag
        syntax.  The conversion is performed on the localeId object
        directly.
        @return The converted String with language tag syntax.
     */
    static String& convertLocaleIdToLanguageTag(String& localeId);
#endif

    /**
        Returns the default accept languages based on environment settings.
        ATTN: What is the behavior?  This is only implemented if MESSAGES and
        ICU are used.
     */
    static AcceptLanguageList getDefaultAcceptLanguages();

private:

    /**
        Parses the value portion of an HTTP Accept-Language or Content-Language
        header into individual language elements.  Comments and whitespace are
        removed.
        @param headerElements An array of Strings into which the language
        elements are stored.
        @param languageHeaderValue A String containing the value portion of
        an HTTP Accept-Language or Content-Language header.
        @exception Exception if a syntax error is encountered.
     */
    static void _parseLanguageHeader(
        const String& languageHeaderValue,
        Array<String>& languageElements);

    /**
        Parses an AcceptLanguage or ContentLanguage value from a header.
        If a valid AcceptLanguage value is determined to contain a quality
        value, then this quality is validated and returned. Otherwise -1 is
        returned.  It is up to the caller to know in what context this call
        is being made, that is: it is the callers responsibility to know
        whether an AcceptLanguage or a ContentLanguage value is being parsed.
        @param acceptLanguageHeaderElement String to be parsed into a
        language tag and quality value.  The String must not contain comments
        or whitespace.
        @param languageTag String into which the parsed language tag is
        stored.
        @param quality Real32 into which the parsed quality value is stored.
        @exception Exception if a syntax error is encountered.
     */
    static void _parseAcceptLanguageElement(
        const String& acceptLanguageElement,
        String& languageTag,
        Real32& quality);

    /**
        Fills the first position in the array with the most general part of
        the language String, each subsequent postion is the array gets more
        and more detailed.  For example: en-us-fooblah would be returned as
        ["en"]["us"]["fooblah"]
        @param subtags Array<String>& array filled in with the root and subtags
        @param languageTag String the language to parse
        @return String the root of the language tag
        @exception Exception if a syntax error is encountered.
     */
    static void _parseLanguageSubtags(
        Array<String>& subtags,
        const String& languageTagString);

    /**
        Checks if a String represents a valid language tag primary subtag.
        A valid primary subtag contains 1 to 8 alphabetic characters.
        @param subtag The subtag String to validate.
        @return True if the String represents a valid language tag primary
        subtag; false otherwise.
     */
    static Boolean _isValidPrimarySubtagSyntax(const String& subtag);

    /**
        Checks if a String represents a valid language tag subtag.
        A valid primary subtag contains 1 to 8 alphanumeric characters.
        @param subtag The subtag String to validate.
        @return True if the String represents a valid language tag subtag;
        false otherwise.
     */
    static Boolean _isValidSubtagSyntax(const String& subtag);
};

PEGASUS_NAMESPACE_END

#endif
