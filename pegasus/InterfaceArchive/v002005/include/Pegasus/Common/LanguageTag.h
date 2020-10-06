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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_LanguageTag_h
#define Pegasus_LanguageTag_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

class LanguageTagRep;

/** <I><B>Experimental Interface</B></I><BR>
    This class specifies a language in a standard form (based on RFC 3066),
    including the special language range "*".  Note:  This class may be
    extended to support RFC 3066bis in the future.
 */
class PEGASUS_COMMON_LINKAGE LanguageTag
{
public:

    /**
        Constructs an uninitialized LanguageTag object.  A method
        invocation on an uninitialized LanguageTag object will
        result in the throwing of an UninitializedObjectException.  An
        uninitialized object may be converted into an initialized object only
        by using the assignment operator with an initialized object.
     */
    LanguageTag();

    /**
        Constructs a LanguageTag object from a language tag String.
        The syntax of the language tag String is validated, but the subtags
        are not verified to be values registered with ISO or IANA.
        @param languageTag A String containing a language tag (for example,
        "en-US").
        @exception Exception if the syntax of the language tag String is not
        valid.
     */
    LanguageTag(const String& languageTagString);

    /**
        Copy constructor.
        @param languageTag The LanguageTag object to copy.
     */
    LanguageTag(const LanguageTag& languageTag);

    /**
        Destructor.
     */
    ~LanguageTag();

    /**
        Assignment operator.
        @param languageTag The LanguageTag object to copy.
     */
    LanguageTag& operator=(const LanguageTag& languageTag);

    /**
        Gets the language value, if present.
        @return A String containing the language value for this LanguageTag
        object if the primary subtag is two or three characters in length,
        an empty String otherwise.
        @exception UninitializedObjectException if the LanguageTag
        has not been initialized.
     */
    String getLanguage() const;

    /**
        Gets the country code from the second subtag, if present.
        @return A String containing the country code for this LanguageTag
        object if the primary subtag is two or three characters in length
        and the second subtag is two characters in length, an empty String
        otherwise.
        @exception UninitializedObjectException if the LanguageTag
        has not been initialized.
     */
    String getCountry() const;

    /**
        Gets the language variant, if applicable.  The language variant
        includes all the subtags after the country code (if present) or
        language value (if present).
        @return A String containing the language variant for this LanguageTag
        object if the primary subtag is two or three characters in length,
        an empty String otherwise.
        @exception UninitializedObjectException if the LanguageTag
        has not been initialized.
     */
    String getVariant() const;

    /**
        Returns a String representation of the language tag.
        @return A String representing the LanguageTag object in RFC 3066
        syntax (for example, "language-country-variant").
        @exception UninitializedObjectException if the LanguageTag has not
        been initialized.
     */
    String toString() const;

    /**
        Tests LanguageTag objects for equality.  Comparisons are
        case-insensitive.  Distinct but equivalent language tags are not
        considered equal.  Language range matching is not performed.
        @param languageTag A LanguageTag object to be compared.
        @return True if the language tags differ only in case, false otherwise.
        @exception UninitializedObjectException if either of the
        LanguageTags has not been initialized.
     */
    Boolean operator==(const LanguageTag& languageTag) const;

    /**
        Tests LanguageTag objects for inequality.  Comparisons are
        case-insensitive.  Distinct but equivalent language tags are not
        considered equal.  Language range matching is not performed.
        @param languageTag A LanguageTag object to be compared.
        @return True if the language tags differ in more than just case,
        false otherwise.
        @exception UninitializedObjectException if either of the
        LanguageTags has not been initialized.
     */
    Boolean operator!=(const LanguageTag& languageTag) const;

private:
    LanguageTagRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif // PEGASUS_USE_EXPERIMENTAL_INTERFACES

#endif
