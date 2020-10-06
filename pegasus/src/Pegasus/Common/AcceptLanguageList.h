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

#ifndef Pegasus_AcceptLanguageList_h
#define Pegasus_AcceptLanguageList_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/LanguageTag.h>

PEGASUS_NAMESPACE_BEGIN

class AcceptLanguageListRep;

//////////////////////////////////////////////////////////////
//
// AcceptLanguageList
//
//////////////////////////////////////////////////////////////

/**
    This class represents an list of languages that a reader can understand
    (as may be specified in an HTTP Accept-Language header value).  It is
    managed as a prioritized list of LanguageTag objects and quality values.
 */
class PEGASUS_COMMON_LINKAGE AcceptLanguageList
{
public:

    /**
        Constructs an empty AcceptLanguageList object.
     */
    AcceptLanguageList();

    /**
        Copy constructor.
        @param acceptLanguages The AcceptLanguageList object to copy.
     */
    AcceptLanguageList(const AcceptLanguageList& acceptLanguages);

    /**
        Destructor.
     */
    ~AcceptLanguageList();

    /**
        Assignment operator.
        @param acceptLanguages The AcceptLanguageList object to copy.
     */
    AcceptLanguageList& operator=(const AcceptLanguageList& acceptLanguages);

    /**
        Returns the number of LanguagesTags in the AcceptLanguageList object.
        @return Integer size of the AcceptLanguageList list.
     */
    Uint32 size() const;

    /**
        Accesses an LanguageTag at a specified index.
        @param index Integer index of the LanguageTag to access.
        Valid indices range from 0 to size()-1.
        @return The LanguageTag corresponding to the specified index.
        @exception IndexOutOfBoundsException If the specified index is out of
        range.
     */
    LanguageTag getLanguageTag(Uint32 index) const;

    /**
        Accesses a quality value at a specified index (corresponding to a
        language tag).
        @param index Integer index of the quality value to access.
        Valid indices range from 0 to size()-1.
        @return The quality value corresponding to the specified index.
        @exception IndexOutOfBoundsException If the specified index is out of
        range.
     */
    Real32 getQualityValue(Uint32 index) const;

    /**
        Inserts a LanguageTag and quality value into the AcceptLanguageList
        object.  The element is inserted in order of descending quality value
        and after any other elements with the same quality value.
        @param languageTag The LanguageTag to insert.
        @param qualityValue The quality value to insert.
     */
    void insert(
        const LanguageTag& languageTag,
        Real32 qualityValue);

    /**
        Removes the specified LanguageTag and quality value from the
        AcceptLanguageList object.
        @param index Integer index of the element to remove.
        @exception IndexOutOfBoundsException If the specified index is out of
        range.
     */
    void remove(Uint32 index);

    /**
        Finds the first occurrence of the specified LanguageTag in the
        AcceptLanguageList object and returns its index.
        @param languageTag The LanguageTag to find.
        @return Integer index of the element, if found; otherwise
        PEG_NOT_FOUND.
     */
    Uint32 find(const LanguageTag& languageTag) const;

    /**
        Removes all the LanguageTags and quality values from the
        AcceptLanguageList object.
     */
    void clear();

    /**
        Tests AcceptLanguageList objects for equality.
        @param acceptLanguages An AcceptLanguageList object to be compared.
        @return True if the AcceptLanguageList objects contain the same
        LanguageTags and quality values in the same order, false otherwise.
     */
    Boolean operator==(const AcceptLanguageList& acceptLanguages) const;

    /**
        Tests AcceptLanguageList objects for inequality.
        @param acceptLanguages An AcceptLanguageList object to be compared.
        @return False if the AcceptLanguageList objects contain the same
        LanguageTags and quality values in the same order, true otherwise.
     */
    Boolean operator!=(const AcceptLanguageList& acceptLanguages) const;

private:
    AcceptLanguageListRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif
