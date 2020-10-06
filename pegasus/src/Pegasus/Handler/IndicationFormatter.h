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

#ifndef Pegasus_IndicationFormatter_h
#define Pegasus_IndicationFormatter_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/ContentLanguageList.h>

#include <Pegasus/Handler/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The IndicationFormatter class formats an indication message.
*/
class PEGASUS_HANDLER_LINKAGE IndicationFormatter
{
public:

    /**
        Based on subscription instance, gets the formatted indication text
        message from the received indication instance.

        @param   subscription   the subscription instance
        @param   indication     the received indication instance
        @param   contentLangs   the Content-Languages in the
                                subscription instance

        @return  String containing the formatted indication text message
    */
    static String getFormattedIndText(
        const CIMInstance& subscription,
        const CIMInstance& indication,
        const ContentLanguageList& contentLangs);


    /**
        Validates the syntax and the provided type for the property
        TextFormat in the instance.
        If the value of the property has a syntax error, or the
        provided type does not match the property type,
        an exception is thrown.

        This function is called by the _canCreate function, and is used to
        validate the syntax and the provided type for the property
        TextFormat in the Formatted Subscription instance.

        @param   textFormatStr     the string to be validated
        @param   indicationClass   the indication class
        @param   textFormatParams  the value of the TextFormatParameters

        @throw   CIM_ERR_INVALID_PARAMETER  if there is a syntax error
                                            for the value of property
                                            TextFormat or type mismatch
     */
    static void validateTextFormat(
        const String& textFormatStr,
        const CIMClass& indicationClass,
        const Array<String>& textFormatParams);

    /**
        Validates the value of the property TextFormatParameters in the
        subscriptionInstance.
        If the property name in the TextFormatParameters is not the name
        contained in the select clause of the associated filter query
        in the filterInstance, an exception is thrown.

        @param   propertyList        the list of properties to be validated
        @param   indicationClass     the indication class
        @param   textFormatParams    the value of the property
                                     TextFormatParameters

        @throw   CIM_ERR_INVALID_PARAMETER  if the select clause of the
                                            filter query does not contain
                                            the provided property name
     */
    static void validateTextFormatParameters(
        const CIMPropertyList& propertyList,
        const CIMClass& indicationClass,
        const Array<String>& textFormatParams);

private:

    /**
        Gets a specified indication property value from an indication instance.

        @param propertyName The name of the property for which to get the value.
        @param arrayIndex The array index of the value to get, if the property
            has an array value.  A value of PEG_NOT_FOUND indicates that all
            array values are requested.
        @param indication The indication from which to get the property value.
        @param contentLangs The language in which to encode the value.
        @return String containing property value
    */
    static String _getIndPropertyValue(
        const String& propertyName,
        Uint32 arrayIndex,
        const CIMInstance& indication,
        const ContentLanguageList& contentLangs);

    /**
        Retrieves the array values contained by the specified CIMValue.

        @param value The CIMValue from which to get the array values.
        @param arrayIndex The array index for which to get the value.  A value
            of PEG_NOT_FOUND indicates that all values are requested.
        @param contentLangs The language in which to encode the value.
        @return String containing array values contained by the CIMValue.
    */
    static String _getArrayValues(
        const CIMValue& value,
        Uint32 arrayIndex,
        const ContentLanguageList& contentLangs);

    /**
        Parses an index value from a string.  The string must represent an
        unsigned integer.  Leading and trailing whitespace is ignored.

        @param indexStr The string from which to parse the index.
        @throw CIM_ERR_INVALID_PARAMETER If the string is improperly formatted.
     */
    static Uint32 _parseIndex(const String& indexStr);

    /**
        Trims leading and trailing whitespace from a String.
        @param s The string from which to trim whitespace.
    */
    static void _trim(String& s);

    /**
        Validates the provided property type string.
        If the provided property type does not match the type of the property,
        or the provided property type is not valid type, or the dynamic
        content with an array index refers to a non array type, an
        exception is thrown.

        This function is called by the _validatePropertySyntaxAndType
        function, and is used to validate the provided property type.

        @param   indicationClass   the indication class
        @param   propertyParam     the provided property
        @param   propertyTypeStr   the provided property type
        @param   isArray           indicates whether refers to an array type

        @throw   CIM_ERR_INVALID_PARAMETER  if the provided property type is
                                            unexpected
     */
    static void _validatePropertyType(
        const CIMClass& indicationClass,
        const String& propertyParam,
        const String& propertyTypeStr,
        const Boolean& isArray);

    /**
        Constructs a default indication text message from the received
        indication instance.

        @param   indication     the received indication instance
        @param   contentLangs   the Content-Languages in the
                                subscription instance

        @return  String containing default indication text message
    */
    static String _formatDefaultIndicationText(
        const CIMInstance& indication,
        const ContentLanguageList& contentLangs);

    /**
        Constructs a human readable indication text message from the
        specified indication textFormat, textFormatParams, and
        received indication instance.

        @param   textFormat        the specified indication textFormat
        @param   textFormatParams  the indexed array containing the names
                                   of properties defined in the select
                                   clause of the associated
                                   CIM_IndicationFilter Query
        @param   indication        the received indication instance
        @param   contentLangs      the Content-Languages in the
                                   subscription instance

        @return  String containing a human readable indication text message
    */
    static String _formatIndicationText(
        const String& textFormat,
        const Array<String>& textFormatParams,
        const CIMInstance& indication,
        const ContentLanguageList& contentLangs);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_IndicationFormatter_h */
