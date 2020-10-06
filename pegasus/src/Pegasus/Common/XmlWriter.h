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

#ifndef Pegasus_XmlWriter_h
#define Pegasus_XmlWriter_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/Buffer.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/XmlGenerator.h>
#include <Pegasus/Common/UintArgs.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_COMMON_LINKAGE XmlWriter : public XmlGenerator
{
public:

    static void appendLocalNameSpacePathElement(
        Buffer& out,
        const CIMNamespaceName& nameSpace);

    static void appendNameSpacePathElement(
        Buffer& out,
        const String& host,
        const CIMNamespaceName& nameSpace);

    static void appendClassNameElement(
        Buffer& out,
        const CIMName& className);

    static void appendInstanceNameElement(
        Buffer& out,
        const CIMObjectPath& instanceName);

    static void appendClassPathElement(
        Buffer& out,
        const CIMObjectPath& classPath);

    static void appendInstancePathElement(
        Buffer& out,
        const CIMObjectPath& instancePath);

    static void appendLocalClassPathElement(
        Buffer& out,
        const CIMObjectPath& classPath);

    static void appendLocalInstancePathElement(
        Buffer& out,
        const CIMObjectPath& instancePath);

    static void appendLocalObjectPathElement(
        Buffer& out,
        const CIMObjectPath& objectPath);

    static void appendValueElement(
        Buffer& out,
        const CIMValue& value);

    static void printValueElement(
        const CIMValue& value,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendValueObjectWithPathElement(
        Buffer& out,
        const CIMObject& objectWithPath,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true,
        Boolean isClassObject = false,
        const CIMPropertyList& propertyList = CIMPropertyList());

    // Appends classPath or instancePath based on isClassPath param
    static void appendValueReferenceElement(
        Buffer& out,
        const CIMObjectPath& reference,
        Boolean isClassPath);

    // Append either classPath or InstancePath Element depending on
    // isClassPath value.
    static void appendClassOrInstancePathElement(
        Buffer& out,
        const CIMObjectPath& reference,
        Boolean isClassPath);

    static void printValueReferenceElement(
        const CIMObjectPath& reference,
        Boolean isClassPath,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendValueNamedInstanceElement(
        Buffer& out,
        const CIMInstance& namedInstance,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true,
        const CIMPropertyList& propertyList = CIMPropertyList());

//EXP_PULL_BEGIN
    static void appendValueInstanceWithPathElement(
        Buffer& out,
        const CIMInstance& namedInstance,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true,
        const CIMPropertyList& propertyList = CIMPropertyList());
//EXP_PULL_END

    static void appendClassElement(
        Buffer& out,
        const CIMConstClass& cimclass);

    static void printClassElement(
        const CIMConstClass& cimclass,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendInstanceElement(
        Buffer& out,
        const CIMConstInstance& instance,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true,
        const CIMPropertyList& propertyList = CIMPropertyList());


    static void printInstanceElement(
        const CIMConstInstance& instance,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendObjectElement(
        Buffer& out,
        const CIMConstObject& object,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true,
        const CIMPropertyList& propertyList = CIMPropertyList());

    static void appendPropertyElement(
        Buffer& out,
        const CIMConstProperty& property,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = true);

    static void printPropertyElement(
        const CIMConstProperty& property,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendMethodElement(
        Buffer& out,
        const CIMConstMethod& method);

    static void printMethodElement(
        const CIMConstMethod& method,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendParameterElement(
        Buffer& out,
        const CIMConstParameter& parameter);

    static void printParameterElement(
        const CIMConstParameter& parameter,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendParamValueElement(
        Buffer& out,
        const CIMParamValue& paramValue);

    static void printParamValueElement(
        const CIMParamValue& paramValue,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendQualifierElement(
        Buffer& out,
        const CIMConstQualifier& qualifier);

    static void printQualifierElement(
        const CIMConstQualifier& qualifier,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendQualifierDeclElement(
        Buffer& out,
        const CIMConstQualifierDecl& qualifierDecl);

    static void printQualifierDeclElement(
        const CIMConstQualifierDecl& qualifierDecl,
        PEGASUS_STD(ostream)& os=PEGASUS_STD(cout));

    static void appendQualifierFlavorEntity(
        Buffer& out,
        const CIMFlavor & flavor);

    static void appendScopeElement(
        Buffer& out,
        const CIMScope & scope);

    static void appendMethodCallHeader(
        Buffer& out,
        const char* host,
        const CIMName& cimMethod,
        const String& cimObject,
        const String& authenticationHeader,
        HttpMethod httpMethod,
        const AcceptLanguageList& acceptLanguages,
        const ContentLanguageList& contentLanguages,
        Uint32 contentLength,
        bool binaryRequest = false,
        bool binaryResponse = false);

    static void appendMethodResponseHeader(
        Buffer& out,
        HttpMethod httpMethod,
        const ContentLanguageList& contentLanguages,
        Uint32 contentLength,
        Uint64 serverResponseTime = 0,
        bool binaryResponse = false);

    static void appendHttpErrorResponseHeader(
        Buffer& out,
        const String& status,
        const String& cimError = String::EMPTY,
        const String& errorDetail = String::EMPTY);

    static void appendUnauthorizedResponseHeader(
        Buffer& out,
        const String& errorDetail,
        const String& content);

    static void appendParamTypeAndEmbeddedObjAttrib(
        Buffer& out,
        const CIMType& type);

    static void appendReturnValueElement(
        Buffer& out,
        const CIMValue& value);

    static void appendBooleanIParameter(
        Buffer& out,
        const char* name,
        Boolean flag);
//EXP_PULL_BEGIN
    static void appendBooleanParameter(
        Buffer& out,
        const char* name,
        Boolean flag);

    static void appendBooleanIReturnValue(
        Buffer& out,
        const char* name,
        Boolean flag);

    static void appendUint32IParameter(
        Buffer& out,
        const char* name,
        Uint32 val);

    static void appendUint32ArgIParameter(
        Buffer& out,
        const char* name,
        const Uint32Arg& val,
        const Boolean required);

    static void appendUint64ReturnValue(
        Buffer& out,
        const char* name,
        const Uint64Arg& val);
//EXP_PULL_END

    static void appendStringIParameter(
        Buffer& out,
        const char* name,
        const String& str);

//EXP_PULL_BEGIN
    static void appendStringIParameterIfNotEmpty(
        Buffer& out,
        const char* name,
        const String& str);

    static void appendStringParameter(
        Buffer& out,
        const char* name,
        const String& str);
    static void appendStringIReturnValue(
        Buffer& out,
        const char* name,
        const String& str);
//EXP_PULL_END

    static void appendClassNameIParameter(
        Buffer& out,
        const char* name,
        const CIMName& className);

    static void appendInstanceNameIParameter(
        Buffer& out,
        const char* name,
        const CIMObjectPath& instanceName);

    static void appendClassIParameter(
        Buffer& out,
        const char* name,
        const CIMConstClass& cimClass);

    static void appendInstanceIParameter(
        Buffer& out,
        const char* name,
        const CIMConstInstance& instance);

    static void appendNamedInstanceIParameter(
        Buffer& out,
        const char* name,
        const CIMInstance& namedInstance) ;

    static void appendPropertyNameIParameter(
        Buffer& out,
        const CIMName& propertyName);

    static void appendPropertyValueIParameter(
        Buffer& out,
        const char* name,
        const CIMValue& value);

    static void appendPropertyListIParameter(
        Buffer& out,
        const CIMPropertyList& propertyList);

    static void appendQualifierDeclarationIParameter(
        Buffer& out,
        const char* name,
        const CIMConstQualifierDecl& qualifierDecl);

    static Buffer formatHttpErrorRspMessage(
        const String& status,
        const String& cimError = String::EMPTY,
        const String& errorDetail = String::EMPTY);

    static Buffer formatSimpleMethodReqMessage(
        const char* host,
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& path,
        const CIMName& methodName,
        const Array<CIMParamValue>& parameters,
        const String& messageId,
        HttpMethod httpMethod,
        const String& authenticationHeader,
        const AcceptLanguageList& httpAcceptLanguages,
        const ContentLanguageList& httpContentLanguages,
        bool binaryResponse);

    static Buffer formatSimpleMethodRspMessage(
        const CIMName& methodName,
        const String& messageId,
        HttpMethod httpMethod,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& bodyParams,
        const Buffer& body,
        Uint64 serverResponseTime,
        Boolean isFirst = true,
        Boolean isLast = true);

    static Buffer formatSimpleMethodErrorRspMessage(
        const CIMName& methodName,
        const String& messageId,
        HttpMethod httpMethod,
        const CIMException& cimException);

    static Buffer formatSimpleIMethodReqMessage(
        const char* host,
        const CIMNamespaceName& nameSpace,
        const CIMName& iMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const String& authenticationHeader,
        const AcceptLanguageList& httpAcceptLanguages,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& body,
        bool binaryResponse);

    static Buffer formatSimpleIMethodRspMessage(
        const CIMName& iMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& rtnParams,
        const Buffer& body,
        Uint64 serverResponseTime,
        Boolean isFirst = true,
        Boolean isLast = true);


    static Buffer formatSimpleIMethodErrorRspMessage(
        const CIMName& iMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const CIMException& cimException);

    static void appendInstanceEParameter(
        Buffer& out,
        const char* name,
        const CIMInstance& instance);

    static void appendEMethodRequestHeader(
        Buffer& out,
        const char* requestUri,
        const char* host,
        const CIMName& cimMethod,
        HttpMethod httpMethod,
        const String& authenticationHeader,
        const AcceptLanguageList& acceptLanguages,
        const ContentLanguageList& contentLanguages,
        Uint32 contentLength);

    static void appendEMethodResponseHeader(
        Buffer& out,
        HttpMethod httpMethod,
        const ContentLanguageList& contentLanguages,
        Uint32 contentLength);

    static Buffer formatSimpleEMethodReqMessage(
        const char* requestUri,
        const char* host,
        const CIMName& eMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const String& authenticationHeader,
        const AcceptLanguageList& httpAcceptLanguages,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& body);

    static Buffer formatSimpleEMethodRspMessage(
        const CIMName& eMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const ContentLanguageList& httpContentLanguages,
        const Buffer& body);

    static Buffer formatSimpleEMethodErrorRspMessage(
        const CIMName& eMethodName,
        const String& messageId,
        HttpMethod httpMethod,
        const CIMException& cimException);

    static String getNextMessageId();

    /** Converts the given CIMKeyBinding type to one of the following:
        "boolean", "string", or "numeric"
    */
    static const StrLit keyBindingTypeToString (CIMKeyBinding::Type type);

private:

    static void _appendMessageElementBegin(
        Buffer& out,
        const String& messageId);
        static void _appendMessageElementEnd(
        Buffer& out);

    static void _appendSimpleReqElementBegin(Buffer& out);
    static void _appendSimpleReqElementEnd(Buffer& out);

    static void _appendMethodCallElementBegin(
        Buffer& out,
        const CIMName& name);

    static void _appendMethodCallElementEnd(
        Buffer& out);

    static void _appendIMethodCallElementBegin(
        Buffer& out,
        const CIMName& name);
        static void _appendIMethodCallElementEnd(
        Buffer& out);

//EXP_PULL_BEGIN
    static void _appendParamValueElementBegin(
        Buffer& out,
        const char* name);

    static void _appendParamValueElementEnd(
        Buffer& out);
//EXP_PULL_END

    static void _appendIParamValueElementBegin(
        Buffer& out,
        const char* name);

    static void _appendIParamValueElementEnd(
        Buffer& out);

    static void _appendSimpleRspElementBegin(Buffer& out);
    static void _appendSimpleRspElementEnd(Buffer& out);

    static void _appendMethodResponseElementBegin(
        Buffer& out,
        const CIMName& name);
        static void _appendMethodResponseElementEnd(
        Buffer& out);

    static void _appendIMethodResponseElementBegin(
        Buffer& out,
        const CIMName& name);
        static void _appendIMethodResponseElementEnd(
        Buffer& out);

    static void _appendErrorElement(
        Buffer& out,
        const CIMException& cimException);

    static void _appendIReturnValueElementBegin(Buffer& out);
    static void _appendIReturnValueElementEnd(Buffer& out);

//EXP_PULL_BEGIN
    static void _appendIReturnValueElementWithNameBegin(
        Buffer& out,
        const char* name);
// EXP_PULL_END

    static void _appendSimpleExportReqElementBegin(Buffer& out);
    static void _appendSimpleExportReqElementEnd(Buffer& out);

    static void _appendEMethodCallElementBegin(
        Buffer& out,
        const CIMName& name);

    static void _appendEMethodCallElementEnd(
        Buffer& out);

    static void _appendEParamValueElementBegin(
        Buffer& out,
        const char* name);
        static void _appendEParamValueElementEnd(
        Buffer& out);

    static void _appendSimpleExportRspElementBegin(Buffer& out);
    static void _appendSimpleExportRspElementEnd(Buffer& out);

    static void _appendEMethodResponseElementBegin(
        Buffer& out,
        const CIMName& name);

    static void _appendEMethodResponseElementEnd(
        Buffer& out);

    static void appendInstancePath(
        Buffer& out,
        const CIMObjectPath& reference);

    static void appendClassPath(
        Buffer& out,
        const CIMObjectPath& reference);

    XmlWriter();
};

//==============================================================================
//
// _toString() routines:
//
//==============================================================================

inline void _toString(Buffer& out, Boolean x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Uint8 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _toString(Buffer& out, Sint8 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _toString(Buffer& out, Uint16 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _toString(Buffer& out, Sint16 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _toString(Buffer& out, Uint32 x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Sint32 x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Uint64 x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Sint64 x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Real32 x)
{
    XmlWriter::append(out, Real64(x));
}

inline void _toString(Buffer& out, Real64 x)
{
    XmlWriter::append(out, x);
}

inline void _toString(Buffer& out, Char16 x)
{
    // We need to convert the Char16 to UTF8 then append the UTF8
    // character into the array.
    // NOTE: The UTF8 character could be several bytes long.
    // WARNING: This function will put in replacement character for
    // all characters that have surogate pairs.

    char str[6];
    memset(str,0x00,sizeof(str));
    char* charIN = (char *)&x;

    const Uint16 *strsrc = (Uint16 *)charIN;
    Uint16 *endsrc = (Uint16 *)&charIN[1];

    Uint8 *strtgt = (Uint8 *)str;
    Uint8 *endtgt = (Uint8 *)&str[5];

    UTF16toUTF8(&strsrc, endsrc, &strtgt, endtgt);
    out.append(str, UTF_8_COUNT_TRAIL_BYTES(str[0]) +1);
}

inline void _toString(Buffer& out, const String& x)
{
    out << x;
}

inline void _toString(Buffer& out, const CIMDateTime& x)
{
    out << x.toString();
}

inline void _toString(Buffer& out, const CIMObjectPath& x)
{
    out << x.toString();
}

inline void _toString(Buffer& out, const CIMObject& x)
{
    out << x.toString();
}
inline void _toString(Buffer& out, const CIMInstance& x)
{
    out << CIMObject(x).toString();
}

template<class T>
void _toString(Buffer& out, const T* p, Uint32 size)
{
    while (size--)
    {
        _toString(out, *p++);
        out.append(' ');
    }
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_XmlWriter_h */
