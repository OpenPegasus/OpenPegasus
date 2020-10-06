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

#ifndef Pegasus_XmlReader_h
#define Pegasus_XmlReader_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/UintArgs.h>

PEGASUS_NAMESPACE_BEGIN

class CIMQualifier;
class CIMQualifierDecl;
class CIMClass;
class CIMInstance;
class CIMProperty;
class CIMParameter;
class CIMMethod;
class CIMObject;
class CIMParamValue;
class CIMValue;

struct CharString
{
    const char *value;
    Uint32 length;
    CharString(const char *value, Uint32 length) :
        value(value), length(length)
    {
    }
};

class PEGASUS_COMMON_LINKAGE XmlReader
{
public:

    static void getXmlDeclaration(
        XmlParser& parser,
        const char*& xmlVersion,
        const char*& xmlEncoding);

    static Boolean testXmlDeclaration(
        XmlParser& parser,
        XmlEntry& entry);

    static void expectStartTag(
        XmlParser& parser,
        XmlEntry& entry,
        const char* tagName);

    static void expectEndTag(
        XmlParser& parser,
        const char* tagName);

    static void expectStartTagOrEmptyTag(
        XmlParser& parser,
        XmlEntry& entry,
        const char* tagName);

    static void skipElement(
        XmlParser& parser,
        XmlEntry& entry);

    static Boolean expectContentOrCData(
        XmlParser& parser,
        XmlEntry& entry);

    static Boolean testStartTag(
        XmlParser& parser,
        XmlEntry& entry,
        const char* tagName);

    static Boolean testEndTag(
        XmlParser& parser,
        const char* tagName);

    static Boolean testStartTagOrEmptyTag(
        XmlParser& parser,
        XmlEntry& entry,
        const char* tagName);

    static Boolean testStartTagOrEmptyTag(
        XmlParser& parser,
        XmlEntry& entry);

    static Boolean testContentOrCData(
        XmlParser& parser,
        XmlEntry& entry);

    static void getCimStartTag(
        XmlParser& parser,
        const char*& cimVersion,
        const char*& dtdVersion);

    static CIMName getCimNameAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* elementName,
        Boolean acceptNull = false);

    static String getClassNameAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* elementName);

    static CIMName getClassOriginAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* tagName);

    typedef enum _embeddedObjectAttributeType
    {
        NO_EMBEDDED_OBJECT     = 0,
        EMBEDDED_OBJECT_ATTR   = 1,
        EMBEDDED_INSTANCE_ATTR = 2
    } EmbeddedObjectAttributeType;

    static EmbeddedObjectAttributeType getEmbeddedObjectAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* tagName);

    static CIMName getReferenceClassAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* elementName);

    static CIMName getSuperClassAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* tagName);

    static Boolean getCimTypeAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        CIMType& cimType,  // Output parameter
        const char* tagName,
        const char* attributeName = "TYPE",
        Boolean required = true);

    static Boolean getCimBooleanAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* tagName,
        const char* attributeName,
        Boolean defaultValue,
        Boolean required);

    static String decodeURICharacters(String uriString);

    static CIMValue stringToValue(
        Uint32 lineNumber,
        const char* valueString,
        CIMType type);

    static CIMValue stringToValue(
        Uint32 lineNumber,
        const char* valueString,
        Uint32 valueStringLen,
        CIMType type);


    /* getValueElement - Gets the CIMType and CIMValue for a single
        XML Value element.  It expects <VALUE> as the start tag.
        @return Returns True if the value element exists
    */
    static Boolean getValueElement(
        XmlParser& parser,
        CIMType type,
        CIMValue& value);

    static Boolean getStringValueElement(
        XmlParser& parser,
        String& str,
        Boolean required);

// KS_PULL_BEGIN
    static Boolean getUint32ArgValueElement(
        XmlParser& parser,
        Uint32Arg& val,
        Boolean required);

    static Boolean getUint32ValueElement(
        XmlParser& parser,
        Uint32& val,
        Boolean required);

    static Boolean getUint64ValueElement(
        XmlParser& parser,
        Uint64Arg& val,
        Boolean required);
//KS_PULL_END

    static CIMValue stringArrayToValue(
        Uint32 lineNumber,
        const Array<const char*>& array,
        CIMType type);

    static Boolean getValueArrayElement(
        XmlParser& parser,
        CIMType type,
        CIMValue& value);

    static CIMFlavor getFlavor(
        XmlEntry& entry,
        Uint32 lineNumber,
        const char* tagName);

    static CIMScope getOptionalScope(
        XmlParser& parser);

    static Boolean getQualifierElement(
        XmlParser& parser,
        CIMQualifier& qualifier);

    static Boolean getPropertyElement(
        XmlParser& parser,
        CIMProperty& property);

    static Boolean getArraySizeAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* tagName,
        Uint32& value);

    static Boolean getPropertyArrayElement(
        XmlParser& parser,
        CIMProperty& property);

    static Boolean getHostElement(
        XmlParser& parser,
        String& host);

    static Boolean getNameSpaceElement(
        XmlParser& parser,
        CIMName& nameSpaceComponent);

    static Boolean getLocalNameSpacePathElement(
        XmlParser& parser,
        String& nameSpace);

    static Boolean getNameSpacePathElement(
        XmlParser& parser,
        String& host,
        String& nameSpace);

    static Boolean getClassNameElement(
        XmlParser& parser,
        CIMName& className,
        Boolean required = false);

    static CIMKeyBinding::Type getValueTypeAttribute(
        Uint32 lineNumber,
        const XmlEntry& entry,
        const char* elementName);

    static Boolean getKeyValueElement(
        XmlParser& parser,
        CIMKeyBinding::Type& type,
        String& value);

    static Boolean getKeyBindingElement(
        XmlParser& parser,
        CIMName& name,
        String& value,
        CIMKeyBinding::Type& type);

    static Boolean getInstanceNameElement(
        XmlParser& parser,
        String& className,
        Array<CIMKeyBinding>& keyBindings);

    static Boolean getInstanceNameElement(
        XmlParser& parser,
        CIMObjectPath& instanceName);

    static Boolean getInstancePathElement(
        XmlParser& parser,
        CIMObjectPath& reference);

    static Boolean getLocalInstancePathElement(
        XmlParser& parser,
        CIMObjectPath& reference);

    static Boolean getClassPathElement(
        XmlParser& parser,
        CIMObjectPath& reference);

    static Boolean getLocalClassPathElement(
        XmlParser& parser,
        CIMObjectPath& reference);

    static Boolean getValueReferenceElement(
        XmlParser& parser,
        CIMObjectPath& reference);

    static Boolean getValueReferenceArrayElement(
        XmlParser& parser,
        CIMValue& value);

    static Boolean getPropertyReferenceElement(
        XmlParser& parser,
        CIMProperty& property);

    static Boolean getParameterElement(
        XmlParser& parser,
        CIMParameter& parameter);

    static Boolean getParameterArrayElement(
        XmlParser& parser,
        CIMParameter& parameter);

    static Boolean getParameterReferenceElement(
        XmlParser& parser,
        CIMParameter& parameter);

    static Boolean getParameterReferenceArrayElement(
        XmlParser& parser,
        CIMParameter& parameter);

    static Boolean getQualifierDeclElement(
        XmlParser& parser,
        CIMQualifierDecl& qualifierDecl);

    static Boolean getMethodElement(
        XmlParser& parser,
        CIMMethod& method);

    static Boolean getClassElement(
        XmlParser& parser,
        CIMClass& cimClass);

    static Boolean getInstanceElement(
        XmlParser& parser,
        CIMInstance& cimInstance);

    static Boolean getNamedInstanceElement(
        XmlParser& parser,
        CIMInstance& namedInstance);

//EXP_PULL_BEGIN
    static Boolean getInstanceWithPathElement(
        XmlParser& parser,
        CIMInstance& namedInstance);
//EXP_PULL_END

    static void getObject(XmlParser& parser, CIMClass& x);

    static void getObject(XmlParser& parser, CIMInstance& x);

    static void getObject(XmlParser& parser, CIMQualifierDecl& x);

    static Boolean getMessageStartTag(
        XmlParser& parser,
        String& id,
        String& protocolVersion);

    static Boolean getIMethodCallStartTag(
        XmlParser& parser,
        const char*& name);

    static Boolean getIMethodResponseStartTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);

    static Boolean getIParamValueTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);

// EXP_PULL_BEGIN

    static Boolean getParamValueTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);

    static Boolean getIReturnValueTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);
// EXP_PULL_END
    static void rejectNullIParamValue(
        XmlParser& parser,
        Boolean isEmptyTag,
        const char* paramName);

// EXP_PULL_BEGIN
    static void rejectNullParamValue(
        XmlParser& parser,
        Boolean isEmptyTag,
        const char* paramName);
    static void rejectNullIReturnValue(
        XmlParser& parser,
        Boolean isEmptyTag,
        const char* paramName);
// EXP_PULL_END
    static Boolean getPropertyValue(
        XmlParser& parser,
        CIMValue& cimValue);

    static Boolean getBooleanValueElement(
        XmlParser& parser,
        Boolean& result,
        Boolean required = false);

    static Boolean getErrorElement(
        XmlParser& parser,
        CIMException& cimException,
        Boolean required = false);

    static Boolean getValueObjectElement(
        XmlParser& parser,
        CIMObject& object);

    static Boolean getValueObjectWithPathElement(
        XmlParser& parser,
        CIMObject& objectWithPath);

    static Boolean getValueObjectWithLocalPathElement(
        XmlParser& parser,
        CIMObject& objectWithPath);

    static void getObjectArray(
        XmlParser& parser,
        Array<CIMObject>& objectArray);

    static Boolean getObjectNameElement(
        XmlParser& parser,
        CIMObjectPath& objectName);

    static Boolean getObjectPathElement(
        XmlParser& parser,
        CIMObjectPath& objectPath);

    static Boolean getEMethodCallStartTag(
        XmlParser& parser,
        const char*& name);

    static Boolean getEMethodResponseStartTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);

    static Boolean getEParamValueTag(
        XmlParser& parser,
        const char*& name);

    static Boolean getMethodCallStartTag(
        XmlParser& parser,
        const char*& name);

    static Boolean getMethodResponseStartTag(
        XmlParser& parser,
        const char*& name,
        Boolean& isEmptyTag);

    static Boolean getParamValueElement(
        XmlParser& parser,
        CIMParamValue& paramValue);

    static Boolean getReturnValueElement(
        XmlParser& parser,
        CIMValue& returnValue);

    static Boolean isSupportedCIMVersion(
        const char* cimVersion);

    static Boolean isSupportedProtocolVersion(
        const String& protocolVersion);

    static Boolean isSupportedDTDVersion(
        const char* dtdVersion);
private:

    static CIMValue _stringArrayToValue(
        Uint32 lineNumber,
        const Array<CharString>& array,
        CIMType type);

    XmlReader()
    {

    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_XmlReader_h */
