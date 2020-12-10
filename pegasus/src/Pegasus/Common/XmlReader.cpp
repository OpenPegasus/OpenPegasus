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
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <errno.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#if defined(PEGASUS_OS_TYPE_UNIX) || defined(PEGASUS_OS_VMS)
# include <errno.h>
#endif
#include "CIMName.h"
#include "CIMNameCast.h"
#include "XmlReader.h"
#include "XmlWriter.h"
#include "CIMQualifier.h"
#include "CIMQualifierDecl.h"
#include "CIMClass.h"
#include "CIMInstance.h"
#include "CIMObject.h"
#include "CIMParamValue.h"
#include "System.h"
#include "XmlConstants.h"

#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/AutoPtr.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

static const Uint32 MESSAGE_SIZE = 128;

//------------------------------------------------------------------------------
//
// getXmlDeclaration()
//
//     <?xml version="1.0" encoding="utf-8"?>
//
//------------------------------------------------------------------------------

void XmlReader::getXmlDeclaration(
    XmlParser& parser,
    const char*& xmlVersion,
    const char*& xmlEncoding)
{
    XmlEntry entry;

    if (!parser.next(entry) ||
        entry.type != XmlEntry::XML_DECLARATION ||
        strcmp(entry.text, "xml") != 0)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_XML_STYLE",
            "Expected <?xml ... ?> style declaration");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (!entry.getAttributeValue("version", xmlVersion))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_XML_ATTRIBUTE",
            "missing xml.version attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (!entry.getAttributeValue("encoding", xmlEncoding))
    {
        // ATTN-RK-P3-20020403:  Is there a default encoding?
    }
}

//------------------------------------------------------------------------------
//
//  testXmlDeclaration ()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testXmlDeclaration (
    XmlParser& parser,
    XmlEntry& entry)
{
    if (!parser.next (entry) ||
        entry.type != XmlEntry::XML_DECLARATION ||
        strcmp (entry.text, "xml") != 0)
    {
        parser.putBack (entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// expectStartTag()
//
//------------------------------------------------------------------------------

void XmlReader::expectStartTag(
    XmlParser& parser,
    XmlEntry& entry,
    const char* tagName)
{
    if (!parser.next(entry) ||
        entry.type != XmlEntry::START_TAG ||
        strcmp(entry.text, tagName) != 0)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_OPEN",
            "Expected open of $0 element",
            tagName);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// expectEndTag()
//
//------------------------------------------------------------------------------

void XmlReader::expectEndTag(XmlParser& parser, const char* tagName)
{
    XmlEntry entry;

    if (!parser.next(entry) ||
        entry.type != XmlEntry::END_TAG ||
        strcmp(entry.text, tagName) != 0)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CLOSE",
            "Expected close of $0 element, got $1 instead",
            tagName,entry.text);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// expectStartTagOrEmptyTag()
//
//------------------------------------------------------------------------------

void XmlReader::expectStartTagOrEmptyTag(
    XmlParser& parser,
    XmlEntry& entry,
    const char* tagName)
{
    if (!parser.next(entry) ||
        (entry.type != XmlEntry::START_TAG &&
        entry.type != XmlEntry::EMPTY_TAG) ||
        strcmp(entry.text, tagName) != 0)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_OPENCLOSE",
            "Expected either open or open/close $0 element",
            tagName);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// expectContentOrCData()
//
//------------------------------------------------------------------------------

Boolean XmlReader::expectContentOrCData(
    XmlParser& parser,
    XmlEntry& entry)
{
    if (!parser.next(entry) ||
        (entry.type != XmlEntry::CONTENT &&
        entry.type != XmlEntry::CDATA))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CDATA",
            "Expected content or CDATA");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//------------------------------------------------------------------------------
//
// testStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testStartTag(
    XmlParser& parser,
    XmlEntry& entry,
    const char* tagName)
{
    if (!parser.next(entry) ||
        entry.type != XmlEntry::START_TAG ||
        strcmp(entry.text, tagName) != 0)
    {
        parser.putBack(entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// testEndTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testEndTag(XmlParser& parser, const char* tagName)
{
    XmlEntry entry;

    if (!parser.next(entry) ||
        entry.type != XmlEntry::END_TAG ||
        strcmp(entry.text, tagName) != 0)
    {
        parser.putBack(entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// testStartTagOrEmptyTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testStartTagOrEmptyTag(
    XmlParser& parser,
    XmlEntry& entry,
    const char* tagName)
{
    if (!parser.next(entry))
    {
        return false;
    }
    if ((entry.type != XmlEntry::START_TAG &&
         entry.type != XmlEntry::EMPTY_TAG) ||
        strcmp(entry.text, tagName) != 0)
    {
        parser.putBack(entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// testStartTagOrEmptyTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testStartTagOrEmptyTag(
    XmlParser& parser,
    XmlEntry& entry)
{
    if (!parser.next(entry) ||
        (entry.type != XmlEntry::START_TAG &&
         entry.type != XmlEntry::EMPTY_TAG))
    {
        parser.putBack(entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// testContentOrCData()
//
//------------------------------------------------------------------------------

Boolean XmlReader::testContentOrCData(
    XmlParser& parser,
    XmlEntry& entry)
{
    if (!parser.next(entry) ||
        (entry.type != XmlEntry::CONTENT &&
        entry.type != XmlEntry::CDATA))
    {
        parser.putBack(entry);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getCimStartTag()
//
//     <!ELEMENT CIM (MESSAGE|DECLARATION)>
//     <!ATTRLIST CIM
//         CIMVERSION CDATA #REQUIRED
//         DTDVERSION CDATA #REQUIRED>
//
//------------------------------------------------------------------------------

void XmlReader::getCimStartTag(
    XmlParser& parser,
    const char*& cimVersion,
    const char*& dtdVersion)
{
    XmlEntry entry;
    XmlReader::expectStartTag(parser, entry, "CIM");

    if (!entry.getAttributeValue("CIMVERSION", cimVersion))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_CIMVERSION_ATTRIBUTE",
            "missing CIM.CIMVERSION attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (!entry.getAttributeValue("DTDVERSION", dtdVersion))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_DTDVERSION_ATTRIBUTE",
            "missing CIM.DTDVERSION attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// getCimNameAttribute()
//
//     <!ENTITY % CIMName "NAME CDATA #REQUIRED">
//
//------------------------------------------------------------------------------

CIMName XmlReader::getCimNameAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* elementName,
    Boolean acceptNull)
{
    const char* name;

    if (!entry.getAttributeValue("NAME", name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.NAME", elementName);
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_ATTRIBUTE",
            "missing $0 attribute",
            buffer);

        throw XmlValidationError(lineNumber, mlParms);
    }

    if (acceptNull && *name == '\0')
        return CIMName();

    Uint32 size = CIMNameLegalASCII(name);

    if (size)
    {
        String tmp(name, size);
        return CIMName(CIMNameCast(tmp));
    }

    if (!CIMName::legal(name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.NAME", elementName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);

        throw XmlSemanticError(lineNumber, mlParms);
    }

    return CIMNameCast(name);
}

//------------------------------------------------------------------------------
//
// getClassNameAttribute()
//
//     <!ENTITY % CIMName "CLASSNAME CDATA #REQUIRED">
//
//------------------------------------------------------------------------------

String XmlReader::getClassNameAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* elementName)
{
    String name;

    if (!entry.getAttributeValue("CLASSNAME", name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.CLASSNAME", elementName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_ATTRIBUTE",
            "missing $0 attribute",
            buffer);

        throw XmlValidationError(lineNumber, mlParms);
    }

    if (!CIMName::legal(name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.CLASSNAME", elementName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);
        throw XmlSemanticError(lineNumber, mlParms);
    }

    return name;
}

//------------------------------------------------------------------------------
//
// getClassOriginAttribute()
//
//     <!ENTITY % ClassOrigin "CLASSORIGIN CDATA #IMPLIED">
//
//------------------------------------------------------------------------------

CIMName XmlReader::getClassOriginAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* tagName)
{
    String name;

    if (!entry.getAttributeValue("CLASSORIGIN", name))
        return CIMName();

    /* Interoperability hack to make the C++ client of OpenPegasus able
       to deal with the wbemservices CIMOM delivered with Sun Solaris.
       The issue is that the wbemservices delivers Xml responses with
       CLASSORIGIN=""
       Originally this had been reported with Bug#537.
    */
    if (name.size()==0)
    {
        return CIMName();
    }

    if (!CIMName::legal(name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.CLASSORIGIN", tagName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);
        throw XmlSemanticError(lineNumber, mlParms);
    }
    // The CIMName was already checked with legal() + String()
    return CIMNameCast(name);
}

//------------------------------------------------------------------------------
//
// getEmbeddedObjectAttribute()
//
//     <!ENTITY % EmbeddedObject "EmbeddedObject (object | instance) #IMPLIED">
//
//     EmbeddedObjectAttributeType:
//        NO_EMBEDDED_OBJECT     = 0,
//        EMBEDDED_OBJECT_ATTR   = 1,
//        EMBEDDED_INSTANCE_ATTR = 2
//
//------------------------------------------------------------------------------

XmlReader::EmbeddedObjectAttributeType XmlReader::getEmbeddedObjectAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* tagName)
{
    const char* embeddedObject;

    // Check for both upper case and mixed case "EmbeddedObject"
    // because of an error in an earlier pegasus version  where we
    // used upper case in the generation of the attribute name
    // whereas the DMTF spec calls for mixed case.
    if (!entry.getAttributeValue("EmbeddedObject", embeddedObject) &&
        !entry.getAttributeValue("EMBEDDEDOBJECT", embeddedObject))
        return NO_EMBEDDED_OBJECT;

    // The embeddedObject attribute, if present, must have the string
    // value "object" or "instance".
    if (strcmp(embeddedObject, "object") == 0)
    {
        return EMBEDDED_OBJECT_ATTR;
    }

    if (strcmp(embeddedObject, "instance") == 0)
    {
        return EMBEDDED_INSTANCE_ATTR;
    }

    char buffer[MESSAGE_SIZE];
    sprintf(buffer, "%s.EmbeddedObject", tagName);

    MessageLoaderParms mlParms(
        "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
        "Illegal value for $0 attribute",
        buffer);
    throw XmlSemanticError(lineNumber, mlParms);

    return NO_EMBEDDED_OBJECT;
}

//------------------------------------------------------------------------------
//
// getReferenceClassAttribute()
//
//     <!ENTITY % ReferenceClass "REFERENCECLASS CDATA #IMPLIED">
//
//------------------------------------------------------------------------------

CIMName XmlReader::getReferenceClassAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* elementName)
{
    String name;

    if (!entry.getAttributeValue("REFERENCECLASS", name))
        return CIMName();

    if (!CIMName::legal(name))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.REFERENCECLASS", elementName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);
        throw XmlSemanticError(lineNumber, mlParms);
    }
    // The CIMName was already checked with legal() + String()
    return CIMNameCast(name);
}

//------------------------------------------------------------------------------
//
// getSuperClassAttribute()
//
//     <!ENTITY % SuperClass "SUPERCLASS CDATA #IMPLIED">
//
//------------------------------------------------------------------------------

CIMName XmlReader::getSuperClassAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* tagName)
{
    String superClass;

    if (!entry.getAttributeValue("SUPERCLASS", superClass))
        return CIMName();

    if (!CIMName::legal(superClass))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.SUPERCLASS", tagName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);
        throw XmlSemanticError(lineNumber, mlParms);
    }
    // The CIMName was already checked with legal() + String()
    return CIMNameCast(superClass);
}

//------------------------------------------------------------------------------
//
// getCimTypeAttribute()
//
// This method can be used to get a TYPE attribute or a PARAMTYPE attribute.
// The only significant difference is that PARAMTYPE may specify a value of
// "reference" type.  This method recognizes these attributes by name, and
// does not allow a "TYPE" attribute to be of "reference" type.
//
//     <!ENTITY % CIMType "TYPE (boolean|string|char16|uint8|sint8|uint16
//         |sint16|uint32|sint32|uint64|sint64|datetime|real32|real64)">
//
//     <!ENTITY % ParamType "PARAMTYPE (boolean|string|char16|uint8|sint8
//         |uint16|sint16|uint32|sint32|uint64|sint64|datetime|real32|real64
//         |reference)">
//
//------------------------------------------------------------------------------

Boolean XmlReader::getCimTypeAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    CIMType& cimType,  // Output parameter
    const char* tagName,
    const char* attributeName,
    Boolean required)
{
    const char* typeName;

    if (!entry.getAttributeValue(attributeName, typeName))
    {
        if (required)
        {
            char message[MESSAGE_SIZE];
            sprintf(message, "%s.%s", tagName, attributeName);

            MessageLoaderParms mlParms(
                "Common.XmlReader.MISSING_ATTRIBUTE",
                "missing $0 attribute",
                message);
            throw XmlValidationError(lineNumber, mlParms);
        }
        else
        {
            return false;
        }
    }

    CIMType type = CIMTYPE_BOOLEAN;
    Boolean unrecognizedType = false;

    if (strcmp(typeName, "boolean") == 0)
        type = CIMTYPE_BOOLEAN;
    else if (strcmp(typeName, "string") == 0)
        type = CIMTYPE_STRING;
    else if (strcmp(typeName, "char16") == 0)
        type = CIMTYPE_CHAR16;
    else if (strcmp(typeName, "uint8") == 0)
        type = CIMTYPE_UINT8;
    else if (strcmp(typeName, "sint8") == 0)
        type = CIMTYPE_SINT8;
    else if (strcmp(typeName, "uint16") == 0)
        type = CIMTYPE_UINT16;
    else if (strcmp(typeName, "sint16") == 0)
        type = CIMTYPE_SINT16;
    else if (strcmp(typeName, "uint32") == 0)
        type = CIMTYPE_UINT32;
    else if (strcmp(typeName, "sint32") == 0)
        type = CIMTYPE_SINT32;
    else if (strcmp(typeName, "uint64") == 0)
        type = CIMTYPE_UINT64;
    else if (strcmp(typeName, "sint64") == 0)
        type = CIMTYPE_SINT64;
    else if (strcmp(typeName, "datetime") == 0)
        type = CIMTYPE_DATETIME;
    else if (strcmp(typeName, "real32") == 0)
        type = CIMTYPE_REAL32;
    else if (strcmp(typeName, "real64") == 0)
        type = CIMTYPE_REAL64;
    else if (strcmp(typeName, "reference") == 0)
        type = CIMTYPE_REFERENCE;
//  PEP 194:
//  Note that "object" (ie. CIMTYPE_OBJECT) is not a real CIM datatype, just a
//  Pegasus internal representation of an embedded object, so it won't be
//  found here.
    else unrecognizedType = true;

    if (unrecognizedType ||
        ((type == CIMTYPE_REFERENCE) &&
         (strcmp(attributeName, "PARAMTYPE") != 0)))
    {
        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.%s", tagName, attributeName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE_FOR_ATTRIBUTE",
            "Illegal value for $0 attribute",
            buffer);

        throw XmlSemanticError(lineNumber, mlParms);
    }

    cimType = type;
    return true;
}

//------------------------------------------------------------------------------
//
// getCimBooleanAttribute()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getCimBooleanAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* tagName,
    const char* attributeName,
    Boolean defaultValue,
    Boolean required)
{
    const char* tmp;

    if (!entry.getAttributeValue(attributeName, tmp))
    {
        if (!required)
            return defaultValue;

        char buffer[MESSAGE_SIZE];
        sprintf(buffer, "%s.%s", attributeName, tagName);

        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_REQUIRED_ATTRIBUTE",
            "missing required $0 attribute",
            buffer);
        throw XmlValidationError(lineNumber, mlParms);
    }

    if (strcmp(tmp, "true") == 0)
        return true;
    else if (strcmp(tmp, "false") == 0)
        return false;

    char buffer[MESSAGE_SIZE];
    sprintf(buffer, "%s.%s", attributeName, tagName);

    MessageLoaderParms mlParms(
        "Common.XmlReader.INVALID_ATTRIBUTE",
        "Invalid $0 attribute value",
        buffer);

    throw XmlSemanticError(lineNumber, mlParms);

    return false;
}

// See http://www.ietf.org/rfc/rfc2396.txt section 2
//
// Also see the "CIM Operations over HTTP" spec, section 3.3.2 and
// 3.3.3, for the treatment of non US-ASCII chars (UTF-8)
String XmlReader::decodeURICharacters(String uriString)
{
    Uint32 i;

    Buffer utf8Chars;

    for (i=0; i<uriString.size(); i++)
    {
        if (uriString[i] == '%')
        {
            if (i+2 >= uriString.size())
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_URI_ENCODING",
                    "Invalid URI encoding");
                throw ParseError(MessageLoader::getMessage(mlParms));
            }

            Uint8 digit1 =
                StringConversion::hexCharToNumeric(char(uriString[++i]));
            Uint8 digit2 =
                StringConversion::hexCharToNumeric(char(uriString[++i]));
            if ( (digit1 > 15) || (digit2 > 15) )
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_URI_ENCODING",
                    "Invalid URI encoding");
                throw ParseError(MessageLoader::getMessage(mlParms));
            }

            Uint16 decodedChar = Uint16(digit1<<4) + Uint16(digit2);
            utf8Chars.append((char)decodedChar);
        }
        else
        {
            utf8Chars.append((char)uriString[i]);
        }
    }

    // If there was a string to decode...
    if (uriString.size() > 0)
    {
        // Convert UTF-8 to UTF-16 and return the String
        return String(utf8Chars.getData(), utf8Chars.size());
    }
    else
    {
        return String();
    }
}

//------------------------------------------------------------------------------
//
// stringToValue()
//
// Return: CIMValue. If the string input is zero length creates a CIMValue
//         with value defined by the type.  Else the value is inserted.
//
//         Note that this does not set the CIMValue Null if the string is empty.
//
//------------------------------------------------------------------------------

CIMValue XmlReader::stringToValue(
    Uint32 lineNumber,
    const char* valueString,
    CIMType type)
{
    return stringToValue(lineNumber, valueString, strlen(valueString), type);
}

CIMValue XmlReader::stringToValue(
    Uint32 lineNumber,
    const char* valueString,
    Uint32 valueStringLen,
    CIMType type)
{
    // ATTN-B: accepting only UTF-8 for now! (affects string and char16):

    // Char string must have been null terminated.
    PEGASUS_ASSERT (!valueString[valueStringLen]);

    // Create value per type
    switch (type)
    {
        case CIMTYPE_BOOLEAN:
        {
            if (System::strcasecmp(valueString, "TRUE") == 0)
                return CIMValue(true);
            else if (System::strcasecmp(valueString, "FALSE") == 0)
                return CIMValue(false);
            else
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_BOOLEAN_VALUE",
                    "Invalid boolean value");
                throw XmlSemanticError(lineNumber, mlParms);
            }
        }

        case CIMTYPE_STRING:
        {
            return CIMValue(String(valueString, valueStringLen));
        }

        case CIMTYPE_CHAR16:
        {
            // Converts UTF-8 to UTF-16
            String tmp(valueString, valueStringLen);
            if (tmp.size() != 1)
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_CHAR16_VALUE",
                    "Invalid char16 value");
                throw XmlSemanticError(lineNumber, mlParms);
            }

            return CIMValue(tmp[0]);
        }

        case CIMTYPE_UINT8:
        case CIMTYPE_UINT16:
        case CIMTYPE_UINT32:
        case CIMTYPE_UINT64:
        {
            Uint64 x;

            if (!StringConversion::stringToUnsignedInteger(valueString, x))
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_UI_VALUE",
                    "Invalid unsigned integer value");
                throw XmlSemanticError(lineNumber, mlParms);
            }

            switch (type)
            {
                case CIMTYPE_UINT8:
                {
                    if (!StringConversion::checkUintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.U8_VALUE_OUT_OF_RANGE",
                            "Uint8 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Uint8(x));
                }
                case CIMTYPE_UINT16:
                {
                    if (!StringConversion::checkUintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.U16_VALUE_OUT_OF_RANGE",
                            "Uint16 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Uint16(x));
                }
                case CIMTYPE_UINT32:
                {
                    if (!StringConversion::checkUintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.U32_VALUE_OUT_OF_RANGE",
                            "Uint32 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Uint32(x));
                }
                case CIMTYPE_UINT64: return CIMValue(Uint64(x));
                default: break;
            }
        }

        case CIMTYPE_SINT8:
        case CIMTYPE_SINT16:
        case CIMTYPE_SINT32:
        case CIMTYPE_SINT64:
        {
            Sint64 x;

            if (!StringConversion::stringToSignedInteger(valueString, x))
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_SI_VALUE",
                    "Invalid signed integer value");
                throw XmlSemanticError(lineNumber, mlParms);
            }

            switch (type)
            {
                case CIMTYPE_SINT8:
                {
                    if (!StringConversion::checkSintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.S8_VALUE_OUT_OF_RANGE",
                            "Sint8 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Sint8(x));
                }
                case CIMTYPE_SINT16:
                {
                    if (!StringConversion::checkSintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.S16_VALUE_OUT_OF_RANGE",
                            "Sint16 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Sint16(x));
                }
                case CIMTYPE_SINT32:
                {
                    if (!StringConversion::checkSintBounds(x, type))
                    {
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.S32_VALUE_OUT_OF_RANGE",
                            "Sint32 value out of range");
                        throw XmlSemanticError(lineNumber, mlParms);
                    }
                    return CIMValue(Sint32(x));
                }
                case CIMTYPE_SINT64: return CIMValue(Sint64(x));
                default: break;
            }
        }

        case CIMTYPE_DATETIME:
        {
            CIMDateTime tmp;

            try
            {
                if (valueStringLen != 0)
                {
                    tmp.set(valueString);
                }
            }
            catch (InvalidDateTimeFormatException&)
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_DATETIME_VALUE",
                    "Invalid datetime value");
                throw XmlSemanticError(lineNumber, mlParms);
            }

            return CIMValue(tmp);
        }

        case CIMTYPE_REAL32:
        {
            Real64 x;

            if (!StringConversion::stringToReal64(valueString, x))
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_RN_VALUE",
                    "Invalid real number value");
                throw XmlSemanticError(lineNumber, mlParms);
            }
            return CIMValue(Real32(x));
        }

        case CIMTYPE_REAL64:
        {
            Real64 x;

            if (!StringConversion::stringToReal64(valueString, x))
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_RN_VALUE",
                    "Invalid real number value");
                throw XmlSemanticError(lineNumber, mlParms);
            }
            return CIMValue(x);
        }

//  PEP 194:
//  Note that "object" (ie. CIMTYPE_OBJECT) is not a real CIM datatype, but
//  just a Pegasus internal representation of an embedded object. However,
//  this case is used when decoding string representations of embedded objects.
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
        {
            CIMObject x;

            if (*valueString == '\0')
            {
                return CIMValue(type, false, 0);
            }
            else
            {
                // Convert the non-NULL string to a CIMObject (containing
                // either a CIMInstance or a CIMClass).

                // First we need to create a new "temporary" XmlParser that is
                // just the value of the Embedded Object in String
                // representation.
                AutoArrayPtr<char> tmp_buffer(
                    new char[valueStringLen + 1]);
                memcpy(tmp_buffer.get(), valueString, valueStringLen + 1);
                XmlParser tmp_parser(tmp_buffer.get());

                // The next bit of logic constructs a CIMObject from the
                // Embedded Object String.  It is similar to the method
                // XmlReader::getValueObjectElement().
                CIMInstance cimInstance;
                CIMClass cimClass;

                if (XmlReader::getInstanceElement(tmp_parser, cimInstance))
                {
                    if (type == CIMTYPE_INSTANCE)
                        return CIMValue(cimInstance);
                    x = CIMObject(cimInstance);
                }
                else if (XmlReader::getClassElement(tmp_parser, cimClass))
                {
                    x = CIMObject(cimClass);
                }
                else
                {
                    if (type == CIMTYPE_OBJECT)
                    {
                        // change "element" to "embedded object"
                        MessageLoaderParms mlParms(
                            "Common.XmlReader."
                                "EXPECTED_INSTANCE_OR_CLASS_ELEMENT",
                            "Expected INSTANCE or CLASS element");
                        throw XmlValidationError(lineNumber, mlParms);
                    }
                    else
                    {
                        // change "element" to "embedded object"
                        MessageLoaderParms mlParms(
                            "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
                            "Expected INSTANCE element");
                        throw XmlValidationError(lineNumber, mlParms);
                    }
                }
                // Ok, now we can delete the storage for the temporary
                // XmlParser.
                tmp_buffer.reset();
            }
            return CIMValue(x);
        }

        default:
            break;
    }

    MessageLoaderParms mlParms(
        "Common.XmlReader.MALFORMED_XML",
        "malformed XML");
    throw XmlSemanticError(lineNumber, mlParms);

    return false;
}

//------------------------------------------------------------------------------
//
// skipElement()
//
//------------------------------------------------------------------------------
void XmlReader::skipElement(
    XmlParser& parser,
    XmlEntry& entry)
{
    const char * tag_name = entry.text;

    if (entry.type == XmlEntry::EMPTY_TAG)
    {
        return;
    }

    while (testStartTagOrEmptyTag(parser, entry))
    {
        skipElement(parser, entry);
    }

    if (testContentOrCData(parser, entry))
    {
        ; // skip
    }

    expectEndTag(parser, tag_name);
    return;
}

//------------------------------------------------------------------------------
//
// getValueElement()
//
//     <!ELEMENT VALUE (#PCDATA)>
//
// Return: false if no value element.
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueElement(
    XmlParser& parser,
    CIMType type,
    CIMValue& value)
{
    // Get VALUE start tag: Return false if no VALUE start Tag

    XmlEntry entry;
    if (!testStartTagOrEmptyTag(parser, entry, "VALUE"))
    {
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    const char* valueString = "";
    Uint32 valueStringLen = 0;
    if (!empty)
    {
        if (testContentOrCData(parser, entry))
        {
            valueString = entry.text;
            valueStringLen = entry.textLen;
        }

        expectEndTag(parser, "VALUE");
    }
    value = stringToValue(parser.getLine(),valueString,valueStringLen,type);

    return true;
}

//------------------------------------------------------------------------------
//
// getStringValueElement()
//
//     <!ELEMENT VALUE (#PCDATA)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getStringValueElement(
    XmlParser& parser,
    String& str,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "VALUE"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "Expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    const char* valueString = "";
    Uint32 valueStringLen = 0;

    if (!empty)
    {
        if (testContentOrCData(parser, entry))
        {
            valueString = entry.text;
            valueStringLen = entry.textLen;
        }

        expectEndTag(parser, "VALUE");
    }

    str = String(valueString, valueStringLen);
    return true;
}

// EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// getUint32ArgValueElement()
//
//     <!ELEMENT VALUE (#PCDATA)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getUint32ArgValueElement(
    XmlParser& parser,
    Uint32Arg& val,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "VALUE"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "Expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    const char* valueString = "";

    if (!empty)
    {
        if (testContentOrCData(parser, entry))
            valueString = entry.text;

        expectEndTag(parser, "VALUE");
    }
    else
    {
        // create the arg object with the NULL value in it.
        val = Uint32Arg();
        return true;
    }

    //convert to Uint32. Note error if overflow.

    Uint64 x;
    if (!StringConversion::stringToUnsignedInteger(valueString, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_UI_VALUE",
            "Invalid unsigned integer value");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }

    if (!StringConversion::checkUintBounds(x, CIMTYPE_UINT32))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.U32_VALUE_OUT_OF_RANGE",
            "Uint32 value out of range");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }
    // create the arg object with the value in it.
    val = Uint32Arg(x);

    return true;
}
//------------------------------------------------------------------------------
//
// getUint64ValueElement()
//
//     <!ELEMENT VALUE (#PCDATA)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getUint64ValueElement(
    XmlParser& parser,
    Uint64Arg& val,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "VALUE"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "Expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    const char* valueString = "";

    if (!empty)
    {
        if (testContentOrCData(parser, entry))
            valueString = entry.text;

        expectEndTag(parser, "VALUE");
    }
    else
    {
        // create the arg object with the NULL value in it.
        val = Uint64Arg();
        return true;
    }

    Uint64 x;

    if (!StringConversion::stringToUnsignedInteger(valueString, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_UI_VALUE",
            "The unsigned integer value is not valid.");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }

    // create the arg object with the value in it.
    val = Uint64Arg(x);

    return true;
}


//------------------------------------------------------------------------------
//
// getUint32ValueElement()
//
//     <!ELEMENT VALUE (#PCDATA)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getUint32ValueElement(
    XmlParser& parser,
    Uint32& val,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "VALUE"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "Expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    const char* valueString = "";

    if (!empty)
    {
        if (testContentOrCData(parser, entry))
            valueString = entry.text;

        expectEndTag(parser, "VALUE");
    }
    else
    {
        // leave the existing value
        return true;
    }

    //convert to Uint32. Note error if overflow.

    Uint64 x;
    if (!StringConversion::stringToUnsignedInteger(valueString, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_UI_VALUE",
            "Invalid unsigned integer value");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }

    if (!StringConversion::checkUintBounds(x, CIMTYPE_UINT32))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.U32_VALUE_OUT_OF_RANGE",
            "Uint32 value out of range");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }
    // create the arg object with the value in it.
    val = x;

    return true;
}
//EXP_PULL_END

//----------------------------------------------------------------------------
//
// getPropertyValue
//     Use: Decode property value from SetProperty request and
//     GetProperty response.
//
//     PropertyValue is one of:
//
//
//      <!ELEMENT VALUE.ARRAY (VALUE*)>
//
//      <!ELEMENT VALUE.REFERENCE (CLASSPATH|LOCALCLASSPATH|CLASSNAME|
//         <!ELEMENT VALUE.ARRAY (VALUE*)>
//
//         <!ELEMENT VALUE.REFERENCE (CLASSPATH|LOCALCLASSPATH|CLASSNAME|
//                           INSTANCEPATH|LOCALINSTANCEPATH|INSTANCENAME)>
//
//         <!ELEMENT VALUE.REFARRAY (VALUE.REFERENCE*)>
//
//----------------------------------------------------------------------------
Boolean XmlReader::getPropertyValue(
    XmlParser& parser,
    CIMValue& cimValue)
{
    // Can not test for value type, so assume String
    const CIMType type = CIMTYPE_STRING;

    // Test for VALUE element
    if (XmlReader::getValueElement(parser, type, cimValue))
    {
        return true;
    }

    // Test for VALUE.ARRAY element
    if (XmlReader::getValueArrayElement(parser, type, cimValue))
    {
       return true;
    }

    // Test for VALUE.REFERENCE element
    CIMObjectPath reference;
    if (XmlReader::getValueReferenceElement(parser, reference))
    {
        cimValue.set(reference);
        return true;
    }

    // Test for VALUE.REFARRAY element
    if (XmlReader::getValueReferenceArrayElement(parser, cimValue))
    {
       return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
// stringArrayToValue()
//
//------------------------------------------------------------------------------

template<class T>
CIMValue StringArrayToValueAux(
    Uint32 lineNumber,
    const Array<CharString>& stringArray,
    CIMType type,
    T*)
{
    Array<T> array;

    for (Uint32 i = 0, n = stringArray.size(); i < n; i++)
    {
        CIMValue value = XmlReader::stringToValue(
            lineNumber,
            stringArray[i].value,
            stringArray[i].length,
            type);

        T x;
        value.get(x);
        array.append(x);
    }

    return CIMValue(array);
}

CIMValue XmlReader::stringArrayToValue(
    Uint32 lineNumber,
    const Array<const char*> &array,
    CIMType type)
{
    Array<CharString> strArray;

    for (Uint32 i = 0, n = array.size() ; i < n ; ++i)
    {
        strArray.append(CharString(array[i], strlen(array[i])));
    }

    return _stringArrayToValue(lineNumber, strArray, type);
}

CIMValue XmlReader::_stringArrayToValue(
    Uint32 lineNumber,
    const Array<CharString> &array,
    CIMType type)
{
    switch (type)
    {
        case CIMTYPE_BOOLEAN:
            return StringArrayToValueAux(lineNumber, array, type, (Boolean*)0);

        case CIMTYPE_STRING:
            return StringArrayToValueAux(lineNumber, array, type, (String*)0);

        case CIMTYPE_CHAR16:
            return StringArrayToValueAux(lineNumber, array, type, (Char16*)0);

        case CIMTYPE_UINT8:
            return StringArrayToValueAux(lineNumber, array, type, (Uint8*)0);

        case CIMTYPE_UINT16:
            return StringArrayToValueAux(lineNumber, array, type, (Uint16*)0);

        case CIMTYPE_UINT32:
            return StringArrayToValueAux(lineNumber, array, type, (Uint32*)0);

        case CIMTYPE_UINT64:
            return StringArrayToValueAux(lineNumber, array, type, (Uint64*)0);

        case CIMTYPE_SINT8:
            return StringArrayToValueAux(lineNumber, array, type, (Sint8*)0);

        case CIMTYPE_SINT16:
            return StringArrayToValueAux(lineNumber, array, type, (Sint16*)0);

        case CIMTYPE_SINT32:
            return StringArrayToValueAux(lineNumber, array, type, (Sint32*)0);

        case CIMTYPE_SINT64:
            return StringArrayToValueAux(lineNumber, array, type, (Sint64*)0);

        case CIMTYPE_DATETIME:
            return StringArrayToValueAux(
                lineNumber, array, type, (CIMDateTime*)0);

        case CIMTYPE_REAL32:
            return StringArrayToValueAux(lineNumber, array, type, (Real32*)0);

        case CIMTYPE_REAL64:
            return StringArrayToValueAux(lineNumber, array, type, (Real64*)0);

//  PEP 194:
//  Note that "object" (ie. CIMTYPE_OBJECT) is not a real CIM datatype, but
//  just a Pegasus internal representation of an embedded object. However,
//  this case is used when decoding string representations of embedded objects.
        case CIMTYPE_OBJECT:
            return StringArrayToValueAux(
                lineNumber, array, type, (CIMObject*)0);
        case CIMTYPE_INSTANCE:
            return StringArrayToValueAux(
                lineNumber, array, type, (CIMInstance*)0);
        default:
            break;
    }

    // Unreachable:
    return CIMValue();
}

//------------------------------------------------------------------------------
//
// getValueArrayElement()
//
//     <!ELEMENT VALUE.ARRAY (VALUE*)>
//
//  Return: Boolean. Returns false if there is no VALUE.ARRAY start element
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueArrayElement(
    XmlParser& parser,
    CIMType type,
    CIMValue& value)
{
    // Clears any values from the Array. Assumes this is array CIMValue
    value.clear();

    // Get VALUE.ARRAY open tag:

    XmlEntry entry;
    Array<CharString> stringArray;

    // If no VALUE.ARRAY start tag, return false
    if (!testStartTagOrEmptyTag(parser, entry, "VALUE.ARRAY"))
        return false;

    if (entry.type != XmlEntry::EMPTY_TAG)
    {
        // For each VALUE element:

        while (testStartTagOrEmptyTag(parser, entry, "VALUE"))
        {
            // ATTN: NULL values in array will have VALUE.NULL subelement
            // See DSP0201 Version 2.3.0, Section 5.2.3.2
            if (entry.type == XmlEntry::EMPTY_TAG)
            {
                stringArray.append(CharString("", 0));
                continue;
            }

            if (testContentOrCData(parser, entry))
            {
                stringArray.append(CharString(entry.text, entry.textLen));
            }
            else
            {
                stringArray.append(CharString("", 0));
            }
            expectEndTag(parser, "VALUE");
        }

        expectEndTag(parser, "VALUE.ARRAY");
    }

    value = _stringArrayToValue(parser.getLine(), stringArray, type);
    return true;
}

//------------------------------------------------------------------------------
//
// getFlavor()
//
//     <!ENTITY % QualifierFlavor
//         "OVERRIDABLE (true|false) 'true'
//         TOSUBCLASS (true|false) 'true'
//         TOINSTANCE (true|false)  'false'
//         TRANSLATABLE (true|false)  'false'">
//
//------------------------------------------------------------------------------

CIMFlavor XmlReader::getFlavor(
    XmlEntry& entry,
    Uint32 lineNumber,
    const char* tagName)
{
    // Get QUALIFIER.OVERRIDABLE

    Boolean overridable = getCimBooleanAttribute(
        lineNumber, entry, tagName, "OVERRIDABLE", true, false);

    // Get QUALIFIER.TOSUBCLASS

    Boolean toSubClass = getCimBooleanAttribute(
        lineNumber, entry, tagName, "TOSUBCLASS", true, false);

    // Get QUALIFIER.TOINSTANCE

    Boolean toInstance = getCimBooleanAttribute(
        lineNumber, entry, tagName, "TOINSTANCE", false, false);

    // Get QUALIFIER.TRANSLATABLE

    Boolean translatable = getCimBooleanAttribute(
        lineNumber, entry, tagName, "TRANSLATABLE", false, false);

    // Start with CIMFlavor::NONE.  Defaults are specified in the
    // getCimBooleanAttribute() calls above.
    CIMFlavor flavor = CIMFlavor (CIMFlavor::NONE);

    if (overridable)
        flavor.addFlavor (CIMFlavor::OVERRIDABLE);
    else
        flavor.addFlavor (CIMFlavor::DISABLEOVERRIDE);

    if (toSubClass)
        flavor.addFlavor (CIMFlavor::TOSUBCLASS);
    else
        flavor.addFlavor (CIMFlavor::RESTRICTED);

    if (toInstance)
        flavor.addFlavor (CIMFlavor::TOINSTANCE);

    if (translatable)
        flavor.addFlavor (CIMFlavor::TRANSLATABLE);

    return flavor;
}

//------------------------------------------------------------------------------
//
// getOptionalScope()
//
//     DTD:
//         <!ELEMENT SCOPE EMPTY>
//         <!ATTLIST SCOPE
//              CLASS (true|false) 'false'
//              ASSOCIATION (true|false) 'false'
//              REFERENCE (true|false) 'false'
//              PROPERTY (true|false) 'false'
//              METHOD (true|false) 'false'
//              PARAMETER (true|false) 'false'
//              INDICATION (true|false) 'false'>
//
//------------------------------------------------------------------------------

CIMScope XmlReader::getOptionalScope(XmlParser& parser)
{
    XmlEntry entry;
    CIMScope scope;

    if (!parser.next(entry))
        return scope;    // No SCOPE element found; return the empty scope

    Boolean isEmptyTag = entry.type == XmlEntry::EMPTY_TAG;

    if ((!isEmptyTag &&
        entry.type != XmlEntry::START_TAG) ||
        strcmp(entry.text, "SCOPE") != 0)
    {
        // No SCOPE element found; return the empty scope
        parser.putBack(entry);
        return scope;
    }

    Uint32 line = parser.getLine();

    if (getCimBooleanAttribute(line, entry, "SCOPE", "CLASS", false, false))
        scope.addScope (CIMScope::CLASS);

    if (getCimBooleanAttribute(
        line, entry, "SCOPE", "ASSOCIATION", false, false))
        scope.addScope (CIMScope::ASSOCIATION);

    if (getCimBooleanAttribute(
        line, entry, "SCOPE", "REFERENCE", false, false))
        scope.addScope (CIMScope::REFERENCE);

    if (getCimBooleanAttribute(line, entry, "SCOPE", "PROPERTY", false, false))
        scope.addScope (CIMScope::PROPERTY);

    if (getCimBooleanAttribute(line, entry, "SCOPE", "METHOD", false, false))
        scope.addScope (CIMScope::METHOD);

    if (getCimBooleanAttribute(line, entry, "SCOPE", "PARAMETER", false, false))
        scope.addScope (CIMScope::PARAMETER);

    if (getCimBooleanAttribute(line, entry, "SCOPE", "INDICATION",false, false))
        scope.addScope (CIMScope::INDICATION);

    if (!isEmptyTag)
        expectEndTag(parser, "SCOPE");

    return scope;
}

//------------------------------------------------------------------------------
//
// getQualifierElement()
//
//     <!ELEMENT QUALIFIER (VALUE|VALUE.ARRAY)?>
//     <!ATTLIST QUALIFIER
//         %CIMName;
//         %CIMType; #REQUIRED
//         %Propagated;
//         %QualifierFlavor;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getQualifierElement(
    XmlParser& parser,
    CIMQualifier& qualifier)
{
    // Get QUALIFIER element:

    XmlEntry entry;
    if (!testStartTagOrEmptyTag(parser, entry, "QUALIFIER"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get QUALIFIER.NAME attribute:

    CIMName name = getCimNameAttribute(parser.getLine(), entry, "QUALIFIER");

    // Get QUALIFIER.TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "QUALIFIER");

    // Get QUALIFIER.PROPAGATED

    Boolean propagated = getCimBooleanAttribute(
        parser.getLine(), entry, "QUALIFIER", "PROPAGATED", false, false);

    // Get flavor oriented attributes:

    CIMFlavor flavor = getFlavor(entry, parser.getLine(), "QUALIFIER");

    // Get VALUE or VALUE.ARRAY element:

    CIMValue value;

    if (empty)
    {
        value.setNullValue(type, false);
    }
    else
    {
        if (!getValueElement(parser, type, value) &&
            !getValueArrayElement(parser, type, value))
        {
            value.setNullValue(type, false);
        }

        // Expect </QUALIFIER>:

        expectEndTag(parser, "QUALIFIER");
    }

    // Build qualifier:

    qualifier = CIMQualifier(name, value, flavor, propagated);
    return true;
}

//------------------------------------------------------------------------------
//
// getQualifierElements()
//
//------------------------------------------------------------------------------

template<class CONTAINER>
void getQualifierElements(XmlParser& parser, CONTAINER& container)
{
    CIMQualifier qualifier;

    while (XmlReader::getQualifierElement(parser, qualifier))
    {
        try
        {
            container.addQualifier(qualifier);
        }
        catch (AlreadyExistsException&)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.DUPLICATE_QUALIFIER",
                "duplicate qualifier");
            throw XmlSemanticError(parser.getLine(), mlParms);
        }
    }
}

//------------------------------------------------------------------------------
//
// getPropertyElement()
//
//     <!ELEMENT PROPERTY (QUALIFIER*,VALUE?)>
//     <!ATTLIST PROPERTY
//         %CIMName;
//         %ClassOrigin;
//         %Propagated;
//         %EmbeddedObject; #IMPLIED
//         %CIMType; #REQUIRED>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getPropertyElement(XmlParser& parser, CIMProperty& property)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PROPERTY"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PROPERTY.NAME attribute:

    CIMName name = getCimNameAttribute(parser.getLine(), entry, "PROPERTY");

    // Get PROPERTY.CLASSORIGIN attribute:

    CIMName classOrigin =
        getClassOriginAttribute(parser.getLine(), entry, "PROPERTY");

    // Get PROPERTY.PROPAGATED

    Boolean propagated = getCimBooleanAttribute(
        parser.getLine(), entry, "PROPERTY", "PROPAGATED", false, false);

    // Get PROPERTY.EmbeddedObject attribute:

    EmbeddedObjectAttributeType embeddedObject =
        getEmbeddedObjectAttribute(parser.getLine(), entry, "PROPERTY");

    // Get PROPERTY.TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "PROPERTY");

    // Create property: Sets type and !isarray

    CIMValue value(type, false);
    property = CIMProperty(name, value, 0, CIMName(), classOrigin, propagated);

    if (!empty)
    {
        // Get qualifiers. We need to do this before checking for the
        // property as an embedded object, because we need to also check
        // for the EmbeddedObject qualifier.
        getQualifierElements(parser, property);
    }

    Boolean embeddedObjectQualifierValue = false;
    Uint32 ix = property.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT);
    if (ix != PEG_NOT_FOUND)
    {
        property.getQualifier(ix).getValue().get(embeddedObjectQualifierValue);
    }
    String embeddedInstanceQualifierValue;
    ix = property.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);
    if (ix != PEG_NOT_FOUND)
    {
        property.getQualifier(ix).getValue().get(
            embeddedInstanceQualifierValue);
    }
    // If the EmbeddedObject attribute is present with value "object"
    // or the EmbeddedObject qualifier exists on this property with value "true"
    // then convert the EmbeddedObject-encoded string into a CIMObject
    Boolean isEmbeddedObject = (embeddedObject == EMBEDDED_OBJECT_ATTR) ||
        embeddedObjectQualifierValue;
    Boolean isEmbeddedInstance = (embeddedObject == EMBEDDED_INSTANCE_ATTR) ||
        embeddedInstanceQualifierValue.size() > 0;
    if (isEmbeddedObject || isEmbeddedInstance)
    {
        // The EmbeddedObject attribute is only valid on Properties of type
        // string
        if (type == CIMTYPE_STRING)
        {
            if (isEmbeddedObject)
                type = CIMTYPE_OBJECT;
            else
                type = CIMTYPE_INSTANCE;
            CIMValue new_value(type, false);
            CIMProperty new_property = CIMProperty(
                name, new_value, 0, CIMName(), classOrigin, propagated);

            // Copy the qualifiers from the String property to the CIMObject
            // property.
            for (Uint32 i = 0; i < property.getQualifierCount(); ++i)
            {
                // All properties are copied, including the EmbeddedObject
                // qualifier.  This way we don't have to keep track to know
                // that the EmbeddedObject qualifier needs to be added back
                // during the encode step.
                new_property.addQualifier(property.getQualifier(i));
            }

            value = new_value;
            property = new_property;
        }
        else
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.INVALID_EMBEDDEDOBJECT_TYPE",
                "The EmbeddedObject attribute is only valid on string types.");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
    }
    // Continue on to get property value, if not empty.
    if (!empty)
    {
        if (getValueElement(parser, type, value))
            property.setValue(value);
        expectEndTag(parser, "PROPERTY");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getArraySizeAttribute()
//
//     Returns true if able to get array-size. Note that array size will
//     always be a positive integer.
//
//     <!ENTITY % ArraySize "ARRAYSIZE CDATA #IMPLIED">
//
//------------------------------------------------------------------------------

Boolean XmlReader::getArraySizeAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* tagName,
    Uint32& value)
{
    const char* tmp;

    if (!entry.getAttributeValue("ARRAYSIZE", tmp))
        return false;

    Uint64 arraySize;

    if (!StringConversion::stringToUnsignedInteger(tmp, arraySize) ||
        (arraySize == 0) ||
        !StringConversion::checkUintBounds(arraySize, CIMTYPE_UINT32))
    {
        char message[128];
        sprintf(message, "%s.%s", tagName, "ARRAYSIZE");

        MessageLoaderParms mlParms(
            "Common.XmlReader.ILLEGAL_VALUE",
            "Illegal value for $0",
            message);
        throw XmlSemanticError(lineNumber, mlParms);
    }

    value = Uint32(arraySize);
    return true;
}

//------------------------------------------------------------------------------
//
// getPropertyArrayElement()
//
//     <!ELEMENT PROPERTY.ARRAY (QUALIFIER*,VALUE.ARRAY?)>
//     <!ATTLIST PROPERTY.ARRAY
//             %CIMName;
//             %CIMType; #REQUIRED
//             %ArraySize;
//             %ClassOrigin;
//             %Propagated;
//             %EmbeddedObject; #IMPLIED>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getPropertyArrayElement(
    XmlParser& parser,
    CIMProperty& property)
{
    // Get PROPERTY element:

    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PROPERTY.ARRAY"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PROPERTY.NAME attribute:

    CIMName name =
        getCimNameAttribute(parser.getLine(), entry, "PROPERTY.ARRAY");

    // Get PROPERTY.TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "PROPERTY.ARRAY");

    // Get PROPERTY.ARRAYSIZE attribute:

    Uint32 arraySize = 0;
    getArraySizeAttribute(parser.getLine(), entry, "PROPERTY.ARRAY", arraySize);

    // Get PROPERTY.CLASSORIGIN attribute:

    CIMName classOrigin =
        getClassOriginAttribute(parser.getLine(), entry, "PROPERTY.ARRAY");

    // Get PROPERTY.ARRAY.PROPAGATED

    Boolean propagated = getCimBooleanAttribute(
        parser.getLine(),
        entry,
        "PROPERTY.ARRAY",
        "PROPAGATED",
        false,
        false);

    // Get PROPERTY.EmbeddedObject attribute:

    EmbeddedObjectAttributeType embeddedObject = getEmbeddedObjectAttribute(
        parser.getLine(), entry, "PROPERTY.ARRAY");

    // Create property:

    CIMValue value(type, true, arraySize);
    property = CIMProperty(
        name, value, arraySize, CIMName(), classOrigin, propagated);

    if (!empty)
    {
        // Get qualifiers:
        getQualifierElements(parser, property);
    }

    Boolean embeddedObjectQualifierValue = false;
    Uint32 ix = property.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT);
    if (ix != PEG_NOT_FOUND)
    {
        property.getQualifier(ix).getValue().get(embeddedObjectQualifierValue);
    }
    String embeddedInstanceQualifierValue;
    ix = property.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);
    if (ix != PEG_NOT_FOUND)
    {
        property.getQualifier(ix).getValue().get(
            embeddedInstanceQualifierValue);
    }
    // If the EmbeddedObject attribute is present with value "object"
    // or the EmbeddedObject qualifier exists on this property with value "true"
    // then
    //     Convert the EmbeddedObject-encoded string into a CIMObject
    Boolean isEmbeddedObject = (embeddedObject == EMBEDDED_OBJECT_ATTR) ||
        embeddedObjectQualifierValue;
    Boolean isEmbeddedInstance = (embeddedObject == EMBEDDED_INSTANCE_ATTR) ||
        embeddedInstanceQualifierValue.size() > 0;
    if (isEmbeddedObject || isEmbeddedInstance)
    {
        // The EmbeddedObject attribute is only valid on Properties of type
        // string
        if (type == CIMTYPE_STRING)
        {
            if (isEmbeddedObject)
                type = CIMTYPE_OBJECT;
            else
                type = CIMTYPE_INSTANCE;
            CIMValue new_value(type, true, arraySize);
            CIMProperty new_property = CIMProperty(
                name, new_value, arraySize, CIMName(), classOrigin, propagated);

            // Copy the qualifiers from the String property to the CIMObject
            // property.
            for (Uint32 i = 0; i < property.getQualifierCount(); ++i)
            {
                // All properties are copied, including the EmbeddedObject
                // qualifier.  This way we don't have to keep track to know
                // that the EmbeddedObject qualifier needs to be added back
                // during the encode step.
                new_property.addQualifier(property.getQualifier(i));
            }

            value = new_value;
            property = new_property;
        }
        else
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.INVALID_EMBEDDEDOBJECT_TYPE",
                "The EmbeddedObject attribute is only valid on string types.");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
    }
    // Continue on to get property array value, if not empty.
    // Else not an embedded object, if not empty, get the property array value.
    if (!empty)
    {
        if (getValueArrayElement(parser, type, value))
        {
            if (arraySize && arraySize != value.getArraySize())
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.ARRAY_SIZE_DIFFERENT",
                    "ARRAYSIZE attribute and value-array size are different");
                throw XmlSemanticError(parser.getLine(), mlParms);
            }

            property.setValue(value);
        }
        expectEndTag(parser, "PROPERTY.ARRAY");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getHostElement()
//
//     <!ELEMENT HOST (#PCDATA)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getHostElement(
    XmlParser& parser,
    String& host)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "HOST"))
        return false;

    if (!parser.next(entry) || entry.type != XmlEntry::CONTENT)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CONTENT_ELEMENT",
            "expected content of HOST element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    host = String(entry.text);
    expectEndTag(parser, "HOST");
    return true;
}

//------------------------------------------------------------------------------
//
// getNameSpaceElement()
//
//     <!ELEMENT NAMESPACE EMPTY>
//     <!ATTLIST NAMESPACE %CIMName;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getNameSpaceElement(
    XmlParser& parser,
    CIMName& nameSpaceComponent)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "NAMESPACE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    nameSpaceComponent = getCimNameAttribute(
        parser.getLine(), entry, "NAMESPACE");

    if (!empty)
        expectEndTag(parser, "NAMESPACE");

    return true;
}

//------------------------------------------------------------------------------
//
// getLocalNameSpacePathElement()
//
//     <!ELEMENT LOCALNAMESPACEPATH (NAMESPACE+)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getLocalNameSpacePathElement(
    XmlParser& parser,
    String& nameSpace)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "LOCALNAMESPACEPATH"))
        return false;

    CIMName nameSpaceComponent;

    while (getNameSpaceElement(parser, nameSpaceComponent))
    {
        if (nameSpace.size())
            nameSpace.append('/');

        nameSpace.append(nameSpaceComponent.getString());
    }

    if (!nameSpace.size())
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_NAMESPACE_ELEMENTS",
            "Expected one or more NAMESPACE elements within "
                "LOCALNAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    expectEndTag(parser, "LOCALNAMESPACEPATH");
    return true;
}

//------------------------------------------------------------------------------
//
// getNameSpacePathElement()
//
//     <!ELEMENT NAMESPACEPATH (HOST,LOCALNAMESPACEPATH)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getNameSpacePathElement(
    XmlParser& parser,
    String& host,
    String& nameSpace)
{
    host.clear();
    nameSpace.clear();

    XmlEntry entry;

    if (!testStartTag(parser, entry, "NAMESPACEPATH"))
        return false;

    if (!getHostElement(parser, host))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_HOST_ELEMENT",
            "expected HOST element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (!getLocalNameSpacePathElement(parser, nameSpace))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_LOCALNAMESPACEPATH_ELEMENT",
            "expected LOCALNAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    expectEndTag(parser, "NAMESPACEPATH");

    return true;
}

//------------------------------------------------------------------------------
//
// getClassNameElement()
//
//     <!ELEMENT CLASSNAME EMPTY>
//     <!ATTLIST CLASSNAME %CIMName;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getClassNameElement(
    XmlParser& parser,
    CIMName& className,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "CLASSNAME"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_CLASSNAME_ELEMENT",
                "expected CLASSNAME element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        else
            return false;
    }
    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    className = getCimNameAttribute(
        parser.getLine(), entry, "CLASSNAME", false);

    if (!empty)
        expectEndTag(parser, "CLASSNAME");

    return true;
}

//------------------------------------------------------------------------------
//
// getValueTypeAttribute()
//
//     VALUETYPE (string|boolean|numeric) 'string'
//
//------------------------------------------------------------------------------

CIMKeyBinding::Type XmlReader::getValueTypeAttribute(
    Uint32 lineNumber,
    const XmlEntry& entry,
    const char* elementName)
{
    const char* tmp;

    if (!entry.getAttributeValue("VALUETYPE", tmp))
        return CIMKeyBinding::STRING;

    if (strcmp(tmp, "string") == 0)
        return CIMKeyBinding::STRING;
    else if (strcmp(tmp, "boolean") == 0)
        return CIMKeyBinding::BOOLEAN;
    else if (strcmp(tmp, "numeric") == 0)
        return CIMKeyBinding::NUMERIC;

    char buffer[MESSAGE_SIZE];
    sprintf(buffer, "%s.VALUETYPE", elementName);

    MessageLoaderParms mlParms(
        "Common.XmlReader.ILLEGAL_VALUE_FOR_CIMVALUE_ATTRIBUTE",
        "Illegal value for $0 attribute; CIMValue must be one of \"string\", "
            "\"boolean\", or \"numeric\"",
        buffer);
    throw XmlSemanticError(lineNumber, mlParms);

    return CIMKeyBinding::BOOLEAN;
}

//------------------------------------------------------------------------------
//
// getKeyValueElement()
//
//     <!ELEMENT KEYVALUE (#PCDATA)>
//     <!ATTLIST KEYVALUE
//         VALUETYPE (string|boolean|numeric)  'string'>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getKeyValueElement(
    XmlParser& parser,
    CIMKeyBinding::Type& type,
    String& value)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "KEYVALUE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    type = getValueTypeAttribute(parser.getLine(), entry, "KEYVALUE");

    value.clear();

    if (!empty)
    {
        if (!parser.next(entry))
            throw XmlException(XmlException::UNCLOSED_TAGS, parser.getLine());

        if (entry.type == XmlEntry::CONTENT)
            value = String(entry.text);
        else
            parser.putBack(entry);

        expectEndTag(parser, "KEYVALUE");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getKeyBindingElement()
//
//     <!ELEMENT KEYBINDING (KEYVALUE|VALUE.REFERENCE)>
//     <!ATTLIST KEYBINDING
//         %CIMName;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getKeyBindingElement(
    XmlParser& parser,
    CIMName& name,
    String& value,
    CIMKeyBinding::Type& type)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "KEYBINDING"))
        return false;

    name = getCimNameAttribute(parser.getLine(), entry, "KEYBINDING");

    if (!getKeyValueElement(parser, type, value))
    {
        CIMObjectPath reference;

        if (!getValueReferenceElement(parser, reference))
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_KEYVALUE_OR_REFERENCE_ELEMENT",
                "Expected KEYVALUE or VALUE.REFERENCE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }

        type = CIMKeyBinding::REFERENCE;
        value = reference.toString();
    }

    expectEndTag(parser, "KEYBINDING");
    return true;
}

//------------------------------------------------------------------------------
//
// getInstanceNameElement()
//
//     <!ELEMENT INSTANCENAME (KEYBINDING*|KEYVALUE?|VALUE.REFERENCE?)>
//     <!ATTLIST INSTANCENAME
//         %ClassName;>
//
// Note: An empty key name is used in the keyBinding when the INSTANCENAME is
// specified using a KEYVALUE or a VALUE.REFERENCE.
//
//------------------------------------------------------------------------------

Boolean XmlReader::getInstanceNameElement(
    XmlParser& parser,
    String& className,
    Array<CIMKeyBinding>& keyBindings)
{
    className.clear();
    keyBindings.clear();

    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "INSTANCENAME"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    className = getClassNameAttribute(parser.getLine(), entry, "INSTANCENAME");

    if (empty)
    {
        return true;
    }

    CIMName name;
    CIMKeyBinding::Type type;
    String value;
    CIMObjectPath reference;

    if (getKeyValueElement(parser, type, value))
    {
        // Use empty key name because none was specified
        keyBindings.append(CIMKeyBinding(name, value, type));
    }
    else if (getValueReferenceElement(parser, reference))
    {
        // Use empty key name because none was specified
        type = CIMKeyBinding::REFERENCE;
        value = reference.toString();
        keyBindings.append(CIMKeyBinding(name, value, type));
    }
    else
    {
        while (getKeyBindingElement(parser, name, value, type))
        {
            keyBindings.append(CIMKeyBinding(name, value, type));
            if (keyBindings.size() > PEGASUS_MAXELEMENTS_NUM)
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.TOO_MANY_KEYBINDINGS",
                    "More than $0 key-value pairs per object path"
                        " are not supported.",
                    PEGASUS_MAXELEMENTS_NUM);
                throw XmlValidationError(parser.getLine(), mlParms);
            }
        }
    }

    expectEndTag(parser, "INSTANCENAME");

    return true;
}

Boolean XmlReader::getInstanceNameElement(
    XmlParser& parser,
    CIMObjectPath& instanceName)
{
    String className;
    Array<CIMKeyBinding> keyBindings;

    if (!XmlReader::getInstanceNameElement(parser, className, keyBindings))
        return false;

    instanceName.set(String(), CIMNamespaceName(), className, keyBindings);
    return true;
}

//------------------------------------------------------------------------------
//
// getInstancePathElement()
//
//     <!ELEMENT INSTANCEPATH (NAMESPACEPATH,INSTANCENAME)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getInstancePathElement(
    XmlParser& parser,
    CIMObjectPath& reference)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "INSTANCEPATH"))
        return false;

    String host;
    String nameSpace;

    if (!getNameSpacePathElement(parser, host, nameSpace))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_NAMESPACEPATH_ELEMENT",
            "expected NAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    String className;
    Array<CIMKeyBinding> keyBindings;

    if (!getInstanceNameElement(parser, className, keyBindings))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCENAME_ELEMENT",
            "expected INSTANCENAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    reference.set(host, nameSpace, className, keyBindings);

    expectEndTag(parser, "INSTANCEPATH");
    return true;
}

//------------------------------------------------------------------------------
//
// getLocalInstancePathElement()
//
//     <!ELEMENT LOCALINSTANCEPATH (NAMESPACEPATH,INSTANCENAME)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getLocalInstancePathElement(
    XmlParser& parser,
    CIMObjectPath& reference)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "LOCALINSTANCEPATH"))
        return false;

    String nameSpace;

    if (!getLocalNameSpacePathElement(parser, nameSpace))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_LOCALNAMESPACEPATH_ELEMENT",
            "expected LOCALNAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    String className;
    Array<CIMKeyBinding> keyBindings;

    if (!getInstanceNameElement(parser, className, keyBindings))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCENAME_ELEMENT",
            "expected INSTANCENAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    reference.set(String(), nameSpace, className, keyBindings);

    expectEndTag(parser, "LOCALINSTANCEPATH");
    return true;
}

//------------------------------------------------------------------------------
//
// getClassPathElement()
//
//     <!ELEMENT CLASSPATH (NAMESPACEPATH,CLASSNAME)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getClassPathElement(
    XmlParser& parser,
    CIMObjectPath& reference)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "CLASSPATH"))
        return false;

    String host;
    String nameSpace;

    if (!getNameSpacePathElement(parser, host, nameSpace))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_NAMESPACEPATH_ELEMENT",
            "expected NAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    CIMName className;

    if (!getClassNameElement(parser, className))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CLASSNAME_ELEMENT",
            "expected CLASSNAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    reference.set(host, nameSpace, className);

    expectEndTag(parser, "CLASSPATH");
    return true;
}

//------------------------------------------------------------------------------
//
// getLocalClassPathElement()
//
//     <!ELEMENT LOCALCLASSPATH (LOCALNAMESPACEPATH,CLASSNAME)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getLocalClassPathElement(
    XmlParser& parser,
    CIMObjectPath& reference)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "LOCALCLASSPATH"))
        return false;

    String nameSpace;

    if (!getLocalNameSpacePathElement(parser, nameSpace))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_LOCALNAMESPACEPATH_ELEMENT",
            "expected LOCALNAMESPACEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    CIMName className;

    if (!getClassNameElement(parser, className))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CLASSNAME_ELEMENT",
            "expected CLASSNAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    reference.set(String(), nameSpace, className);

    expectEndTag(parser, "LOCALCLASSPATH");

    return true;
}

//------------------------------------------------------------------------------
//
// getValueReferenceElement()
//
//     <!ELEMENT VALUE.REFERENCE (CLASSPATH|LOCALCLASSPATH|CLASSNAME|
//         INSTANCEPATH|LOCALINSTANCEPATH|INSTANCENAME)>
//
//
//------------------------------------------------------------------------------
//
// Parses the input to a CIMObjectPath.  Note that today the differences
// between ClassPath and InstancePath are lost in this mapping because
// Pegasus uses the existence of keys as separator . See BUG_3302
Boolean XmlReader::getValueReferenceElement(
    XmlParser& parser,
    CIMObjectPath& reference)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE.REFERENCE"))
        return false;

    if (!parser.next(entry))
        throw XmlException(XmlException::UNCLOSED_TAGS, parser.getLine());

    if (entry.type != XmlEntry::START_TAG &&
        entry.type != XmlEntry::EMPTY_TAG)
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_START_TAGS",
            "Expected one of the following start tags: CLASSPATH, "
                "LOCALCLASSPATH, CLASSNAME, INSTANCEPATH, LOCALINSTANCEPATH, "
                "INSTANCENAME");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (strcmp(entry.text, "CLASSPATH") == 0)
    {
        parser.putBack(entry);
        getClassPathElement(parser, reference);
    }
    else if (strcmp(entry.text, "LOCALCLASSPATH") == 0)
    {
        parser.putBack(entry);
        getLocalClassPathElement(parser, reference);
    }
    else if (strcmp(entry.text, "CLASSNAME") == 0)
    {
        parser.putBack(entry);
        CIMName className;
        getClassNameElement(parser, className);
        reference.set(String(), CIMNamespaceName(), className);
    }
    else if (strcmp(entry.text, "INSTANCEPATH") == 0)
    {
        parser.putBack(entry);
        getInstancePathElement(parser, reference);
    }
    else if (strcmp(entry.text, "LOCALINSTANCEPATH") == 0)
    {
        parser.putBack(entry);
        getLocalInstancePathElement(parser, reference);
    }
    else if (strcmp(entry.text, "INSTANCENAME") == 0)
    {
        parser.putBack(entry);
        String className;
        Array<CIMKeyBinding> keyBindings;
        getInstanceNameElement(parser, className, keyBindings);
        reference.set(String(), CIMNamespaceName(), className, keyBindings);
    }

    expectEndTag(parser, "VALUE.REFERENCE");
    return true;
}

//------------------------------------------------------------------------------
//
// getValueReferenceArrayElement()
//
//     <!ELEMENT VALUE.REFARRAY (VALUE.REFERENCE*)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueReferenceArrayElement(
    XmlParser& parser,
    CIMValue& value)
{
    XmlEntry entry;
    Array<CIMObjectPath> referenceArray;
    CIMObjectPath reference;

    value.clear();

    // Get VALUE.REFARRAY open tag:

    if (!testStartTagOrEmptyTag(parser, entry, "VALUE.REFARRAY"))
        return false;

    if (entry.type != XmlEntry::EMPTY_TAG)
    {
        // For each VALUE.REFERENCE element:

        while (getValueReferenceElement(parser, reference))
        {
            referenceArray.append(reference);
        }

        expectEndTag(parser, "VALUE.REFARRAY");
    }

    value.set(referenceArray);
    return true;
}

//------------------------------------------------------------------------------
//
// getPropertyReferenceElement()
//
//     <!ELEMENT PROPERTY.REFERENCE (QUALIFIER*,(VALUE.REFERENCE)?)>
//     <!ATTLIST PROPERTY.REFERENCE
//         %CIMName;
//         %ReferenceClass;
//         %ClassOrigin;
//         %Propagated;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getPropertyReferenceElement(
    XmlParser& parser,
    CIMProperty& property)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PROPERTY.REFERENCE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PROPERTY.NAME attribute:

    CIMName name = getCimNameAttribute(
        parser.getLine(), entry, "PROPERTY.REFERENCE");

    // Get PROPERTY.REFERENCECLASS attribute:

    CIMName referenceClass = getReferenceClassAttribute(
        parser.getLine(), entry, "PROPERTY.REFERENCE");

    // Get PROPERTY.CLASSORIGIN attribute:

    CIMName classOrigin =
        getClassOriginAttribute(parser.getLine(), entry, "PROPERTY.REFERENCE");

    // Get PROPERTY.PROPAGATED

    Boolean propagated = getCimBooleanAttribute(parser.getLine(), entry,
        "PROPERTY.REFERENCE", "PROPAGATED", false, false);

    // Create property:

    CIMValue value = CIMValue(CIMTYPE_REFERENCE, false, 0);
//    value.set(CIMObjectPath());
    property = CIMProperty(
        name, value, 0, referenceClass, classOrigin, propagated);

    if (!empty)
    {
        getQualifierElements(parser, property);

        CIMObjectPath reference;

        if (getValueReferenceElement(parser, reference))
            property.setValue(reference);

        expectEndTag(parser, "PROPERTY.REFERENCE");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// GetPropertyElements()
//
//------------------------------------------------------------------------------

template<class CONTAINER>
void GetPropertyElements(XmlParser& parser, CONTAINER& container)
{
    CIMProperty property;

    while (XmlReader::getPropertyElement(parser, property) ||
        XmlReader::getPropertyArrayElement(parser, property) ||
        XmlReader::getPropertyReferenceElement(parser, property))
    {
        try
        {
            container.addProperty(property);
        }
        catch (AlreadyExistsException&)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.DUPLICATE_PROPERTY",
                "duplicate property");
            throw XmlSemanticError(parser.getLine(), mlParms);
        }
    }
}

//------------------------------------------------------------------------------
//
// getParameterElement()
//
//     <!ELEMENT PARAMETER (QUALIFIER*)>
//     <!ATTLIST PARAMETER
//         %CIMName;
//         %CIMType; #REQUIRED>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParameterElement(
    XmlParser& parser,
    CIMParameter& parameter)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMETER"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PARAMETER.NAME attribute:

    CIMName name = getCimNameAttribute(parser.getLine(), entry, "PARAMETER");

    // Get PARAMETER.TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "PARAMETER");

    // Create parameter:

    parameter = CIMParameter(name, type);

    if (!empty)
    {
        getQualifierElements(parser, parameter);

        expectEndTag(parser, "PARAMETER");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getParameterArrayElement()
//
//     <!ELEMENT PARAMETER.ARRAY (QUALIFIER*)>
//     <!ATTLIST PARAMETER.ARRAY
//         %CIMName;
//         %CIMType; #REQUIRED
//         %ArraySize;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParameterArrayElement(
    XmlParser& parser,
    CIMParameter& parameter)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMETER.ARRAY"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PARAMETER.ARRAY.NAME attribute:

    CIMName name = getCimNameAttribute(
        parser.getLine(), entry, "PARAMETER.ARRAY");

    // Get PARAMETER.ARRAY.TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "PARAMETER.ARRAY");

    // Get PARAMETER.ARRAYSIZE attribute:

    Uint32 arraySize = 0;
    getArraySizeAttribute(parser.getLine(), entry, "PARAMETER.ARRAY",arraySize);

    // Create parameter:

    parameter = CIMParameter(name, type, true, arraySize);

    if (!empty)
    {
        getQualifierElements(parser, parameter);

        expectEndTag(parser, "PARAMETER.ARRAY");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getParameterReferenceElement()
//
//     <!ELEMENT PARAMETER.REFERENCE (QUALIFIER*)>
//     <!ATTLIST PARAMETER.REFERENCE
//         %CIMName;
//         %ReferenceClass;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParameterReferenceElement(
    XmlParser& parser,
    CIMParameter& parameter)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMETER.REFERENCE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PARAMETER.NAME attribute:

    CIMName name = getCimNameAttribute(
        parser.getLine(), entry, "PARAMETER.REFERENCE");

    // Get PARAMETER.REFERENCECLASS attribute:

    CIMName referenceClass = getReferenceClassAttribute(
        parser.getLine(), entry, "PARAMETER.REFERENCE");

    // Create parameter:

    parameter = CIMParameter(name, CIMTYPE_REFERENCE, false, 0, referenceClass);

    if (!empty)
    {
        getQualifierElements(parser, parameter);
        expectEndTag(parser, "PARAMETER.REFERENCE");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getParameterReferenceArrayElement()
//
//     <!ELEMENT PARAMETER.REFARRAY (QUALIFIER*)>
//     <!ATTLIST PARAMETER.REFARRAY
//         %CIMName;
//         %ReferenceClass;
//         %ArraySize;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParameterReferenceArrayElement(
    XmlParser& parser,
    CIMParameter& parameter)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMETER.REFARRAY"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PARAMETER.NAME attribute:

    CIMName name = getCimNameAttribute(
        parser.getLine(), entry, "PARAMETER.REFARRAY");

    // Get PARAMETER.REFERENCECLASS attribute:

    CIMName referenceClass = getReferenceClassAttribute(
        parser.getLine(), entry, "PARAMETER.REFARRAY");

    // Get PARAMETER.ARRAYSIZE attribute:

    Uint32 arraySize = 0;
    getArraySizeAttribute(
        parser.getLine(), entry, "PARAMETER.REFARRAY", arraySize);

    // Create parameter:

    parameter = CIMParameter(
        name, CIMTYPE_REFERENCE, true, arraySize, referenceClass);

    if (!empty)
    {
        getQualifierElements(parser, parameter);
        expectEndTag(parser, "PARAMETER.REFARRAY");
    }

    return true;
}

//------------------------------------------------------------------------------
//
// GetParameterElements()
//
//------------------------------------------------------------------------------

template<class CONTAINER>
void GetParameterElements(XmlParser& parser, CONTAINER& container)
{
    CIMParameter parameter;

    while (XmlReader::getParameterElement(parser, parameter) ||
        XmlReader::getParameterArrayElement(parser, parameter) ||
        XmlReader::getParameterReferenceElement(parser, parameter) ||
        XmlReader::getParameterReferenceArrayElement(parser, parameter))
    {
        try
        {
            container.addParameter(parameter);
        }
        catch (AlreadyExistsException&)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.DUPLICATE_PARAMETER",
                "duplicate parameter");
            throw XmlSemanticError(parser.getLine(), mlParms);
        }
    }
}

//------------------------------------------------------------------------------
//
// getQualifierDeclElement()
//
//     <!ELEMENT QUALIFIER.DECLARATION (SCOPE?,(VALUE|VALUE.ARRAY)?)>
//     <!ATTLIST QUALIFIER.DECLARATION
//         %CIMName;
//         %CIMType; #REQUIRED
//         ISARRAY (true|false) #IMPLIED
//         %ArraySize;
//         %QualifierFlavor;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getQualifierDeclElement(
    XmlParser& parser,
    CIMQualifierDecl& qualifierDecl)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "QUALIFIER.DECLARATION"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get NAME attribute:

    CIMName name = getCimNameAttribute(
        parser.getLine(), entry, "QUALIFIER.DECLARATION");

    // Get TYPE attribute:

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "QUALIFIER.DECLARATION");

    // Get ISARRAY attribute:

    Boolean isArray = getCimBooleanAttribute(
        parser.getLine(), entry, "QUALIFIER.DECLARATION", "ISARRAY",
        false, false);

    // Get ARRAYSIZE attribute:

    Uint32 arraySize = 0;
    getArraySizeAttribute(parser.getLine(),
        entry, "QUALIFIER.DECLARATION", arraySize);

    // Get flavor oriented attributes:

    CIMFlavor flavor = getFlavor (entry, parser.getLine (),
        "QUALIFIER.DECLARATION");

    // No need to look for interior elements if empty tag:

    CIMScope scope = CIMScope ();
    CIMValue value;
    Boolean gotValue = false;

    if (!empty)
    {
        // Get the option SCOPE element:

        scope = getOptionalScope(parser);

        // Get VALUE or VALUE.ARRAY element:

        if (getValueArrayElement(parser, type, value))
        {
            if (!isArray)
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.ARRAY_WITHOUT_ISARRAY",
                    "VALUE.ARRAY element encountered without ISARRAY "
                        "attribute");
                throw XmlSemanticError(parser.getLine(), mlParms);
            }

            if (arraySize && arraySize != value.getArraySize())
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.ARRAY_SIZE_NOT_SAME",
                    "VALUE.ARRAY size is not the same as ARRAYSIZE attribute");
                throw XmlSemanticError(parser.getLine(), mlParms);
            }

            gotValue = true;
        }
        else if (getValueElement(parser, type, value))
        {
            if (isArray)
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.ARRAY_ATTRIBUTE_DIFFERENT",
                    "ISARRAY attribute used but VALUE element encountered");
                throw XmlSemanticError(parser.getLine(), mlParms);
            }

            gotValue = true;
        }

        // Now get the closing tag:

        expectEndTag(parser, "QUALIFIER.DECLARATION");
    }

    if (!gotValue)
    {
        if (isArray)
            value.setNullValue(type, true, arraySize);
        else
            value.setNullValue(type, false);
    }

    CIMQualifierDecl tmp(name, value, scope, flavor, arraySize);
    qualifierDecl = CIMQualifierDecl(name, value, scope, flavor, arraySize);
    return true;
}

//------------------------------------------------------------------------------
// getMethodElement()
//
//     <!ELEMENT METHOD (QUALIFIER*,(PARAMETER|PARAMETER.REFERENCE|
//         PARAMETER.ARRAY|PARAMETER.REFARRAY)*)>
//     <!ATTLIST METHOD
//         %CIMName;
//         %CIMType; #IMPLIED
//         %ClassOrigin;
//         %Propagated;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getMethodElement(XmlParser& parser, CIMMethod& method)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "METHOD"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    CIMName name = getCimNameAttribute(parser.getLine(), entry, "METHOD");

    CIMType type;
    getCimTypeAttribute(parser.getLine(), entry, type, "METHOD");

    CIMName classOrigin =
        getClassOriginAttribute(parser.getLine(), entry, "METHOD");

    Boolean propagated = getCimBooleanAttribute(
        parser.getLine(), entry, "METHOD", "PROPAGATED", false, false);

    method = CIMMethod(name, type, classOrigin, propagated);

    if (!empty)
    {
        // ATTN-RK-P2-20020219: Decoding algorithm must not depend on the
        // ordering of qualifiers and parameters.
        getQualifierElements(parser, method);

        GetParameterElements(parser, method);

        expectEndTag(parser, "METHOD");
    }

    return true;
}

//------------------------------------------------------------------------------
// getClassElement()
//
//     <!ELEMENT CLASS (QUALIFIER*,
//         (PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*,METHOD*)>
//     <!ATTLIST CLASS %CIMName; %SuperClass;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getClassElement(XmlParser& parser, CIMClass& cimClass)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "CLASS"))
        return false;

    CIMName name = getCimNameAttribute(parser.getLine(), entry, "CLASS");

    CIMName superClass =
        getSuperClassAttribute(parser.getLine(), entry,"CLASS");

    cimClass = CIMClass(name, superClass);

    if (entry.type != XmlEntry::EMPTY_TAG)
    {
        // Get QUALIFIER elements:

        getQualifierElements(parser, cimClass);

        // Get PROPERTY elements:

        GetPropertyElements(parser, cimClass);

        // Get METHOD elements:

        CIMMethod method;

        while (getMethodElement(parser, method))
            cimClass.addMethod(method);

        // Get CLASS end tag:

        expectEndTag(parser, "CLASS");
    }

    return true;
}

//------------------------------------------------------------------------------
// getInstanceElement()
//
//     <!ELEMENT INSTANCE (QUALIFIER*,
//         (PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*) >
//     <!ATTLIST INSTANCE
//         %ClassName;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getInstanceElement(
    XmlParser& parser,
    CIMInstance& cimInstance)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "INSTANCE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    String className = getClassNameAttribute(
        parser.getLine(), entry, "INSTANCE");

    cimInstance = CIMInstance(className);

    if (!empty)
    {
        // Get QUALIFIER elements:
        getQualifierElements(parser, cimInstance);

        // Get PROPERTY elements:
        GetPropertyElements(parser, cimInstance);

        // Get INSTANCE end tag:
        expectEndTag(parser, "INSTANCE");
    }

    return true;
}

//------------------------------------------------------------------------------
// getNamedInstanceElement()
//
//     <!ELEMENT VALUE.NAMEDINSTANCE (INSTANCENAME,INSTANCE)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getNamedInstanceElement(
    XmlParser& parser,
    CIMInstance& namedInstance)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE.NAMEDINSTANCE"))
        return false;

    CIMObjectPath instanceName;

    // Get INSTANCENAME elements:

    if (!getInstanceNameElement(parser, instanceName))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCENAME_ELEMENT",
            "expected INSTANCENAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Get INSTANCE elements:

    if (!getInstanceElement(parser, namedInstance))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
            "expected INSTANCE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Get VALUE.NAMEDINSTANCE end tag:

    expectEndTag(parser, "VALUE.NAMEDINSTANCE");

    namedInstance.setPath (instanceName);

    return true;
}


//EXP_PULL_BEGIN
//------------------------------------------------------------------------------
// getInstanceWithPathElement()
//
//     <!ELEMENT VALUE.NAMEDINSTANCE (INSTANCENAME,INSTANCE)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getInstanceWithPathElement(
    XmlParser& parser,
    CIMInstance& namedInstance)
{
    XmlEntry entry;
    if (!testStartTag(parser, entry, "VALUE.INSTANCEWITHPATH"))
        return false;

    CIMObjectPath instanceName;

    // Get INSTANCENAME elements:
    if (!getInstancePathElement(parser, instanceName))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCEPATH_ELEMENT",
            "expected INSTANCEPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Get INSTANCE elements:

    if (!getInstanceElement(parser, namedInstance))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
            "expected INSTANCE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    // Get VALUE.NAMEDINSTANCE end tag:

    expectEndTag(parser, "VALUE.INSTANCEWITHPATH");

    namedInstance.setPath (instanceName);

    return true;
}

//EXP_PULL_END

//------------------------------------------------------------------------------
//
// getObject()
//
//------------------------------------------------------------------------------

void XmlReader::getObject(XmlParser& parser, CIMClass& x)
{
    if (!getClassElement(parser, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CLASS_ELEMENT",
            "expected CLASS element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// getObject()
//
//------------------------------------------------------------------------------

void XmlReader::getObject(XmlParser& parser, CIMInstance& x)
{
    if (!getInstanceElement(parser, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
            "expected INSTANCE element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// getObject()
//
//------------------------------------------------------------------------------

void XmlReader::getObject(XmlParser& parser, CIMQualifierDecl& x)
{
    if (!getQualifierDeclElement(parser, x))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_QUALIFIER_DECLARATION_ELEMENT",
            "expected QUALIFIER.DECLARATION element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

//------------------------------------------------------------------------------
//
// getMessageStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getMessageStartTag(
    XmlParser& parser,
    String& id,
    String& protocolVersion)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "MESSAGE"))
        return false;

    // Get MESSAGE.ID:

    if (!entry.getAttributeValue("ID", id))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_MISSING_MESSAGE_ID_ATTRIBUTE",
            "Invalid or missing MESSAGE.ID attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }


    // Get MESSAGE.PROTOCOLVERSION:

    if (!entry.getAttributeValue("PROTOCOLVERSION", protocolVersion))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_MISSING_PROTOCOLVERSION_ATTRIBUTE",
            "Invalid or missing MESSAGE.PROTOCOLVERSION attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getIMethodCallStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getIMethodCallStartTag(
    XmlParser& parser,
    const char*& name)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "IMETHODCALL"))
        return false;

    // Get IMETHODCALL.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_IMETHODCALL_ATTRIBUTE",
            "Missing IMETHODCALL.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }


    return true;
}

//------------------------------------------------------------------------------
//
// getIMethodResponseStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getIMethodResponseStartTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "IMETHODRESPONSE"))
        return false;

    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    // Get IMETHODRESPONSE.NAME attribute:

    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_IMETHODRESPONSE_ATTRIBUTE",
            "Missing IMETHODRESPONSE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }


    return true;
}

//------------------------------------------------------------------------------
//
// getIParamValueTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getIParamValueTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "IPARAMVALUE"))
        return false;

    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    // Get IPARAMVALUE.NAME attribute:

    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_IPARAMVALUE_ATTRIBUTE",
            "Missing IPARAMVALUE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// getIParamValueTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParamValueTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMVALUE"))
    {

        return false;
    }

    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    // Get PARAMVALUE.NAME attribute:

    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_PARAMVALUE_ATTRIBUTE",
            "Missing PARAMVALUE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    return true;
}
//------------------------------------------------------------------------------
//
// getIReturnValueTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getIReturnValueTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;
    if (!testStartTagOrEmptyTag(parser, entry, "IRETURNVALUE"))
        return false;
    //entry.print();
    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_IRETURNVALUE_ATTRIBUTE",
            "Missing IRETURNVALUE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }
    return true;
}

//EXP_PULL_END
//------------------------------------------------------------------------------
//
// rejectNullIParamValue()
//
//------------------------------------------------------------------------------

void XmlReader::rejectNullIParamValue(
    XmlParser& parser,
    Boolean isEmptyTag,
    const char* paramName)
{
    if (isEmptyTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.INVALID_NULL_IPARAMVALUE",
            "A null value is not valid for IPARAMVALUE \"$0\".",
            paramName);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

// EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// rejectNullIReturnParamValue()
//
//------------------------------------------------------------------------------

void XmlReader::rejectNullIReturnValue(
    XmlParser& parser,
    Boolean isEmptyTag,
    const char* paramName)
{
    if (isEmptyTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.INVALID_NULL_IRETURNVALUE",
            "A null value is not valid for IRETURNVALUE \"$0\".",
            paramName);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}

void XmlReader::rejectNullParamValue(
    XmlParser& parser,
    Boolean isEmptyTag,
    const char* paramName)
{
    if (isEmptyTag)
    {
        MessageLoaderParms mlParms("Common.XmlReader.INVALID_NULL_PARAMVALUE",
            "A null value is not valid for PARAMVALUE \"$0\".",
            paramName);
        throw XmlValidationError(parser.getLine(), mlParms);
    }
}
// EXP_PULL_END
//------------------------------------------------------------------------------
//
// getBooleanValueElement()
//
//     Get an elements like: "<VALUE>FALSE</VALUE>"
//
//------------------------------------------------------------------------------

Boolean XmlReader::getBooleanValueElement(
    XmlParser& parser,
    Boolean& result,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "Expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    expectContentOrCData(parser, entry);

    if (System::strcasecmp(entry.text, "TRUE") == 0)
        result = true;
    else if (System::strcasecmp(entry.text, "FALSE") == 0)
        result = false;
    else
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.INVALID_VALUE_FOR_VALUE_ELEMENT",
            "Invalid value for VALUE element: must be \"TRUE\" or \"FALSE\"");
        throw XmlSemanticError(parser.getLine(), mlParms);
    }

    expectEndTag(parser, "VALUE");

    return true;
}

//------------------------------------------------------------------------------
//
//     DMTF CR Pending
//
//     <!ELEMENT ERROR (INSTANCE*)>
//     <!ATTLIST ERROR
//         CODE CDATA #REQUIRED
//         DESCRIPTION CDATA #IMPLIED>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getErrorElement(
    XmlParser& parser,
    CIMException& cimException,
    Boolean required)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "ERROR"))
    {
        if (required)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_ERROR_ELEMENT",
                "Expected ERROR element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        return false;
    }

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get ERROR.CODE

    Uint32 tmpCode;

    if (!entry.getAttributeValue("CODE", tmpCode))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_ERROR_CODE_ATTRIBUTE",
            "missing ERROR.CODE attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Get ERROR.DESCRIPTION:

    String tmpDescription;

    entry.getAttributeValue("DESCRIPTION", tmpDescription);

    cimException =
        PEGASUS_CIM_EXCEPTION(CIMStatusCode(tmpCode), tmpDescription);

    if (!empty)
    {
        CIMInstance instance;

        while (getInstanceElement(parser, instance))
        {
            cimException.addError(instance);
        }

        expectEndTag(parser, "ERROR");
    }

    return true;
}

//------------------------------------------------------------------------------
// getValueObjectElement()
//
// <!ELEMENT VALUE.OBJECT (CLASS|INSTANCE)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueObjectElement(
    XmlParser& parser,
    CIMObject& object)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE.OBJECT"))
        return false;

    CIMInstance cimInstance;
    CIMClass cimClass;

    if (XmlReader::getInstanceElement(parser, cimInstance))
    {
        object = CIMObject(cimInstance);
    }
    else if (XmlReader::getClassElement(parser, cimClass))
    {
        object = CIMObject(cimClass);
    }
    else
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCE_OR_CLASS_ELEMENT",
            "Expected INSTANCE or CLASS element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    expectEndTag(parser, "VALUE.OBJECT");

    return true;
}

//------------------------------------------------------------------------------
// getValueObjectWithPathElement()
//
// <!ELEMENT VALUE.OBJECTWITHPATH ((CLASSPATH,CLASS)|(INSTANCEPATH,INSTANCE))>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueObjectWithPathElement(
    XmlParser& parser,
    CIMObject& objectWithPath)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE.OBJECTWITHPATH"))
        return false;

    CIMObjectPath reference;
    Boolean isInstance = false;

    if (XmlReader::getInstancePathElement(parser, reference))
        isInstance = true;
    else if (!XmlReader::getClassPathElement(parser, reference))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCEPATH_OR_CLASSPATH_ELEMENT",
            "Expected INSTANCEPATH or CLASSPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (isInstance)
    {
        CIMInstance cimInstance;

        if (!XmlReader::getInstanceElement(parser, cimInstance))
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
                "Expected INSTANCE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        objectWithPath = CIMObject (cimInstance);
        objectWithPath.setPath (reference);
    }
    else
    {
        CIMClass cimClass;

        if (!XmlReader::getClassElement(parser, cimClass))
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_CLASS_ELEMENT",
                "Expected CLASS element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        objectWithPath = CIMObject (cimClass);
        objectWithPath.setPath (reference);
    }

    expectEndTag(parser, "VALUE.OBJECTWITHPATH");

    return true;
}

//------------------------------------------------------------------------------
// getValueObjectWithLocalPathElement()
//
// <!ELEMENT VALUE.OBJECTWITHLOCALPATH
//     ((LOCALCLASSPATH,CLASS)|(LOCALINSTANCEPATH,INSTANCE))>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getValueObjectWithLocalPathElement(
    XmlParser& parser,
    CIMObject& objectWithPath)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "VALUE.OBJECTWITHLOCALPATH"))
        return false;

    CIMObjectPath reference;
    Boolean isInstance = false;

    if (XmlReader::getLocalInstancePathElement(parser, reference))
        isInstance = true;
    else if (!XmlReader::getLocalClassPathElement(parser, reference))
    {
        MessageLoaderParms mlParms(
            "Common.XmlConstants.MISSING_ELEMENT_LOCALPATH",
            MISSING_ELEMENT_LOCALPATH);
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    if (isInstance)
    {
        CIMInstance cimInstance;

        if (!XmlReader::getInstanceElement(parser, cimInstance))
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_INSTANCE_ELEMENT",
                "Expected INSTANCE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        objectWithPath = CIMObject (cimInstance);
        objectWithPath.setPath (reference);
    }
    else
    {
        CIMClass cimClass;

        if (!XmlReader::getClassElement(parser, cimClass))
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_CLASS_ELEMENT",
                "Expected CLASS element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
        objectWithPath = CIMObject (cimClass);
        objectWithPath.setPath (reference);
    }

    expectEndTag(parser, "VALUE.OBJECTWITHLOCALPATH");

    return true;
}

//------------------------------------------------------------------------------
// getObjectArray()
//
// <object>
//     (VALUE.OBJECT|VALUE.OBJECTWITHLOCALPATH|VALUE.OBJECTWITHPATH)
//
//------------------------------------------------------------------------------

void XmlReader::getObjectArray(
    XmlParser& parser,
    Array<CIMObject>& objectArray)
{
    CIMObject object;
    CIMObject objectWithPath;

    objectArray.clear();

    if (getValueObjectElement(parser, object))
    {
        objectArray.append(object);
        while (getValueObjectElement(parser, object))
            objectArray.append(object);
    }
    else if (getValueObjectWithPathElement(parser, objectWithPath))
    {
        objectArray.append(objectWithPath);
        while (getValueObjectWithPathElement(parser, objectWithPath))
            objectArray.append(objectWithPath);
    }
    else if (getValueObjectWithLocalPathElement(parser, objectWithPath))
    {
        objectArray.append(objectWithPath);
        while (getValueObjectWithLocalPathElement(parser, objectWithPath))
            objectArray.append(objectWithPath);
    }
}

//------------------------------------------------------------------------------
//
// <objectName>: (CLASSNAME|INSTANCENAME)
//
//------------------------------------------------------------------------------

// Returns true if ClassNameElement or false if InstanceNameElement
// Parse errors always throw exception
Boolean XmlReader::getObjectNameElement(
    XmlParser& parser,
    CIMObjectPath& objectName)
{
    CIMName className;

    if (getClassNameElement(parser, className, false))
    {
        objectName.set(String(), CIMNamespaceName(), className);

        // Return flag indicating this is ClassNameElement
        return true;
    }

    if (!getInstanceNameElement(parser, objectName))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_CLASSNAME_OR_INSTANCENAME_ELEMENT",
            "Expected CLASSNAME or INSTANCENAME element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Return flag indicating this is InstanceNameElement
    return false;
}

//------------------------------------------------------------------------------
//
// <!ELEMENT OBJECTPATH (INSTANCEPATH|CLASSPATH)>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getObjectPathElement(
    XmlParser& parser,
    CIMObjectPath& objectPath)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "OBJECTPATH"))
        return false;

    if (!getClassPathElement(parser, objectPath) &&
        !getInstancePathElement(parser, objectPath))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.EXPECTED_INSTANCEPATH_OR_CLASSPATH_ELEMENT",
            "expected INSTANCEPATH or CLASSPATH element");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    expectEndTag(parser, "OBJECTPATH");
    return true;
}

//------------------------------------------------------------------------------
//
// getEMethodCallStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getEMethodCallStartTag(
    XmlParser& parser,
    const char*& name)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "EXPMETHODCALL"))
        return false;

    // Get EXPMETHODCALL.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_EXPMETHODCALL_ATTRIBUTE",
            "Missing EXPMETHODCALL.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getEMethodResponseStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getEMethodResponseStartTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "EXPMETHODRESPONSE"))
        return false;

    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    // Get EXPMETHODRESPONSE.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_EXPMETHODRESPONSE_ATTRIBUTE",
            "Missing EXPMETHODRESPONSE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getEParamValueTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getEParamValueTag(
    XmlParser& parser,
    const char*& name)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "EXPPARAMVALUE"))
        return false;

    // Get EXPPARAMVALUE.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_EXPPARAMVALUE_ATTRIBUTE",
            "Missing EXPPARAMVALUE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }



    return true;
}

//------------------------------------------------------------------------------
//
// getMethodCallStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getMethodCallStartTag(
    XmlParser& parser,
    const char*& name)
{
    XmlEntry entry;

    if (!testStartTag(parser, entry, "METHODCALL"))
        return false;

    // Get METHODCALL.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_METHODCALL_ATTRIBUTE",
            "Missing METHODCALL.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }


    return true;
}

//------------------------------------------------------------------------------
//
// getMethodResponseStartTag()
//
//------------------------------------------------------------------------------

Boolean XmlReader::getMethodResponseStartTag(
    XmlParser& parser,
    const char*& name,
    Boolean& isEmptyTag)
{
    XmlEntry entry;

    if (!testStartTagOrEmptyTag(parser, entry, "METHODRESPONSE"))
        return false;

    isEmptyTag = (entry.type == XmlEntry::EMPTY_TAG);

    // Get METHODRESPONSE.NAME attribute:


    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_METHODRESPONSE_ATTRIBUTE",
            "Missing METHODRESPONSE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    return true;
}

//------------------------------------------------------------------------------
//
// getParamValueElement()
//
// <!ELEMENT PARAMVALUE (VALUE|VALUE.REFERENCE|VALUE.ARRAY|VALUE.REFARRAY)?>
// <!ATTLIST PARAMVALUE
//      %CIMName;
//      %EmbeddedObject; #IMPLIED
//      %ParamType;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getParamValueElement(
    XmlParser& parser,
    CIMParamValue& paramValue)
{
    XmlEntry entry;
    const char* name;
    CIMType type=CIMTYPE_BOOLEAN;
    CIMValue value;

    if (!testStartTagOrEmptyTag(parser, entry, "PARAMVALUE"))
        return false;

    Boolean empty = entry.type == XmlEntry::EMPTY_TAG;

    // Get PARAMVALUE.NAME attribute:

    if (!entry.getAttributeValue("NAME", name))
    {
        MessageLoaderParms mlParms(
            "Common.XmlReader.MISSING_PARAMVALUE_ATTRIBUTE",
            "Missing PARAMVALUE.NAME attribute");
        throw XmlValidationError(parser.getLine(), mlParms);
    }

    // Get PROPERTY.EmbeddedObject

    EmbeddedObjectAttributeType embeddedObject = getEmbeddedObjectAttribute(
        parser.getLine(), entry, "PARAMVALUE");

    // Get PARAMVALUE.PARAMTYPE attribute:

    Boolean gotType = getCimTypeAttribute(parser.getLine(), entry, type,
                                          "PARAMVALUE", "PARAMTYPE", false);

    if (empty)
    {
        gotType = false; // Can't distinguish array and non-array types
    }
    else
    {
        // Parse VALUE.REFERENCE and VALUE.REFARRAY type
        if ( (type == CIMTYPE_REFERENCE) || !gotType )
        {
            CIMObjectPath reference;
            if (XmlReader::getValueReferenceElement(parser, reference))
            {
                value.set(reference);
                type = CIMTYPE_REFERENCE;
                gotType = true;
            }
            else if (XmlReader::getValueReferenceArrayElement(parser, value))
            {
                type = CIMTYPE_REFERENCE;
                gotType = true;
            }
            else
            {
                gotType = false; // Can't distinguish array and non-array types
            }
        }

        // Parse non-reference value
        if ( type != CIMTYPE_REFERENCE )
        {
            CIMType effectiveType;
            if (!gotType)
            {
                // If we don't know what type the value is, read it as a String
                effectiveType = CIMTYPE_STRING;
            }
            else
            {
                effectiveType = type;
            }

            // If the EmbeddedObject attribute is present with value "object"
            // then
            //     Convert the EmbeddedObject-encoded string into a CIMObject
            if (embeddedObject != NO_EMBEDDED_OBJECT)
            {
                // The EmbeddedObject attribute is only valid on Parameters
                // of type string
                // The type must have been specified.
                if (gotType && (type == CIMTYPE_STRING))
                {
                  if (embeddedObject == EMBEDDED_OBJECT_ATTR)
                      // Used below by getValueElement() or
                      // getValueArrayElement()
                      effectiveType = CIMTYPE_OBJECT;
                  else
                      effectiveType = CIMTYPE_INSTANCE;
                }
                else
                {
                    MessageLoaderParms mlParms(
                        "Common.XmlReader.INVALID_EMBEDDEDOBJECT_TYPE",
                        "The EmbeddedObject attribute is only valid on "
                            "string types.");
                    throw XmlValidationError(parser.getLine(), mlParms);
                }
            }

            if (!XmlReader::getValueArrayElement(parser, effectiveType, value)
                && !XmlReader::getValueElement(parser, effectiveType, value))
            {
                gotType = false; // Can't distinguish array and non-array types
                value.clear();   // Isn't necessary; should already be cleared
            }
        }

        expectEndTag(parser, "PARAMVALUE");
    }

    paramValue = CIMParamValue(name, value, gotType);

    return true;
}

//------------------------------------------------------------------------------
//
// getReturnValueElement()
//
// <!ELEMENT RETURNVALUE (VALUE|VALUE.REFERENCE)>
// <!ATTLIST RETURNVALUE
//      %EmbeddedObject; #IMPLIED
//      %ParamType;>
//
//------------------------------------------------------------------------------

Boolean XmlReader::getReturnValueElement(
    XmlParser& parser,
    CIMValue& returnValue)
{
    XmlEntry entry;
    CIMType type;
    CIMValue value;

    if (!testStartTag(parser, entry, "RETURNVALUE"))
        return false;

    // Get PROPERTY.EmbeddedObject

    EmbeddedObjectAttributeType embeddedObject = getEmbeddedObjectAttribute(
        parser.getLine(), entry, "RETURNVALUE");

    // Get RETURNVALUE.PARAMTYPE attribute:
    // NOTE: Array type return values are not allowed (2/20/02)

    Boolean gotType = getCimTypeAttribute(parser.getLine(), entry, type,
                                          "RETURNVALUE", "PARAMTYPE", false);

    // Parse VALUE.REFERENCE type
    if ( (type == CIMTYPE_REFERENCE) || !gotType )
    {
        CIMObjectPath reference;
        if (XmlReader::getValueReferenceElement(parser, reference))
        {
            returnValue.set(reference);
            type = CIMTYPE_REFERENCE;
            gotType = true;
        }
        else if (type == CIMTYPE_REFERENCE)
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_REFERENCE_ELEMENT",
                "expected VALUE.REFERENCE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
    }

    // Parse non-reference return value
    if ( type != CIMTYPE_REFERENCE )
    {
        if (!gotType)
        {
            // If we don't know what type the value is, read it as a String
            type = CIMTYPE_STRING;
        }
        if (embeddedObject != NO_EMBEDDED_OBJECT)
        {
            if (gotType && (type == CIMTYPE_STRING))
            {
                if (embeddedObject == EMBEDDED_OBJECT_ATTR)
                    // Used below by getValueElement() or getValueArrayElement()
                    type = CIMTYPE_OBJECT;
                else
                  type = CIMTYPE_INSTANCE;
            }
            else
            {
                MessageLoaderParms mlParms(
                    "Common.XmlReader.INVALID_EMBEDDEDOBJECT_TYPE",
                    "The EmbeddedObject attribute is only valid on string "
                        "types.");
                throw XmlValidationError(parser.getLine(), mlParms);
            }
        }
        if ( !XmlReader::getValueElement(parser, type, returnValue) )
        {
            MessageLoaderParms mlParms(
                "Common.XmlReader.EXPECTED_VALUE_ELEMENT",
                "expected VALUE element");
            throw XmlValidationError(parser.getLine(), mlParms);
        }
    }

    expectEndTag(parser, "RETURNVALUE");

    return true;
}

//-----------------------------------------------------------------------------
//
// The following is a common set of version tests used by the different
// Pegasus Request and Response Decoders
//
//------------------------------------------------------------------------------
//
// isSupportedCIMVersion()
// tests for valid CIMVersion number
//
// Reject cimVersion not in 2.[0-9]+
//
// CIMXML Secification, Version 2.2 Final, Sect 3.2.1.1
// The CIMVERSION attribute defines the version of the CIM Specification to
// which the XML Document conforms.  It MUST be in the form of "M.N".  Where
// M is the Major Version of the specification in numeric form and N is the
// minor version of the specification in numeric form.  For example, "2.0",
// "2.1".  Implementations must only validate the major version as all minor
// versions are backward compatible.  Implementations may look at the minor
// version to determine additional capabilites.
//
//------------------------------------------------------------------------------
Boolean XmlReader::isSupportedCIMVersion(
    const char* cimVersion)
{
    Boolean cimVersionAccepted = false;
    //printf("testCIMVersion %s \n", cimVersion);
    if ((cimVersion[0] == '2') &&
        (cimVersion[1] == '.') &&
        (cimVersion[2] != 0))
    {
        // Verify that all characters after the '.' are digits
        Uint32 index = 2;
        while (isdigit(cimVersion[index]))
        {
            index++;
        }

        if (cimVersion[index] == 0)
        {
           cimVersionAccepted = true;
        }
    }
    return cimVersionAccepted;
}

//------------------------------------------------------------------------------
//
// isSupportedProtocolVersion()
// tests for valid ProtocolVersion number
//
// Reject ProtocolVersion not 1.[0-9]
//
// cimxml spec 2.2 Final Section 3261
// The PROTOCOLVERSION attribute defines the version of the CIM Operations to
// which this message conforms.  It MUST be in the form of  "M.N". Where M is
// the Major Version of the specification in numeric form and N is the minor
// version of the specification in numeric form.  For example, "1.0", "1.1".
// Implementations must only validate the major version as all minor versions
// are backward compatible. Implementations may look at the minor version to
// determine additional capabilites.
//
//------------------------------------------------------------------------------
Boolean XmlReader::isSupportedProtocolVersion(
    const String& protocolVersion)
{
    Boolean protocolVersionAccepted = false;

    //cout << "testProtocolVersion " << protocolVersion << endl;
    if ((protocolVersion.size() >= 3) &&
        (protocolVersion[0] == '1') &&
        (protocolVersion[1] == '.'))
    {
        // Verify that all characters after the '.' are digits
        Uint32 index = 2;
        while ((index < protocolVersion.size()) &&
               (protocolVersion[index] >= '0') &&
               (protocolVersion[index] <= '9'))
        {
            index++;
        }

        if (index == protocolVersion.size())
        {
            protocolVersionAccepted = true;
        }
    }
    return protocolVersionAccepted;
}

//------------------------------------------------------------------------------
//
// isSupportedDTDVersion()
// Tests for Valid dtdVersion number
// We accept DTD version 2.[0-9]+ (see Bugzilla 1556)//
//
// CIM/XML Specification, V 2.2 Final, Section 3.2.1.1
// The DTDVERSION attribute defines the version of the CIM XML Mapping to
// which the XML Document conforms.  It MUST be in the form of "M.N".  Where
// M is the Major Version of the specification in numeric form and N is the
// minor version of the specification in numeric form.  For example, "2.0",
// "2.1".  Implementations must only validate the major version as all minor
// versions are backward compatible.  Implementations may look at the minor
// version to determine additional capabilites.
//
//------------------------------------------------------------------------------
Boolean XmlReader::isSupportedDTDVersion(
    const char* dtdVersion)
{
    Boolean dtdVersionAccepted = false;

    //printf("testDTDVersion %s \n", dtdVersion);
    if ((dtdVersion[0] == '2') &&
        (dtdVersion[1] == '.') &&
        (dtdVersion[2] != 0))
    {
        // Verify that all characters after the '.' are digits
        Uint32 index = 2;
        while (isdigit(dtdVersion[index]))
        {
            index++;
        }

        if (dtdVersion[index] == 0)
        {
           dtdVersionAccepted = true;
        }
    }
    return dtdVersionAccepted;
}
PEGASUS_NAMESPACE_END
