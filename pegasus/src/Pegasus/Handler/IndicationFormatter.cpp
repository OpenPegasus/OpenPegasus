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

#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ArrayInternal.h>

#if defined(PEGASUS_HAS_ICU) && defined(PEGASUS_INDFORMATTER_USE_ICU)
# include <unicode/locid.h>
# include <unicode/datefmt.h>
# include <unicode/unistr.h>
#endif

#include "IndicationFormatter.h"

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//
// CIMValueLocalizer
//
///////////////////////////////////////////////////////////////////////////////

class CIMValueLocalizer
{
public:

    /**
        Constructs a CIMValueLocalizer object.  The CIMValueLocalizer can be
        used to localize a CIMValue string based on a specified
        ContentLanguageList.  Localization is possible only if the
        ContentLanguageList includes no more than one language tag.

        @param contentLangs A ContentLanguageList containing a LanguageTag
            with which to perform localization.
    */
    CIMValueLocalizer(const ContentLanguageList& contentLangs);
    ~CIMValueLocalizer();

    String getLocalizedValue(Boolean value) const
    {
        return _localizeBoolean(value);
    }

    String getLocalizedValue(Uint8 value) const
    {
        return getLocalizedValue(Uint32(value));
    }

    String getLocalizedValue(Sint8 value) const
    {
        return getLocalizedValue(Sint32(value));
    }

    String getLocalizedValue(Uint16 value) const
    {
        return getLocalizedValue(Uint32(value));
    }

    String getLocalizedValue(Sint16 value) const
    {
        return getLocalizedValue(Sint32(value));
    }

    String getLocalizedValue(Uint32 value) const
    {
        Uint32 outputLength = 0;
        char buffer[22];
        const char* output = Uint32ToString(buffer, value, outputLength);
        return String(output, outputLength);
    }

    String getLocalizedValue(Sint32 value) const
    {
        Uint32 outputLength = 0;
        char buffer[22];
        const char* output = Sint32ToString(buffer, value, outputLength);
        return String(output, outputLength);
    }

    String getLocalizedValue(Uint64 value) const
    {
        Uint32 outputLength = 0;
        char buffer[22];
        const char* output = Uint64ToString(buffer, value, outputLength);
        return String(output, outputLength);
    }

    String getLocalizedValue(Sint64 value) const
    {
        Uint32 outputLength = 0;
        char buffer[22];
        const char* output = Sint64ToString(buffer, value, outputLength);
        return String(output, outputLength);
    }

    String getLocalizedValue(Real32 value) const
    {
        char buffer[32];
        sprintf(buffer, "%.7e", value);
        return String(buffer);
    }

    String getLocalizedValue(Real64 value) const
    {
        char buffer[32];
        sprintf(buffer, "%.16e", value);
        return String(buffer);
    }

    String getLocalizedValue(Char16 value) const
    {
        return String(&value, 1);
    }

    String getLocalizedValue(const String& value) const
    {
        return value;
    }

    String getLocalizedValue(const CIMDateTime& value) const
    {
        return _localizeDateTime(value);
    }

    String getLocalizedValue(const CIMObjectPath& value) const
    {
        return value.toString();
    }

    String getLocalizedValue(const CIMObject& value) const
    {
        return value.toString();
    }

    String getLocalizedValue(const CIMInstance& value) const
    {
        return CIMObject(value).toString();
    }

private:

    CIMValueLocalizer(const CIMValueLocalizer&);
    CIMValueLocalizer& operator=(const CIMValueLocalizer&);

    String _localizeDateTime(const CIMDateTime& dateTimeValue) const;
    String _localizeBoolean(Boolean booleanValue) const;

    Boolean canLocalize;
#if defined(PEGASUS_HAS_ICU) && defined(PEGASUS_INDFORMATTER_USE_ICU)
    Locale locale;
#endif
};

CIMValueLocalizer::CIMValueLocalizer(const ContentLanguageList& contentLangs)
{
    canLocalize = false;

#if defined(PEGASUS_HAS_ICU) && defined(PEGASUS_INDFORMATTER_USE_ICU)
    if (InitializeICU::initICUSuccessful())
    {
        if (contentLangs.size() == 0)
        {
            // No language specified; use the default.
            locale = Locale::getDefault();
            canLocalize = true;
        }
        else if (contentLangs.size() == 1)
        {
            LanguageTag languageTag = contentLangs.getLanguageTag(0);
            locale = Locale(
                (const char*) languageTag.getLanguage.getCString(),
                (const char*) languageTag.getCountry.getCString(),
                (const char*) languageTag.getVariant.getCString());
            canLocalize = !locale.isBogus();
        }
        else
        {
            // ContentLanguageList has multiple language tags; do not localize.
        }
    }
#endif
}

CIMValueLocalizer::~CIMValueLocalizer()
{
}

String CIMValueLocalizer::_localizeDateTime(
    const CIMDateTime& dateTimeValue) const
{
#if defined(PEGASUS_HAS_ICU) && defined(PEGASUS_INDFORMATTER_USE_ICU)
    PEG_METHOD_ENTER(TRC_IND_FORMATTER, "CIMValueLocalizer::_localizeDateTime");

    if (canLocalize)
    {
        // The CIMDateTime epoch begins 1/1/0000 (12 am Jan 1, 1BCE).
        // The ICU epoch begins 1/1/1970 (1 January 1970 0:00 UTC).

        const CIMDateTime EPOCH1970 = CIMDateTime("19700101000000.000000+000");
        UDate icuDateTimeMillisecs = (Sint64)
            (dateTimeValue.toMicroSeconds() - EPOCH1970.toMicroSeconds())/1000;

        // Use a medium length DATE/TIME format (e.g., Jan 12, 1982 3:30:32pm)
        AutoPtr<DateFormat> fmt;

        try
        {
            if (locale == 0)
            {
                fmt.reset(DateFormat::createDateTimeInstance(
                    DateFormat::MEDIUM, DateFormat::MEDIUM));
            }
            else
            {
                fmt.reset(DateFormat::createDateTimeInstance(
                    DateFormat::MEDIUM, DateFormat::MEDIUM, locale));
            }
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_IND_FORMATTER, Tracer::LEVEL2,
                "Caught exception from DateFormat::createDateTimeInstance");

            PEG_METHOD_EXIT();
            return dateTimeValue.toString();
        }

        if (fmt.get() == 0)
        {
            PEG_TRACE_CSTRING(TRC_IND_FORMATTER, Tracer::LEVEL2,
                "Memory allocation error creating DateTime instance.");
            PEG_METHOD_EXIT();
            return dateTimeValue.toString();
        }

        // Format the Date and Time
        UErrorCode status = U_ZERO_ERROR;
        UnicodeString dateTimeUniStr;
        fmt->format(icuDateTimeMillisecs, dateTimeUniStr, status);

        if (U_FAILURE(status))
        {
            PEG_METHOD_EXIT();
            return dateTimeValue.toString();
        }

        // convert UnicodeString to char *
        char dateTimeBuffer[256];
        String datetimeStr;

        // Copy the contents of the string into dateTimeBuffer
        Uint32 strLen = dateTimeUniStr.extract(
            0, sizeof(dateTimeBuffer), dateTimeBuffer);

        if (strLen >= sizeof(dateTimeBuffer))
        {
            // There is not enough space in dateTimeBuffer
            char* extractedStr = new char[strLen + 1];
            strLen = dateTimeUniStr.extract(0, strLen + 1, extractedStr);
            datetimeStr = extractedStr;
            delete [] extractedStr;
        }
        else
        {
            datetimeStr = dateTimeBuffer;
        }

        PEG_METHOD_EXIT();
        return datetimeStr;
    }

    PEG_METHOD_EXIT();
#endif

    return dateTimeValue.toString();
}

String CIMValueLocalizer::_localizeBoolean(Boolean booleanValue) const
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER, "CIMValueLocalizer::_localizeBoolean");

    if (canLocalize)
    {
        if (booleanValue)
        {
            MessageLoaderParms parms(
                "Common.IndicationFormatter._MSG_BOOLEAN_TRUE",
                "true");

            PEG_METHOD_EXIT();
            return MessageLoader::getMessage(parms);
        }
        else
        {
            MessageLoaderParms parms(
                "Common.IndicationFormatter._MSG_BOOLEAN_FALSE",
                "false");

            PEG_METHOD_EXIT();
            return MessageLoader::getMessage(parms);
        }
    }

    PEG_METHOD_EXIT();
    return booleanValue ? "true" : "false";
}


///////////////////////////////////////////////////////////////////////////////
//
// IndicationFormatter
//
///////////////////////////////////////////////////////////////////////////////

template<class T>
void appendArrayValue(
    String& buffer,
    const CIMValue& value,
    Uint32 arrayIndex,
    const CIMValueLocalizer& cimValueLocalizer)
{
    PEGASUS_ASSERT(value.isArray());

    Array<T> arrayValue;
    value.get(arrayValue);

    // Empty brackets (e.g. []), gets all values of the array
    if (arrayIndex == PEG_NOT_FOUND)
    {
        buffer.append("[");

        for (Uint32 i = 0, arraySize = arrayValue.size(); i < arraySize; i++)
        {
            buffer.append(cimValueLocalizer.getLocalizedValue(arrayValue[i]));
            if (i < arraySize - 1)
            {
                buffer.append(",");
            }
        }

        buffer.append("]");
    }
    else
    {
        buffer.append(
            cimValueLocalizer.getLocalizedValue(arrayValue[arrayIndex]));
    }
}

void IndicationFormatter::validateTextFormat(
    const String& textStr,
    const CIMClass& indicationClass,
    const Array<String>& textFormatParams)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER,
        "IndicationFormatter::validateTextFormat");

    String textFormatStr = textStr;
    Uint32 leftBrace = textFormatStr.find("{");
    Uint32 rightBrace;

    do
    {
        String textFormatSubStr;

        if (leftBrace != PEG_NOT_FOUND)
        {
            // Do not expect a right brace before the left
            // brace. e.g An invalid text format string could be:
            // "Indication occurred at 2, datetime} with
            // identify ID {3, string}"

            textFormatSubStr = textFormatStr.subString(0, leftBrace);

            Uint32 rightBrace2 = textFormatSubStr.find("}");
            if (rightBrace2 != PEG_NOT_FOUND)
            {
                textFormatSubStr = textFormatStr.subString(
                    0, (rightBrace2 + 1));
                MessageLoaderParms parms(
                    "Common.IndicationFormatter."
                        "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                    "Invalid syntax at $0 in property $1",
                    textFormatSubStr,
                    _PROPERTY_TEXTFORMAT.getString());

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
            }

            // expect right brace
            textFormatStr = textFormatStr.subString(leftBrace+1);
            rightBrace = textFormatStr.find("}");

            // Do not expect a left brace between left and right
            // braces. e.g A text string: "Indication occurred
            // at {2, datetime with identify ID {3, string}" is
            // an invalid format.

            if (rightBrace != PEG_NOT_FOUND)
            {
                textFormatSubStr.clear();
                textFormatSubStr = textFormatStr.subString(0, rightBrace);

                Uint32 leftBrace2 = textFormatSubStr.find("{");
                if (leftBrace2 != PEG_NOT_FOUND)
                {
                    textFormatSubStr = textFormatStr.subString(
                        0, (leftBrace2 + 1));
                    MessageLoaderParms parms(
                        "Common.IndicationFormatter."
                            "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                        "Invalid syntax at $0 in property $1",
                        textFormatSubStr,
                        _PROPERTY_TEXTFORMAT.getString());

                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER, parms);
                }

                String propertyParam;
                String propertyTypeStr;
                String propertyIndexStr;
                Uint32 comma = textFormatSubStr.find(",");

                if (comma == PEG_NOT_FOUND)
                {
                    // A dynamic content can have format either
                    // {index} or {index[x]}
                    propertyParam = textFormatSubStr;
                    propertyTypeStr = String::EMPTY;
                }
                else
                {
                    // A dynamic content can have format either
                    // {index, type} or {index[x], type}
                    propertyParam = textFormatSubStr.subString(0, comma);
                    propertyTypeStr = textFormatSubStr.subString(comma +1);
                }

                Uint32 leftBracket = propertyParam.find("[");
                Uint32 rightBracket = propertyParam.find("]");
                Boolean isArray = false;

                // A dynamic content has syntax either {index} or {index, type}
                if (leftBracket == PEG_NOT_FOUND)
                {
                    // there is no left bracket, do not expect a right bracket
                    if (rightBracket != PEG_NOT_FOUND)
                    {
                        textFormatSubStr = textFormatStr.subString(
                            0, (rightBracket + 1));
                        MessageLoaderParms parms(
                            "Common.IndicationFormatter."
                                "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                            "Invalid syntax at $0 in property $1",
                            textFormatSubStr,
                            _PROPERTY_TEXTFORMAT.getString());

                        PEG_METHOD_EXIT();
                        throw PEGASUS_CIM_EXCEPTION_L(
                            CIM_ERR_INVALID_PARAMETER, parms);
                    }

                    propertyIndexStr = propertyParam;
                    isArray = false;
                }
                // A dynamic content has syntax either
                // {index[]} or {index[], type}
                else
                {
                    // there is a left bracket, expect a right bracket
                    if (rightBracket == PEG_NOT_FOUND)
                    {
                        MessageLoaderParms parms(
                            "Common.IndicationFormatter."
                                "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                            "Invalid syntax at $0 in property $1",
                            textFormatSubStr,
                            _PROPERTY_TEXTFORMAT.getString());

                        PEG_METHOD_EXIT();
                        throw PEGASUS_CIM_EXCEPTION_L(
                            CIM_ERR_INVALID_PARAMETER, parms);
                    }

                    propertyIndexStr = propertyParam.subString(0, leftBracket);
                    isArray = true;
                }

                Uint32 propertyIndex = _parseIndex(propertyIndexStr);

                // check the property index
                if (propertyIndex >= textFormatParams.size())
                {
                    // property index is out of bounds
                    MessageLoaderParms parms(
                        "Common.IndicationFormatter."
                            "_MSG_INDEX_IS_OUT_OF_BOUNDS",
                        "The value of index $0 in property $1 is out of bounds",
                        propertyIndex,
                        _PROPERTY_TEXTFORMATPARAMETERS.getString());

                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER, parms);
                }

                if (propertyTypeStr != String::EMPTY)
                {
                    _validatePropertyType(indicationClass,
                        textFormatParams[propertyIndex],
                        propertyTypeStr, isArray);
                }

                textFormatStr = textFormatStr.subString(rightBrace+1);
            }
            else // no right brace
            {
                MessageLoaderParms parms(
                    "Common.IndicationFormatter."
                        "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                    "Invalid syntax at $0 in property $1",
                    textFormatSubStr,
                    _PROPERTY_TEXTFORMAT.getString());

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
            }
        }
        else // no left brace
        {
            // does not expect right brace
            rightBrace = textFormatStr.find("}");

            if (rightBrace != PEG_NOT_FOUND)
            {
                textFormatSubStr = textFormatStr.subString(0, rightBrace + 1);
                MessageLoaderParms parms(
                    "Common.IndicationFormatter."
                        "_MSG_INVALID_SYNTAX_OF_FOR_PROPERTY",
                    "Invalid syntax at $0 in property $1",
                    textFormatSubStr,
                    _PROPERTY_TEXTFORMAT.getString());

                    PEG_METHOD_EXIT();
                    throw PEGASUS_CIM_EXCEPTION_L(
                        CIM_ERR_INVALID_PARAMETER, parms);
            }

            break;
        }

        leftBrace = textFormatStr.find("{");
    } while (textFormatStr.size() > 0);

    PEG_METHOD_EXIT();
}

inline Boolean _isSpace(Char16 c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

void IndicationFormatter::_trim(String& s)
{
    while (s.size() && _isSpace(s[s.size()-1]))
    {
        s.remove(s.size() - 1);
    }

    while (s.size() && _isSpace(s[0]))
    {
        s.remove(0, 1);
    }
}

void IndicationFormatter::_validatePropertyType(
    const CIMClass& indicationClass,
    const String& propertyParam,
    const String& typeStr,
    const Boolean& isArray)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER,
        "IndicationFormatter::_validatePropertyType");

    String propertyTypeStr = typeStr;

    Array<String> validPropertyTypes;
    validPropertyTypes.append("boolean");
    validPropertyTypes.append("uint8");
    validPropertyTypes.append("sint8");
    validPropertyTypes.append("uint16");
    validPropertyTypes.append("sint16");
    validPropertyTypes.append("uint32");
    validPropertyTypes.append("sint32");
    validPropertyTypes.append("uint64");
    validPropertyTypes.append("sint64");
    validPropertyTypes.append("real32");
    validPropertyTypes.append("real64");
    validPropertyTypes.append("char16");
    validPropertyTypes.append("string");
    validPropertyTypes.append("datetime");
    validPropertyTypes.append("reference");

    propertyTypeStr.toLower();
    _trim(propertyTypeStr);

    //
    // Checks if the provided property type is a valid type
    //
    if (!(Contains(validPropertyTypes, propertyTypeStr)))
    {
        // the provided property type is not valid type
        MessageLoaderParms parms(
            "Common.IndicationFormatter."
                "_MSG_INVALID_TYPE_OF_FOR_PROPERTY",
            "Invalid property type of $0 in property $1",
            propertyTypeStr,
            _PROPERTY_TEXTFORMAT.getString());

        PEG_METHOD_EXIT();
        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
    }

    for (Uint32 i = 0; i < indicationClass.getPropertyCount(); i++)
    {
        CIMName propertyName = indicationClass.getProperty(i).getName();

        if (String::equalNoCase(propertyParam, propertyName.getString()))
        {
            // get the property type;
            CIMType propertyType = indicationClass.getProperty(i).getType();

            // Check if the property is an array type
            if ((isArray && !(indicationClass.getProperty(i).isArray())) ||
                (!isArray && indicationClass.getProperty(i).isArray()))
            {
                MessageLoaderParms parms(
                    "Common.IndicationFormatter."
                        "_MSG_PROPERTY_IS_NOT_AN_ARRAY_TYPE",
                    "The property $0 is not an array type",
                    propertyName.getString());

                PEG_METHOD_EXIT();
                throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
            }

            // property type matches
            if (String::equalNoCase(
                    propertyTypeStr, cimTypeToString(propertyType)))
            {
                break;
            }

            MessageLoaderParms parms(
                "Common.IndicationFormatter."
                    "_MSG_MISS_MATCHED_TYPE_OF_FOR_PROPERTY",
                "The provided property type of $0 in $1 does not match "
                    "the property type $2",
                propertyTypeStr,
                cimTypeToString(propertyType),
                _PROPERTY_TEXTFORMAT.getString());

            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }
    }

    PEG_METHOD_EXIT();
}

void IndicationFormatter::validateTextFormatParameters(
    const CIMPropertyList& propertyList,
    const CIMClass& indicationClass,
    const Array<String>& textFormatParams)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER,
        "IndicationFormatter::validateTextFormatParameters");

    Array<String> indicationClassProperties;

    if (propertyList.isNull())
    {
        // All the properties are selected
        for (Uint32 i = 0; i < indicationClass.getPropertyCount(); i++)
        {
            indicationClassProperties.append(
                indicationClass.getProperty(i).getName().getString());
        }
    }
    else
    {
        // Partial properties are selected
        Array<CIMName> propertyNames = propertyList.getPropertyNameArray();

        for (Uint32 j = 0; j < propertyNames.size(); j++)
        {
            indicationClassProperties.append(propertyNames[j].getString());
        }
    }

    // check if the textFormatParams is contained in the
    // indicationClassProperties
    for (Uint32 k = 0; k < textFormatParams.size(); k++)
    {
        if (!Contains(indicationClassProperties, textFormatParams[k]))
        {
            // The property name in TextFormatParameters is not
            // included in the select clause of the associated filter query
            MessageLoaderParms parms(
                "Common.IndicationFormatter."
                    "_MSG_MISS_MATCHED_PROPERTY_NAME",
                "The property name $0 in $1 does not match the properties "
                    "in the select clause",
                textFormatParams[k],
                _PROPERTY_TEXTFORMATPARAMETERS.getString());

            PEG_METHOD_EXIT();
            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
        }
    }

    PEG_METHOD_EXIT();
}

Uint32 IndicationFormatter::_parseIndex(const String& indexStr)
{
    Uint32 index = PEG_NOT_FOUND;
    char dummy[2];
    int numConversions =
        sscanf(indexStr.getCString(), "%u%1s", &index, dummy);

    if ((numConversions != 1) || (index == PEG_NOT_FOUND))
    {
        MessageLoaderParms parms(
            "IndicationFormatter.IndicationFormatter._MSG_INVALID_INDEX",
            "Invalid index string '$0'",
            indexStr);

        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_INVALID_PARAMETER, parms);
    }

    return index;
}

String IndicationFormatter::getFormattedIndText(
    const CIMInstance& subscription,
    const CIMInstance& indication,
    const ContentLanguageList& contentLangs)
{
    PEG_METHOD_ENTER (TRC_IND_FORMATTER,
        "IndicationFormatter::getFormattedIndText");

    String indicationText;
    String textFormat;
    CIMValue textFormatValue;
    CIMValue textFormatParamsValue;

    Array<String> textFormatParams;

    // get TextFormat from subscription
    Uint32 textFormatPos = subscription.findProperty(_PROPERTY_TEXTFORMAT);

    // if the property TextFormat is not found,
    // indication is constructed with default format
    if (textFormatPos == PEG_NOT_FOUND)
    {
        indicationText = _formatDefaultIndicationText(indication, contentLangs);
    }
    else
    {
        textFormatValue = subscription.getProperty(textFormatPos).getValue();

        // if the value of textFormat is NULL,
        // indication is constructed with default format
        if (textFormatValue.isNull())
        {
            indicationText =
                _formatDefaultIndicationText(indication, contentLangs);
        }
        else
        {
            // get TextFormatParameters from subscription
            Uint32 textFormatParamsPos = subscription.findProperty(
                _PROPERTY_TEXTFORMATPARAMETERS);

            if (textFormatParamsPos != PEG_NOT_FOUND)
            {
                textFormatParamsValue = subscription.getProperty(
                    textFormatParamsPos).getValue();
            }

            // constructs indication with specified format
            if ((textFormatValue.getType() == CIMTYPE_STRING) &&
                !(textFormatValue.isArray()))
            {
                textFormatValue.get(textFormat);
                if (!textFormatParamsValue.isNull())
                {
                    if ((textFormatParamsValue.getType() == CIMTYPE_STRING) &&
                        (textFormatParamsValue.isArray()))
                    {
                        textFormatParamsValue.get(textFormatParams);
                    }
                }

                indicationText = _formatIndicationText(
                    textFormat,
                    textFormatParams,
                    indication,
                    contentLangs);
            }
            else
            {
                indicationText =
                    _formatDefaultIndicationText(indication, contentLangs);
            }
        }
    }

    PEG_METHOD_EXIT();
    return indicationText;
}

String IndicationFormatter::_formatDefaultIndicationText(
    const CIMInstance& indication,
    const ContentLanguageList& contentLangs)
{
    PEG_METHOD_ENTER (TRC_IND_FORMATTER,
        "IndicationFormatter::_formatDefaultIndicationText");

    String indicationStr = "Indication (default format):";

    CIMValueLocalizer cimValueLocalizer(contentLangs);

    for (Uint32 i = 0, n = indication.getPropertyCount(); i < n; i++)
    {
        if (i > 0)
        {
            indicationStr.append(", ");
        }

        CIMConstProperty property = indication.getProperty(i);
        CIMValue propertyValue = property.getValue();

        indicationStr.append(property.getName().getString());
        indicationStr.append(" = ");

        if (!propertyValue.isNull())
        {
            if (propertyValue.isArray())
            {
                indicationStr.append(_getArrayValues(
                    propertyValue, PEG_NOT_FOUND, contentLangs));
            }
            else // value is not an array
            {
                CIMType type = propertyValue.getType();

                if (type == CIMTYPE_DATETIME)
                {
                    CIMDateTime dateTimeValue;
                    propertyValue.get(dateTimeValue);
                    indicationStr.append(
                        cimValueLocalizer.getLocalizedValue(dateTimeValue));
                }
                else if (type == CIMTYPE_BOOLEAN)
                {
                    Boolean booleanValue;
                    propertyValue.get(booleanValue);
                    indicationStr.append(
                        cimValueLocalizer.getLocalizedValue(booleanValue));
                }
                else
                {
                    indicationStr.append(propertyValue.toString());
                }
            }
        }
        else
        {
            indicationStr.append("NULL");
        }
    }

    PEG_METHOD_EXIT();
    return indicationStr;
}

String IndicationFormatter::_formatIndicationText(
    const String& textFormat,
    const Array<String>& textFormatParams,
    const CIMInstance& indication,
    const ContentLanguageList& contentLangs)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER,
        "IndicationFormatter::_formatIndicationText");

    String indicationText;
    String indicationFormat = textFormat;

    // Parsing the specified indication text format.
    // As an example, a format string for a UPS AlertIndication
    // could be defined as follows: A {4, string} UPS Alert was
    // detected on the device {6[1]}.
    Uint32 leftBrace;
    while ((leftBrace = indicationFormat.find("{")) != PEG_NOT_FOUND)
    {
        // Append the text up to the left brace
        indicationText.append(indicationFormat.subString(0, leftBrace));
        indicationFormat = indicationFormat.subString(leftBrace+1);
        Uint32 rightBrace = indicationFormat.find("}");

        // expecting a right brace
        if (rightBrace != PEG_NOT_FOUND)
        {
            // gets property index which is inside braces.
            // The supported formats are: {index} or {index, type}
            // or {index[x]} or {index[x], type}
            String propertyParam = indicationFormat.subString(0, rightBrace);
            Uint32 comma = propertyParam.find(",");

            // A dynamic content has syntax {index, type} or {index[x], type}
            if (comma != PEG_NOT_FOUND)
            {
                propertyParam = propertyParam.subString(0, comma);
            }

            String propertyIndexStr;
            String arrayIndexStr;
            Uint32 leftBracket = propertyParam.find("[");

            if (leftBracket == PEG_NOT_FOUND)
            {
                // A dynamic content has syntax {index} or {index, type}
                propertyIndexStr = propertyParam;
            }
            else
            {
                // A dynamic content has syntax {index[x]} or {index[x], type}
                propertyIndexStr = propertyParam.subString(0, leftBracket);
                propertyParam = propertyParam.subString(leftBracket);

                Uint32 rightBracket = propertyParam.find("]");
                arrayIndexStr = propertyParam.subString(1, rightBracket-1);
            }

            String propertyValue;

            try
            {
                Uint32 propertyIndex = _parseIndex(propertyIndexStr);
                Uint32 arrayIndex = PEG_NOT_FOUND;

                if (arrayIndexStr.size())
                {
                    arrayIndex = _parseIndex(arrayIndexStr);
                }

                if (propertyIndex >= textFormatParams.size())
                {
                    // Property index is out of range
                    propertyValue = "UNKNOWN";
                }
                else
                {
                    // get indication property value
                    propertyValue = _getIndPropertyValue(
                        textFormatParams[propertyIndex],
                        arrayIndex,
                        indication,
                        contentLangs);
                }
            }
            catch (CIMException& c)
            {
                PEG_TRACE((TRC_IND_FORMATTER, Tracer::LEVEL2,
                    "Exception at parsing indication property: %s",
                    (const char*)c.getMessage().getCString()));
                propertyValue = "UNKNOWN";
            }

            indicationText.append(propertyValue);
        }

        indicationFormat = indicationFormat.subString(rightBrace+1);
    }

    indicationText.append(indicationFormat);

    PEG_METHOD_EXIT();
    return indicationText;
}

String IndicationFormatter::_getIndPropertyValue(
    const String& propertyName,
    Uint32 arrayIndex,
    const CIMInstance& indication,
    const ContentLanguageList& contentLangs)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER,
        "IndicationFormatter::_getIndPropertyValue");

    Uint32 pos = indication.findProperty(propertyName);

    if (pos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        return "UNKNOWN";
    }

    CIMConstProperty property = indication.getProperty(pos);
    CIMValue propertyValue = property.getValue();

    if (propertyValue.isNull())
    {
        PEG_METHOD_EXIT();
        return "NULL";
    }

    if (propertyValue.isArray())
    {
        PEG_METHOD_EXIT();
        return _getArrayValues(propertyValue, arrayIndex, contentLangs);
    }

    // Value is not an array

    CIMValueLocalizer cimValueLocalizer(contentLangs);

    if (propertyValue.getType() == CIMTYPE_DATETIME)
    {
        CIMDateTime dateTimeValue;
        propertyValue.get(dateTimeValue);
        PEG_METHOD_EXIT();
        return cimValueLocalizer.getLocalizedValue(dateTimeValue);
    }

    if (propertyValue.getType() == CIMTYPE_BOOLEAN)
    {
        Boolean booleanValue;
        propertyValue.get(booleanValue);
        PEG_METHOD_EXIT();
        return cimValueLocalizer.getLocalizedValue(booleanValue);
    }

    PEG_METHOD_EXIT();
    return propertyValue.toString();
}

String IndicationFormatter::_getArrayValues(
    const CIMValue& value,
    Uint32 arrayIndex,
    const ContentLanguageList& contentLangs)
{
    PEG_METHOD_ENTER(TRC_IND_FORMATTER, "IndicationFormatter::_getArrayValues");

    if ((arrayIndex != PEG_NOT_FOUND) &&
        (arrayIndex >= value.getArraySize()))
    {
        // Array index is out of range
        PEG_METHOD_EXIT();
        return "UNKNOWN";
    }

    String arrayValues;
    CIMValueLocalizer cimValueLocalizer(contentLangs);

    switch (value.getType())
    {
        case CIMTYPE_UINT8:
        {
            appendArrayValue<Uint8>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_SINT8:
        {
            appendArrayValue<Sint8>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_UINT16:
        {
            appendArrayValue<Uint16>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_SINT16:
        {
            appendArrayValue<Sint16>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_UINT32:
        {
            appendArrayValue<Uint32>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_SINT32:
        {
            appendArrayValue<Sint32>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_UINT64:
        {
            appendArrayValue<Uint64>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_SINT64:
        {
            appendArrayValue<Sint64>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_REAL32:
        {
            appendArrayValue<Real32>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_REAL64:
        {
            appendArrayValue<Real64>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_BOOLEAN:
        {
            appendArrayValue<Boolean>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_CHAR16:
        {
            appendArrayValue<Char16>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_STRING:
        {
            appendArrayValue<String>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_DATETIME:
        {
            appendArrayValue<CIMDateTime>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_REFERENCE:
        {
            appendArrayValue<CIMObjectPath>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_OBJECT:
        {
            appendArrayValue<CIMObject>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        case CIMTYPE_INSTANCE:
        {
            appendArrayValue<CIMInstance>(
                arrayValues, value, arrayIndex, cimValueLocalizer);
            break;
        }

        default:
        {
            PEG_TRACE((TRC_IND_FORMATTER, Tracer::LEVEL2,
                "Unknown CIMType: %u",
                value.getType()));

            arrayValues.append("UNKNOWN");
            break;
        }
    }

    PEG_METHOD_EXIT();
    return arrayValues;
}

PEGASUS_NAMESPACE_END
