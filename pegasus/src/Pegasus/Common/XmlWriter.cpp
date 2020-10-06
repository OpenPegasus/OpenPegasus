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
#include <cstdlib>
#include <cstdio>
#include "Constants.h"
#include "CIMClass.h"
#include "CIMClassRep.h"
#include "CIMInstance.h"
#include "CIMInstanceRep.h"
#include "CIMProperty.h"
#include "CIMPropertyRep.h"
#include "CIMMethod.h"
#include "CIMMethodRep.h"
#include "CIMParameter.h"
#include "CIMParameterRep.h"
#include "CIMParamValue.h"
#include "CIMParamValueRep.h"
#include "CIMQualifier.h"
#include "CIMQualifierRep.h"
#include "CIMQualifierDecl.h"
#include "CIMQualifierDeclRep.h"
#include "CIMValue.h"
#include "XmlWriter.h"
#include "Tracer.h"
#include <Pegasus/Common/StatisticalData.h>
#include "CommonUTF.h"
#include "Buffer.h"
#include "StrLit.h"
#include "IDFactory.h"
#include "StringConversion.h"
PEGASUS_NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//
// appendLocalNameSpacePathElement()
//
//     <!ELEMENT LOCALNAMESPACEPATH (NAMESPACE+)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendLocalNameSpacePathElement(
    Buffer& out,
    const CIMNamespaceName& nameSpace)
{
    out << STRLIT("<LOCALNAMESPACEPATH>\n");

    char* nameSpaceCopy = strdup(nameSpace.getString().getCString());

#if !defined(PEGASUS_COMPILER_MSVC) && !defined(PEGASUS_OS_ZOS)
    char *last;
    for (const char* p = strtok_r(nameSpaceCopy, "/", &last); p;
         p = strtok_r(NULL, "/", &last))
#else
    for (const char* p = strtok(nameSpaceCopy, "/"); p; p = strtok(NULL, "/"))
#endif
    {
        out << STRLIT("<NAMESPACE NAME=\"") << p << STRLIT("\"/>\n");
    }
    free(nameSpaceCopy);

    out << STRLIT("</LOCALNAMESPACEPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendNameSpacePathElement()
//
//     <!ELEMENT NAMESPACEPATH (HOST,LOCALNAMESPACEPATH)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendNameSpacePathElement(
    Buffer& out,
    const String& host,
    const CIMNamespaceName& nameSpace)
{
    out << STRLIT("<NAMESPACEPATH>\n"
                  "<HOST>") << host << STRLIT("</HOST>\n");
    appendLocalNameSpacePathElement(out, nameSpace);
    out << STRLIT("</NAMESPACEPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendClassNameElement()
//
//     <!ELEMENT CLASSNAME EMPTY>
//     <!ATTLIST CLASSNAME
//              %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendClassNameElement(
    Buffer& out,
    const CIMName& className)
{
    out << STRLIT("<CLASSNAME NAME=\"") << className << STRLIT("\"/>\n");
}

//------------------------------------------------------------------------------
//
// appendInstanceNameElement()
//
//    <!ELEMENT INSTANCENAME (KEYBINDING*|KEYVALUE?|VALUE.REFERENCE?)>
//    <!ATTLIST INSTANCENAME
//              %ClassName;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstanceNameElement(
    Buffer& out,
    const CIMObjectPath& instanceName)
{
    out << STRLIT("<INSTANCENAME CLASSNAME=\"");
    out << instanceName.getClassName() << STRLIT("\">\n");

    const Array<CIMKeyBinding>& keyBindings = instanceName.getKeyBindings();
    for (Uint32 i = 0, n = keyBindings.size(); i < n; i++)
    {
        // Append KEYBINDING ELEMENT
        // <!ELEMENT KEYBINDING (KEYVALUE|VALUE.REFERENCE)>
        //   <!ATTLIST KEYBINDING
        out << STRLIT("<KEYBINDING NAME=\"");
        out << keyBindings[i].getName() << STRLIT("\">\n");

        if (keyBindings[i].getType() == CIMKeyBinding::REFERENCE)
        {
            CIMObjectPath ref = keyBindings[i].getValue();
            // isClassPath = false
            appendValueReferenceElement(out, ref, false);
        }
        else
        {
            out << STRLIT("<KEYVALUE VALUETYPE=\"");
            out << keyBindingTypeToString(keyBindings[i].getType());
            out << STRLIT("\">");

            // fixed the special characters

            appendSpecial(out, keyBindings[i].getValue());
            out << STRLIT("</KEYVALUE>\n");
        }
        out << STRLIT("</KEYBINDING>\n");
    }
    out << STRLIT("</INSTANCENAME>\n");
}

//------------------------------------------------------------------------------
//
// appendClassPathElement()
//
//     <!ELEMENT CLASSPATH (NAMESPACEPATH,CLASSNAME)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendClassPathElement(
    Buffer& out,
    const CIMObjectPath& classPath)
{
    out << STRLIT("<CLASSPATH>\n");
    appendNameSpacePathElement(out,
                               classPath.getHost(),
                               classPath.getNameSpace());
    appendClassNameElement(out, classPath.getClassName());
    out << STRLIT("</CLASSPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendInstancePathElement()
//
//     <!ELEMENT INSTANCEPATH (NAMESPACEPATH,INSTANCENAME)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstancePathElement(
    Buffer& out,
    const CIMObjectPath& instancePath)
{
    out << STRLIT("<INSTANCEPATH>\n");
    appendNameSpacePathElement(out,
                               instancePath.getHost(),
                               instancePath.getNameSpace());
    appendInstanceNameElement(out, instancePath);
    out << STRLIT("</INSTANCEPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendLocalClassPathElement()
//
//     <!ELEMENT LOCALCLASSPATH (LOCALNAMESPACEPATH, CLASSNAME)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendLocalClassPathElement(
    Buffer& out,
    const CIMObjectPath& classPath)
{
    out << STRLIT("<LOCALCLASSPATH>\n");
    appendLocalNameSpacePathElement(out, classPath.getNameSpace());
    appendClassNameElement(out, classPath.getClassName());
    out << STRLIT("</LOCALCLASSPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendLocalInstancePathElement()
//
//     <!ELEMENT LOCALINSTANCEPATH (LOCALNAMESPACEPATH, INSTANCENAME)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendLocalInstancePathElement(
    Buffer& out,
    const CIMObjectPath& instancePath)
{
    out << STRLIT("<LOCALINSTANCEPATH>\n");
    appendLocalNameSpacePathElement(out, instancePath.getNameSpace());
    appendInstanceNameElement(out, instancePath);
    out << STRLIT("</LOCALINSTANCEPATH>\n");
}

//------------------------------------------------------------------------------
//
// appendLocalObjectPathElement()
//
//     If the reference refers to an instance, write a LOCALINSTANCEPATH;
//     otherwise write a LOCALCLASSPATH.
//
//------------------------------------------------------------------------------

// appendValueReferenceElement does this correctly with isClassPath flag
// Called only but formatSimple
void XmlWriter::appendLocalObjectPathElement(
    Buffer& out,
    const CIMObjectPath& objectPath)
{
    //
    //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
    //  distinguish instanceNames from classNames in every case
    //  The instanceName of a singleton instance of a keyless class has no
    //  key bindings
    //  See bBUG_3302.
    //
    if (objectPath.getKeyBindings ().size () != 0)
    {
        appendLocalInstancePathElement(out, objectPath);
    }
    else
    {
        appendLocalClassPathElement(out, objectPath);
    }
}

//------------------------------------------------------------------------------
//
// Helper functions for appendValueElement()
//
//------------------------------------------------------------------------------

inline void _xmlWritter_appendValue(Buffer& out, Boolean x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Uint8 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _xmlWritter_appendValue(Buffer& out, Sint8 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _xmlWritter_appendValue(Buffer& out, Uint16 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _xmlWritter_appendValue(Buffer& out, Sint16 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _xmlWritter_appendValue(Buffer& out, Uint32 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Sint32 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Uint64 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Sint64 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Real32 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, Real64 x)
{
    XmlWriter::append(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, const Char16& x)
{
    XmlWriter::appendSpecial(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, const String& x)
{
    XmlWriter::appendSpecial(out, x);
}

inline void _xmlWritter_appendValue(Buffer& out, const CIMDateTime& x)
{
    // It is not necessary to use XmlWriter::appendSpecial(), because
    // CIMDateTime values do not contain special characters.
    out << x.toString();
}

inline void _xmlWritter_appendValue(Buffer& out, const CIMObjectPath& x)
{
    // Emit Instance Path with VALUE.REFERENCE wrapper element
    XmlWriter::appendValueReferenceElement(out, x, false);
}

inline void _xmlWritter_appendValue(Buffer& out, const CIMObject& x)
{
    String myStr = x.toString();
    _xmlWritter_appendValue(out, myStr);
}

void _xmlWritter_appendValueArray(
    Buffer& out, const CIMObjectPath* p, Uint32 size)
{
    out << STRLIT("<VALUE.REFARRAY>\n");
    while (size--)
    {
        _xmlWritter_appendValue(out, *p++);
    }
    out << STRLIT("</VALUE.REFARRAY>\n");
}

template<class T>
void _xmlWritter_appendValueArray(Buffer& out, const T* p, Uint32 size)
{
    out << STRLIT("<VALUE.ARRAY>\n");

    while (size--)
    {
        out << STRLIT("<VALUE>");
        _xmlWritter_appendValue(out, *p++);
        out << STRLIT("</VALUE>\n");
    }

    out << STRLIT("</VALUE.ARRAY>\n");
}

//------------------------------------------------------------------------------
//
// appendValueElement()
//
//    <!ELEMENT VALUE (#PCDATA)>
//    <!ELEMENT VALUE.ARRAY (VALUE*)>
//    <!ELEMENT VALUE.REFERENCE
//        (CLASSPATH|LOCALCLASSPATH|CLASSNAME|INSTANCEPATH|LOCALINSTANCEPATH|
//         INSTANCENAME)>
//    <!ELEMENT VALUE.REFARRAY (VALUE.REFERENCE*)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendValueElement(
    Buffer& out,
    const CIMValue& value)
{
    if (value.isNull())
    {
        return;
    }
    if (value.isArray())
    {
        switch (value.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Array<Boolean> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT8:
            {
                Array<Uint8> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT8:
            {
                Array<Sint8> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT16:
            {
                Array<Uint16> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT16:
            {
                Array<Sint16> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT32:
            {
                Array<Uint32> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT32:
            {
                Array<Sint32> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT64:
            {
                Array<Uint64> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT64:
            {
                Array<Sint64> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL32:
            {
                Array<Real32> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL64:
            {
                Array<Real64> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_CHAR16:
            {
                Array<Char16> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_STRING:
            {
                const String* data;
                Uint32 size;
                value._get(data, size);
                _xmlWritter_appendValueArray(out, data, size);
                break;
            }

            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                Array<CIMObjectPath> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_OBJECT:
            {
                Array<CIMObject> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                Array<CIMInstance> a;
                value.get(a);
                _xmlWritter_appendValueArray(out, a.getData(), a.size());
                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        }
    }
    else if (value.getType() == CIMTYPE_REFERENCE)
    {
        // Has to be separate because it uses VALUE.REFERENCE tag
        CIMObjectPath v;
        value.get(v);
        _xmlWritter_appendValue(out, v);
    }
    else
    {
        out << STRLIT("<VALUE>");

        switch (value.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT8:
            {
                Uint8 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT8:
            {
                Sint8 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT16:
            {
                Uint16 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT16:
            {
                Sint16 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT32:
            {
                Uint32 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT32:
            {
                Sint32 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT64:
            {
                Uint64 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT64:
            {
                Sint64 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_REAL32:
            {
                Real32 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_REAL64:
            {
                Real64 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_CHAR16:
            {
                Char16 v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_STRING:
            {
                String v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_DATETIME:
            {
                CIMDateTime v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }

            case CIMTYPE_OBJECT:
            {
                CIMObject v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                CIMInstance v;
                value.get(v);
                _xmlWritter_appendValue(out, v);
                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        }

        out << STRLIT("</VALUE>\n");
    }
}

void XmlWriter::printValueElement(
    const CIMValue& value,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendValueElement(tmp, value);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendValueObjectWithPathElement()
//
//     <!ELEMENT VALUE.OBJECTWITHPATH
//         ((CLASSPATH,CLASS)|(INSTANCEPATH,INSTANCE))>
//
//------------------------------------------------------------------------------

void XmlWriter::appendValueObjectWithPathElement(
    Buffer& out,
    const CIMObject& objectWithPath,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    Boolean isClassObject,
    const CIMPropertyList& propertyList)
{
    out << STRLIT("<VALUE.OBJECTWITHPATH>\n");

    appendClassOrInstancePathElement(out, objectWithPath.getPath (),
        isClassObject);

    appendObjectElement(
        out,
        objectWithPath,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    out << STRLIT("</VALUE.OBJECTWITHPATH>\n");
}

// Append INSTANCEPATH | LOCALINSTANCEPATH | INSTANCENAME
void XmlWriter::appendInstancePath(
    Buffer& out,
    const CIMObjectPath& reference)
{
    if (reference.getHost().size())
    {
        appendInstancePathElement(out, reference);
    }
    else if (!reference.getNameSpace().isNull())
    {
        appendLocalInstancePathElement(out, reference);
    }
    else
    {
        appendInstanceNameElement(out, reference);
    }
}

// appends CLASSPATH | LOCALCLASSPATH | CLASSNAME
void XmlWriter::appendClassPath(
    Buffer& out,
    const CIMObjectPath& reference)
{
    if (reference.getHost().size())
    {
        appendClassPathElement(out, reference);
    }
    else if (!reference.getNameSpace().isNull())
    {
        appendLocalClassPathElement(out, reference);
    }
    else
    {
        appendClassNameElement(out, reference.getClassName());
    }
}
//------------------------------------------------------------------------------
//
// appendValueReferenceElement()
//
//    <!ELEMENT VALUE.REFERENCE
//        (CLASSPATH|LOCALCLASSPATH|CLASSNAME|INSTANCEPATH|LOCALINSTANCEPATH|
//         INSTANCENAME)>
//
//------------------------------------------------------------------------------
// Builds either a classPath or InstancePath based on isClassPath
// parameter which was carried forward from, for example, the
// request. The caller must define if this is a class or instance path
void XmlWriter::appendValueReferenceElement(
    Buffer& out,
    const CIMObjectPath& reference,
    Boolean isClassPath)
{
    out << STRLIT("<VALUE.REFERENCE>\n");

    appendClassOrInstancePathElement(out, reference, isClassPath);

    out << STRLIT("</VALUE.REFERENCE>\n");
}


// Append either the classPathElement or InstancePathElement depending on
// the isClassPath input argument.
//  INSTANCENAME | CLASSNAME

void XmlWriter::appendClassOrInstancePathElement(
    Buffer& out,
    const CIMObjectPath& reference,
    Boolean isClassPath)
{
    if (isClassPath)
    {
        appendClassPath(out,reference);
    }
    else
    {
        appendInstancePath(out,reference);
    }
}

void XmlWriter::printValueReferenceElement(
    const CIMObjectPath& reference,
    Boolean isClassPath,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendValueReferenceElement(tmp, reference, isClassPath);
    indentedPrint(os, tmp.getData());
}

//------------------------------------------------------------------------------
//
// appendValueNamedInstanceElement()
//
//     <!ELEMENT VALUE.NAMEDINSTANCE (INSTANCENAME,INSTANCE)>
//
//------------------------------------------------------------------------------

void XmlWriter::appendValueNamedInstanceElement(
    Buffer& out,
    const CIMInstance& namedInstance,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    out << STRLIT("<VALUE.NAMEDINSTANCE>\n");

    appendInstanceNameElement(out, namedInstance.getPath ());
    appendInstanceElement(
        out,
        namedInstance,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    out << STRLIT("</VALUE.NAMEDINSTANCE>\n");
}

//EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// appendValueInstanceWithPathElement()
//
//     <!ELEMENT VALUE.INSTANCEWITHPATH (INSTANCEPATH,INSTANCE)>
//
//------------------------------------------------------------------------------
//
void XmlWriter::appendValueInstanceWithPathElement(
    Buffer& out,
    const CIMInstance& namedInstance,
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList& propertyList)
{
    out << STRLIT("<VALUE.INSTANCEWITHPATH>\n");

    appendInstancePathElement(out, namedInstance.getPath ());
    appendInstanceElement(
        out,
        namedInstance,
        includeQualifiers,
        includeClassOrigin,
        propertyList);

    out << STRLIT("</VALUE.INSTANCEWITHPATH>\n");
}
//EXP_PULL_END
//------------------------------------------------------------------------------
//
// appendClassElement()
//
//     <!ELEMENT CLASS
//         (QUALIFIER*,(PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*,METHOD*)>
//     <!ATTLIST CLASS
//         %CIMName;
//         %SuperClass;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendClassElement(
    Buffer& out,
    const CIMConstClass& cimClass)
{
    CheckRep(cimClass._rep);
    const CIMClassRep* rep = cimClass._rep;

    // Class opening element:

    out << STRLIT("<CLASS NAME=\"")
        << rep->getClassName()
        << STRLIT("\" ");

    if (!rep->getSuperClassName().isNull())
    {
        out << STRLIT(" SUPERCLASS=\"")
            << rep->getSuperClassName()
            << STRLIT("\" ");
    }

    out << STRLIT(">\n");

    // Append Class Qualifiers:

    for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

    // Append Property definitions:

    for (Uint32 i = 0, n = rep->getPropertyCount(); i < n; i++)
        XmlWriter::appendPropertyElement(out, rep->getProperty(i));

    // Append Method definitions:

    for (Uint32 i = 0, n = rep->getMethodCount(); i < n; i++)
        XmlWriter::appendMethodElement(out, rep->getMethod(i));

    // Class closing element:

    out << STRLIT("</CLASS>\n");
}

void XmlWriter::printClassElement(
    const CIMConstClass& cimclass,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendClassElement(tmp, cimclass);
    indentedPrint(os, tmp.getData(), 4);
}

//------------------------------------------------------------------------------
//
// appendInstanceElement()
//
//     <!ELEMENT INSTANCE
//         (QUALIFIER*,(PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*)>
//     <!ATTLIST INSTANCE
//         %ClassName;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstanceElement(
    Buffer& out,
    const CIMConstInstance& instance,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    CheckRep(instance._rep);
    const CIMInstanceRep* rep = instance._rep;

    // Class opening element:

    out << STRLIT("<INSTANCE CLASSNAME=\"")
        << rep->getClassName()
        << STRLIT("\" >\n");

    // Append Instance Qualifiers:
    if(includeQualifiers)
    {
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
            XmlWriter::appendQualifierElement(out, rep->getQualifier(i));
    }
    if(propertyList.isNull())
    {
        for (Uint32 i = 0, n = rep->getPropertyCount(); i < n; i++)
        {
            XmlWriter::appendPropertyElement(
                out,
                rep->getProperty(i),
                includeQualifiers,includeClassOrigin);
        }
    }
    else
    {
        for (Uint32 i = 0, n = propertyList.size(); i < n; i++)
        {
            CIMName name = propertyList[i];
            Uint32 pos = rep->_properties.find(
                propertyList[i],
                propertyList.getCIMNameTag(i));
            if(pos != PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_XML,Tracer::LEVEL4,
                    "XmlWriter::appendInstanceElement"
                        " Filtering the property name:%s for the className:%s"
                    "since it was not filtered by the provider.",
                    (const char *)name.getString().getCString(),
                    (const char *)instance.getClassName().
                        getString().getCString()));

                XmlWriter::appendPropertyElement(
                    out,
                    rep->getProperty(pos),
                    includeQualifiers,includeClassOrigin);
            }
        }

    }

    // Instance closing element:

    out << STRLIT("</INSTANCE>\n");
}

void XmlWriter::printInstanceElement(
    const CIMConstInstance& instance,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendInstanceElement(tmp, instance);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendObjectElement()
//
// May refer to a CLASS or an INSTANCE
//
//------------------------------------------------------------------------------

void XmlWriter::appendObjectElement(
    Buffer& out,
    const CIMConstObject& object,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    if (object.isClass())
    {
        CIMConstClass c(object);
        appendClassElement(out, c);
    }
    else if (object.isInstance())
    {
        CIMConstInstance i(object);
        appendInstanceElement(
            out,
            i,
            includeQualifiers,
            includeClassOrigin,
            propertyList);
    }
    // else PEGASUS_ASSERT(0);
}

//------------------------------------------------------------------------------
//
// appendPropertyElement()
//
//     <!ELEMENT PROPERTY (QUALIFIER*,VALUE?)>
//     <!ATTLIST PROPERTY
//              %CIMName;
//              %CIMType;           #REQUIRED
//              %ClassOrigin;
//              %Propagated;>
//
//     <!ELEMENT PROPERTY.ARRAY (QUALIFIER*,VALUE.ARRAY?)>
//     <!ATTLIST PROPERTY.ARRAY
//              %CIMName;
//              %CIMType;           #REQUIRED
//              %ArraySize;
//              %ClassOrigin;
//              %Propagated;>
//
//     <!ELEMENT PROPERTY.REFERENCE (QUALIFIER*,VALUE.REFERENCE?)>
//     <!ATTLIST PROPERTY.REFERENCE
//              %CIMName;
//              %ReferenceClass;
//              %ClassOrigin;
//              %Propagated;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendPropertyElement(
    Buffer& out,
    const CIMConstProperty& property,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    CheckRep(property._rep);
    const CIMPropertyRep* rep = property._rep;

    if (rep->getValue().isArray())
    {
        out << STRLIT("<PROPERTY.ARRAY NAME=\"")
            << rep->getName()
            << STRLIT("\" ");

        if (rep->getValue().getType() == CIMTYPE_OBJECT)
        {
            // If the property array type is CIMObject, then
            //     encode the property in CIM-XML as a string array with the
            //     EmbeddedObject attribute (there is not currently a CIM-XML
            //     "object" datatype)

            Array<CIMObject> a;
            rep->getValue().get(a);
            out << STRLIT(" TYPE=\"string\"");
            // If the Embedded Object is an instance, always add the
            // EmbeddedObject attribute.
            if (a.size() > 0 && a[0].isInstance())
            {
                out << STRLIT(" EmbeddedObject=\"object\""
                              " EMBEDDEDOBJECT=\"object\"");
            }
#ifndef PEGASUS_SNIA_INTEROP_COMPATIBILITY
            else
#endif
            {
                // Else the Embedded Object is a class, always add the
                // EmbeddedObject qualifier.  Note that if the macro
                // PEGASUS_SNIA_INTEROP_COMPATIBILITY is defined, then
                // the EmbeddedObject qualifier will always be added,
                // whether it's a class or an instance.
                if (rep->findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT) ==
                        PEG_NOT_FOUND)
                {
                    // Note that addQualifiers() cannot be called on a const
                    // CIMQualifierRep.  In this case we really do want to add
                    // the EmbeddedObject qualifier, so we cast away the
                    // constness.
                    CIMPropertyRep* tmpRep = const_cast<CIMPropertyRep*>(rep);
                    tmpRep->addQualifier(
                        CIMQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT,
                                     true));
                }
            }
        }
        else if (rep->getValue().getType() == CIMTYPE_INSTANCE)
        {
            // If the property array type is CIMInstance, then
            //   encode the property in CIM-XML as a string array with the
            //   EmbeddedObject attribute (there is not currently a CIM-XML
            //   "instance" datatype)

            Array<CIMInstance> a;
            rep->getValue().get(a);
            out << STRLIT(" TYPE=\"string\"");

            // add the EmbeddedObject attribute
            if (a.size() > 0)
            {
                out << STRLIT(" EmbeddedObject=\"instance\""
                              " EMBEDDEDOBJECT=\"instance\"");

                // Note that if the macro PEGASUS_SNIA_INTEROP_COMPATIBILITY is
                // defined, then the EmbeddedInstance qualifier will be added
# ifdef PEGASUS_SNIA_INTEROP_COMPATIBILITY
                if (rep->findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE)
                        == PEG_NOT_FOUND)
                {
                    // Note that addQualifiers() cannot be called on a const
                    // CIMQualifierRep.  In this case we really do want to add
                    // the EmbeddedInstance qualifier, so we cast away the
                    // constness.

                    // For now, we assume that all the embedded instances in
                    // the array are of the same type
                    CIMPropertyRep* tmpRep = const_cast<CIMPropertyRep*>(rep);
                    tmpRep->addQualifier(CIMQualifier(
                        PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE,
                        a[0].getClassName().getString()));
                }
# endif
            }
        }
        else
        {
            out.append(' ');
            out << xmlWriterTypeStrings(rep->getValue().getType());
        }

        if (rep->getArraySize())
        {
            char buffer[32];
            sprintf(buffer, "%u", rep->getArraySize());
            out << STRLIT(" ARRAYSIZE=\"") << buffer;
            out.append('"');
        }

        if(includeClassOrigin && !rep->getClassOrigin().isNull())
        {
            out << STRLIT(" CLASSORIGIN=\"") << rep->getClassOrigin();
            out.append('"');
        }

        if (rep->getPropagated())
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }

        out << STRLIT(">\n");
        if(includeQualifiers)
        {
            for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
                XmlWriter::appendQualifierElement(out, rep->getQualifier(i));
        }

        XmlWriter::appendValueElement(out, rep->getValue());

        out << STRLIT("</PROPERTY.ARRAY>\n");
    }
    else if (rep->getValue().getType() == CIMTYPE_REFERENCE)
    {
        out << STRLIT("<PROPERTY.REFERENCE"
                      " NAME=\"") << rep->getName() << STRLIT("\" ");

        if (!rep->getReferenceClassName().isNull())
        {
            out << STRLIT(" REFERENCECLASS=\"") << rep->getReferenceClassName();
            out.append('"');
        }
        if(includeClassOrigin && !rep->getClassOrigin().isNull())
        {
            out << STRLIT(" CLASSORIGIN=\"") << rep->getClassOrigin();
            out.append('"');
        }

        if (rep->getPropagated())
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }

        out << STRLIT(">\n");
        if(includeQualifiers)
        {
            for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
                XmlWriter::appendQualifierElement(out, rep->getQualifier(i));
        }

        XmlWriter::appendValueElement(out, rep->getValue());

        out << STRLIT("</PROPERTY.REFERENCE>\n");
    }
    else
    {
        out << STRLIT("<PROPERTY NAME=\"") << rep->getName() << STRLIT("\" ");
        if(includeClassOrigin && !rep->getClassOrigin().isNull())
        {
            out << STRLIT(" CLASSORIGIN=\"") << rep->getClassOrigin();
            out.append('"');
        }

        if (rep->getPropagated())
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }

        if (rep->getValue().getType() == CIMTYPE_OBJECT)
        {
            // If the property type is CIMObject, then
            //   encode the property in CIM-XML as a string with the
            //   EmbeddedObject attribute (there is not currently a CIM-XML
            //   "object" datatype)

            CIMObject a;
            rep->getValue().get(a);
            out << STRLIT(" TYPE=\"string\"");

            // If the Embedded Object is an instance, always add the
            // EmbeddedObject attribute.
            if (a.isInstance())
            {
                out << STRLIT(" EmbeddedObject=\"object\""
                              " EMBEDDEDOBJECT=\"object\"");
            }
            // Else the Embedded Object is a class, always add the
            // EmbeddedObject qualifier.
#ifndef PEGASUS_SNIA_INTEROP_COMPATIBILITY
            else
#endif
            {
                // Note that if the macro PEGASUS_SNIA_INTEROP_COMPATIBILITY
                // is defined, then the EmbeddedObject qualifier will always
                // be added, whether it's a class or an instance.
                if (rep->findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT) ==
                        PEG_NOT_FOUND)
                {
                    // Note that addQualifiers() cannot be called on a const
                    // CIMQualifierRep.  In this case we really do want to add
                    // the EmbeddedObject qualifier, so we cast away the
                    // constness.
                    CIMPropertyRep* tmpRep = const_cast<CIMPropertyRep*>(rep);
                    tmpRep->addQualifier(
                        CIMQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT,
                                     true));
                }
            }
        }
        else if (rep->getValue().getType() == CIMTYPE_INSTANCE)
        {
            CIMInstance a;
            rep->getValue().get(a);
            out << STRLIT(" TYPE=\"string\""
                          " EmbeddedObject=\"instance\""
                          " EMBEDDEDOBJECT=\"instance\"");

# ifdef PEGASUS_SNIA_INTEROP_COMPATIBILITY
            if (rep->findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDOBJECT)
                == PEG_NOT_FOUND)
            {
                // Note that addQualifiers() cannot be called on a const
                // CIMQualifierRep.  In this case we really do want to add
                // the EmbeddedInstance qualifier, so we cast away the
                // constness.
                CIMPropertyRep* tmpRep = const_cast<CIMPropertyRep*>(rep);
                tmpRep->addQualifier(CIMQualifier(
                    PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE,
                    a.getClassName().getString()));
            }
# endif
        }
        else
        {
            out.append(' ');
            out << xmlWriterTypeStrings(rep->getValue().getType());
        }

        out << STRLIT(">\n");
        if(includeQualifiers)
        {
            for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
                XmlWriter::appendQualifierElement(out, rep->getQualifier(i));
        }

        XmlWriter::appendValueElement(out, rep->getValue());

        out << STRLIT("</PROPERTY>\n");
    }
}

void XmlWriter::printPropertyElement(
    const CIMConstProperty& property,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendPropertyElement(tmp, property);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendMethodElement()
//
//     <!ELEMENT METHOD (QUALIFIER*,
//         (PARAMETER|PARAMETER.REFERENCE|PARAMETER.ARRAY|PARAMETER.REFARRAY)*)>
//     <!ATTLIST METHOD
//              %CIMName;
//              %CIMType;          #IMPLIED
//              %ClassOrigin;
//              %Propagated;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendMethodElement(
    Buffer& out,
    const CIMConstMethod& method)
{
    CheckRep(method._rep);
    const CIMMethodRep* rep = method._rep;

    out << STRLIT("<METHOD NAME=\"") << rep->getName();
    out << STRLIT("\" ");

    out << xmlWriterTypeStrings(rep->getType());

    if (!rep->getClassOrigin().isNull())
    {
        out << STRLIT(" CLASSORIGIN=\"") << rep->getClassOrigin();
        out.append('"');
    }

    if (rep->getPropagated())
    {
        out << STRLIT(" PROPAGATED=\"true\"");
    }

    out << STRLIT(">\n");

    for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

    for (Uint32 i = 0, n = rep->getParameterCount(); i < n; i++)
        XmlWriter::appendParameterElement(out, rep->getParameter(i));

    out << STRLIT("</METHOD>\n");
}

void XmlWriter::printMethodElement(
    const CIMConstMethod& method,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendMethodElement(tmp, method);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendParameterElement()
//
//     <!ELEMENT PARAMETER (QUALIFIER*)>
//     <!ATTLIST PARAMETER
//              %CIMName;
//              %CIMType;      #REQUIRED>
//
//     <!ELEMENT PARAMETER.REFERENCE (QUALIFIER*)>
//     <!ATTLIST PARAMETER.REFERENCE
//              %CIMName;
//              %ReferenceClass;>
//
//     <!ELEMENT PARAMETER.ARRAY (QUALIFIER*)>
//     <!ATTLIST PARAMETER.ARRAY
//              %CIMName;
//              %CIMType;           #REQUIRED
//              %ArraySize;>
//
//     <!ELEMENT PARAMETER.REFARRAY (QUALIFIER*)>
//     <!ATTLIST PARAMETER.REFARRAY
//              %CIMName;
//              %ReferenceClass;
//              %ArraySize;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendParameterElement(
    Buffer& out,
    const CIMConstParameter& parameter)
{
    CheckRep(parameter._rep);
    const CIMParameterRep* rep = parameter._rep;

    if (rep->isArray())
    {
        if (rep->getType() == CIMTYPE_REFERENCE)
        {
            out << STRLIT("<PARAMETER.REFARRAY NAME=\"") << rep->getName();
            out.append('"');

            if (!rep->getReferenceClassName().isNull())
            {
                out << STRLIT(" REFERENCECLASS=\"");
                out << rep->getReferenceClassName().getString();
                out.append('"');
            }

            if (rep->getArraySize())
            {
                char buffer[32];
                int n = sprintf(buffer, "%u", rep->getArraySize());
                out << STRLIT(" ARRAYSIZE=\"");
                out.append(buffer, n);
                out.append('"');
            }

            out << STRLIT(">\n");

            for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
                XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

            out << STRLIT("</PARAMETER.REFARRAY>\n");
        }
        else
        {
            out << STRLIT("<PARAMETER.ARRAY"
                          " NAME=\"") << rep->getName();
            out << STRLIT("\" ") << xmlWriterTypeStrings(rep->getType());

            if (rep->getArraySize())
            {
                char buffer[32];
                sprintf(buffer, "%u", rep->getArraySize());
                out << STRLIT(" ARRAYSIZE=\"") << buffer;
                out.append('"');
            }

            out << STRLIT(">\n");

            for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
                XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

            out << STRLIT("</PARAMETER.ARRAY>\n");
        }
    }
    else if (rep->getType() == CIMTYPE_REFERENCE)
    {
        out << STRLIT("<PARAMETER.REFERENCE"
                      " NAME=\"") << rep->getName();
        out.append('"');

        if (!rep->getReferenceClassName().isNull())
        {
            out << STRLIT(" REFERENCECLASS=\"");
            out << rep->getReferenceClassName().getString();
            out.append('"');
        }
        out << STRLIT(">\n");

        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
            XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

        out << STRLIT("</PARAMETER.REFERENCE>\n");
    }
    else
    {
        out << STRLIT("<PARAMETER"
                      " NAME=\"") << rep->getName();
        out << STRLIT("\" ") << xmlWriterTypeStrings(rep->getType());

        out << STRLIT(">\n");

        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
            XmlWriter::appendQualifierElement(out, rep->getQualifier(i));

        out << STRLIT("</PARAMETER>\n");
    }
}

void XmlWriter::printParameterElement(
    const CIMConstParameter& parameter,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendParameterElement(tmp, parameter);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendParamValueElement()
//
//     <!ELEMENT PARAMVALUE (VALUE|VALUE.REFERENCE|VALUE.ARRAY|VALUE.REFARRAY)?>
//     <!ATTLIST PARAMVALUE
//         %CIMName;
//         %EmbeddedObject; #IMPLIED
//         %ParamType;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendParamValueElement(
    Buffer& out,
    const CIMParamValue& paramValue)
{
    CheckRep(paramValue._rep);
    const CIMParamValueRep* rep = paramValue._rep;

    out << STRLIT("<PARAMVALUE NAME=\"") << rep->getParameterName();
    out.append('"');

    CIMType type = rep->getValue().getType();

    if (rep->isTyped())
    {
        XmlWriter::appendParamTypeAndEmbeddedObjAttrib(out, type);
    }

    out << STRLIT(">\n");
    XmlWriter::appendValueElement(out, rep->getValue());

    out << STRLIT("</PARAMVALUE>\n");
}

void XmlWriter::printParamValueElement(
    const CIMParamValue& paramValue,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendParamValueElement(tmp, paramValue);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendQualifierElement()
//
//     <!ELEMENT QUALIFIER (VALUE|VALUE.ARRAY)>
//     <!ATTLIST QUALIFIER
//              %CIMName;
//              %CIMType;               #REQUIRED
//              %Propagated;
//              %QualifierFlavor;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendQualifierElement(
    Buffer& out,
    const CIMConstQualifier& qualifier)
{
    CheckRep(qualifier._rep);
    const CIMQualifierRep* rep = qualifier._rep;

    out << STRLIT("<QUALIFIER NAME=\"") << rep->getName();
    out << STRLIT("\" ") << xmlWriterTypeStrings(rep->getValue().getType());

    if (rep->getPropagated())
    {
        out << STRLIT(" PROPAGATED=\"true\"");
    }

    XmlWriter::appendQualifierFlavorEntity(out, rep->getFlavor());

    out << STRLIT(">\n");

    XmlWriter::appendValueElement(out, rep->getValue());

    out << STRLIT("</QUALIFIER>\n");
}

void XmlWriter::printQualifierElement(
    const CIMConstQualifier& qualifier,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendQualifierElement(tmp, qualifier);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendQualifierDeclElement()
//
//     <!ELEMENT QUALIFIER.DECLARATION (SCOPE?,(VALUE|VALUE.ARRAY)?)>
//     <!ATTLIST QUALIFIER.DECLARATION
//              %CIMName;
//              %CIMType;                       #REQUIRED
//              ISARRAY        (true|false)     #IMPLIED
//              %ArraySize;
//              %QualifierFlavor;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendQualifierDeclElement(
    Buffer& out,
    const CIMConstQualifierDecl& qualifierDecl)
{
    CheckRep(qualifierDecl._rep);
    const CIMQualifierDeclRep* rep = qualifierDecl._rep;

    out << STRLIT("<QUALIFIER.DECLARATION NAME=\"") << rep->getName();
    out << STRLIT("\" ") << xmlWriterTypeStrings(rep->getValue().getType());

    if (rep->getValue().isArray())
    {
        out << STRLIT(" ISARRAY=\"true\"");

        if (rep->getArraySize())
        {
            char buffer[64];
            int n = sprintf(buffer, " ARRAYSIZE=\"%u\"", rep->getArraySize());
            out.append(buffer, n);
        }
    }

    XmlWriter::appendQualifierFlavorEntity(out, rep->getFlavor());

    out << STRLIT(">\n");

    XmlWriter::appendScopeElement(out, rep->getScope());
    XmlWriter::appendValueElement(out, rep->getValue());

    out << STRLIT("</QUALIFIER.DECLARATION>\n");
}

void XmlWriter::printQualifierDeclElement(
    const CIMConstQualifierDecl& qualifierDecl,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendQualifierDeclElement(tmp, qualifierDecl);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendQualifierFlavorEntity()
//
//     <!ENTITY % QualifierFlavor "OVERRIDABLE  (true|false)   'true'
//                                 TOSUBCLASS   (true|false)   'true'
//                                 TOINSTANCE   (true|false)   'false'
//                                 TRANSLATABLE (true|false)   'false'">
//
//     DEPRECATION NOTE:  The attribute TOINSTANCE is DEPRECATED and MAY be
//     removed from the QualifierFlavor entity in a future version of this
//     document.  Use of this qualifier is discouraged.
//
//------------------------------------------------------------------------------

void XmlWriter::appendQualifierFlavorEntity(
    Buffer& out,
    const CIMFlavor & flavor)
{
    if (!(flavor.hasFlavor (CIMFlavor::OVERRIDABLE)))
        out << STRLIT(" OVERRIDABLE=\"false\"");

    if (!(flavor.hasFlavor (CIMFlavor::TOSUBCLASS)))
        out << STRLIT(" TOSUBCLASS=\"false\"");

    //if (flavor.hasFlavor (CIMFlavor::TOINSTANCE))
    //    out << STRLIT(" TOINSTANCE=\"true\"");

    if (flavor.hasFlavor (CIMFlavor::TRANSLATABLE))
        out << STRLIT(" TRANSLATABLE=\"true\"");
}

//------------------------------------------------------------------------------
//
// appendScopeElement()
//
//     <!ELEMENT SCOPE EMPTY>
//     <!ATTLIST SCOPE
//              CLASS        (true|false)      'false'
//              ASSOCIATION  (true|false)      'false'
//              REFERENCE    (true|false)      'false'
//              PROPERTY     (true|false)      'false'
//              METHOD       (true|false)      'false'
//              PARAMETER    (true|false)      'false'
//              INDICATION   (true|false)      'false'>
//
//------------------------------------------------------------------------------

void XmlWriter::appendScopeElement(
    Buffer& out,
    const CIMScope & scope)
{
    if (!(scope.equal (CIMScope ())))
    {
        out << STRLIT("<SCOPE");

        if (scope.hasScope (CIMScope::CLASS))
            out << STRLIT(" CLASS=\"true\"");

        if (scope.hasScope (CIMScope::ASSOCIATION))
            out << STRLIT(" ASSOCIATION=\"true\"");

        if (scope.hasScope (CIMScope::REFERENCE))
            out << STRLIT(" REFERENCE=\"true\"");

        if (scope.hasScope (CIMScope::PROPERTY))
            out << STRLIT(" PROPERTY=\"true\"");

        if (scope.hasScope (CIMScope::METHOD))
            out << STRLIT(" METHOD=\"true\"");

        if (scope.hasScope (CIMScope::PARAMETER))
            out << STRLIT(" PARAMETER=\"true\"");

        if (scope.hasScope (CIMScope::INDICATION))
            out << STRLIT(" INDICATION=\"true\"");

        out << STRLIT("/>");
    }
}

// l10n - added content language and accept language support to
// the header methods below

//------------------------------------------------------------------------------
//
// appendMethodCallHeader()
//
//     Build HTTP method call request header.
//
//------------------------------------------------------------------------------

void XmlWriter::appendMethodCallHeader(
    Buffer& out,
    const char* host,
    const CIMName& cimMethod,
    const String& cimObject,
    const String& authenticationHeader,
    HttpMethod httpMethod,
    const AcceptLanguageList& acceptLanguages,
    const ContentLanguageList& contentLanguages,
    Uint32 contentLength,
    bool binaryRequest,
    bool binaryResponse)
{
    char nn[] = { char('0' + (rand() % 10)), char('0' + (rand() % 10)),'\0'};

    // ATTN: KS 20020926 - Temporary change to issue only POST. This may
    // be changed in the DMTF CIM Operations standard in the future.
    // If we kept M-Post we would have to retry with Post. Does not
    // do that in client today. Permanent change is to retry until spec
    // updated. This change is temp to finish tests or until the retry
    // installed.  Required because of change to wbemservices cimom
    if (httpMethod == HTTP_METHOD_M_POST)
    {
        out << STRLIT("M-POST /cimom HTTP/1.1\r\n");
    }
    else
    {
        out << STRLIT("POST /cimom HTTP/1.1\r\n");
    }
    out << STRLIT("HOST: ") << host << STRLIT("\r\n");

    if (binaryRequest)
    {
        // Tell the server that the payload is encoded in the OpenPegasus
        // binary protocol.
        out << STRLIT("Content-Type: application/x-openpegasus\r\n");
    }
    else
    {
        out << STRLIT("Content-Type: application/xml; charset=utf-8\r\n");
    }

    if (binaryResponse)
    {
        // Tell the server that this client accepts the OpenPegasus binary
        // protocol.
        out << STRLIT("Accept: application/x-openpegasus\r\n");
    }

    OUTPUT_CONTENTLENGTH(out, contentLength);
    if (acceptLanguages.size() > 0)
    {
        out << STRLIT("Accept-Language: ") << acceptLanguages << STRLIT("\r\n");
    }
    if (contentLanguages.size() > 0)
    {
        out << STRLIT("Content-Language: ") << contentLanguages <<
            STRLIT("\r\n");
    }

#ifdef PEGASUS_DEBUG
    // backdoor environment variable to turn OFF client requesting transfer
    // encoding. The default is on. to turn off, set this variable to zero.
    // This should be removed when stable. This should only be turned off in
    // a debugging/testing environment.

    static const char *clientTransferEncodingOff =
        getenv("PEGASUS_HTTP_TRANSFER_ENCODING_REQUEST");

    if (!clientTransferEncodingOff || *clientTransferEncodingOff != '0')
#endif

    if (!binaryResponse)
    {
        // The binary protocol does not allow chunking.
        out << STRLIT("TE: chunked, trailers\r\n");
    }

    if (httpMethod == HTTP_METHOD_M_POST)
    {
        out << STRLIT("Man: http://www.dmtf.org/cim/mapping/http/v1.0; ns=");
        out << nn << STRLIT("\r\n");
        out << nn << STRLIT("-CIMOperation: MethodCall\r\n");
        out << nn << STRLIT("-CIMMethod: ")
            << encodeURICharacters(cimMethod.getString()) << STRLIT("\r\n");
        out << nn << STRLIT("-CIMObject: ")
            << encodeURICharacters(cimObject) << STRLIT("\r\n");
    }
    else
    {
        out << STRLIT("CIMOperation: MethodCall\r\n");
        out << STRLIT("CIMMethod: ")
            << encodeURICharacters(cimMethod.getString()) << STRLIT("\r\n");
        out << STRLIT("CIMObject: ") << encodeURICharacters(cimObject)
            << STRLIT("\r\n");
    }

    if (authenticationHeader.size())
    {
        out << authenticationHeader << STRLIT("\r\n");
    }

    out << STRLIT("\r\n");
}


void XmlWriter::appendMethodResponseHeader(
     Buffer& out,
     HttpMethod httpMethod,
     const ContentLanguageList& contentLanguages,
     Uint32 contentLength,
     Uint64 serverResponseTime,
     bool binaryResponse)
{
    // Optimize the typical case for binary messages, circumventing the
    // more expensive logic below.
    if (binaryResponse &&
        contentLength == 0 &&
        httpMethod != HTTP_METHOD_M_POST &&
        contentLanguages.size() == 0)
    {
        static const char HEADERS[] =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/x-openpegasus\r\n"
            "content-length: 0000000000\r\n"
            "CIMOperation: MethodResponse\r\n"
            "\r\n";

        // The HTTP processor fills in the content-length value later.
        // It searches for a field matching "content-length" (so the first
        // character must be lower case).
        out.append(HEADERS, sizeof(HEADERS) - 1);
        return;
    }

     out << STRLIT("HTTP/1.1 " HTTP_STATUS_OK "\r\n");

#ifndef PEGASUS_DISABLE_PERFINST
     if (StatisticalData::current()->copyGSD)
     {
         out << STRLIT("WBEMServerResponseTime: ") <<
             CIMValue(serverResponseTime).toString() << STRLIT("\r\n");
     }
#endif

     if (binaryResponse)
     {
        // According to MIME RFC, the "x-" prefix should be used for all
        // non-registered values.
         out << STRLIT("Content-Type: application/x-openpegasus\r\n");
     }
     else
     {
         out << STRLIT("Content-Type: application/xml; charset=utf-8\r\n");
     }

     OUTPUT_CONTENTLENGTH(out, contentLength);

     if (contentLanguages.size() > 0)
     {
         out << STRLIT("Content-Language: ") << contentLanguages <<
             STRLIT("\r\n");
     }
     if (httpMethod == HTTP_METHOD_M_POST)
     {
         char nn[] = {char('0'+(rand() % 10)),char('0' + (rand() % 10)),'\0'};

         out << STRLIT("Ext:\r\n"
                       "Cache-Control: no-cache\r\n"
                       "Man: http://www.dmtf.org/cim/mapping/http/v1.0; ns=");
         out << nn << STRLIT("\r\n");
         out << nn << STRLIT("-CIMOperation: MethodResponse\r\n\r\n");
     }
     else
     {
         out << STRLIT("CIMOperation: MethodResponse\r\n\r\n");
     }
}


//------------------------------------------------------------------------------
//
// appendHttpErrorResponseHeader()
//
//     Build HTTP error response header.
//
//     Returns error response message in the following format:
//
//        HTTP/1.1 400 Bad Request       (using specified status code)
//        CIMError: <error type>         (if specified by caller)
//        PGErrorDetail: <error text>    (if specified by caller)
//
//------------------------------------------------------------------------------

void XmlWriter::appendHttpErrorResponseHeader(
    Buffer& out,
    const String& status,
    const String& cimError,
    const String& errorDetail)
{
    out << STRLIT("HTTP/1.1 ") << status << STRLIT("\r\n");
    if (cimError != String::EMPTY)
    {
        out << STRLIT("CIMError: ") << cimError << STRLIT("\r\n");
    }
    if (errorDetail != String::EMPTY)
    {
        out << STRLIT(PEGASUS_HTTPHEADERTAG_ERRORDETAIL ": ")
            << encodeURICharacters(errorDetail) << STRLIT("\r\n");
    }
    out << STRLIT("\r\n");
}

//------------------------------------------------------------------------------
//
// appendUnauthorizedResponseHeader()
//
//     Build HTTP authentication response header for unauthorized requests.
//
//     Returns unauthorized message in the following format:
//
//        HTTP/1.1 401 Unauthorized
//        PGErrorDetail: <error text>    (if specified by caller)
//        WWW-Authenticate: Basic realm="HostName"
//        <HTML><HEAD>
//        <TITLE>401 Unauthorized</TITLE>
//        </HEAD><BODY BGCOLOR="#99cc99">
//        <H2>TEST401 Unauthorized</H2>
//        <HR>
//        </BODY></HTML>
//
//------------------------------------------------------------------------------

void XmlWriter::appendUnauthorizedResponseHeader(
    Buffer& out,
    const String& errorDetail,
    const String& content)
{
    out << STRLIT("HTTP/1.1 " HTTP_STATUS_UNAUTHORIZED "\r\n");
    if (errorDetail.size() > 0)
    {
        out << STRLIT(PEGASUS_HTTPHEADERTAG_ERRORDETAIL ": ")
            << encodeURICharacters(errorDetail) << STRLIT("\r\n");
    }

    OUTPUT_CONTENTLENGTH(out, 0);
    out << content << STRLIT("\r\n\r\n");

//ATTN: We may need to include the following line, so that the browsers
//      can display the error message.
//    out << "<HTML><HEAD>\r\n";
//    out << "<TITLE>" << "401 Unauthorized" <<  "</TITLE>\r\n";
//    out << "</HEAD><BODY BGCOLOR=\"#99cc99\">\r\n";
//    out << "<H2>TEST" << "401 Unauthorized" << "</H2>\r\n";
//    out << "<HR>\r\n";
//    out << "</BODY></HTML>\r\n";
}


//------------------------------------------------------------------------------
//
// _appendMessageElementBegin()
// _appendMessageElementEnd()
//
//     <!ELEMENT MESSAGE (SIMPLEREQ|MULTIREQ|SIMPLERSP|MULTIRSP)>
//     <!ATTLIST MESSAGE
//         ID CDATA #REQUIRED
//         PROTOCOLVERSION CDATA #REQUIRED>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendMessageElementBegin(
    Buffer& out,
    const String& messageId)
{
    out << STRLIT("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
                  "<CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\">\n"
                  "<MESSAGE ID=\"") << messageId;
    out << STRLIT("\" PROTOCOLVERSION=\"1.0\">\n");
}

void XmlWriter::_appendMessageElementEnd(
    Buffer& out)
{
    out << STRLIT("</MESSAGE>\n</CIM>\n");
}

//------------------------------------------------------------------------------
//
// _appendSimpleReqElementBegin()
// _appendSimpleReqElementEnd()
//
//     <!ELEMENT SIMPLEREQ (IMETHODCALL|METHODCALL)>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendSimpleReqElementBegin(
    Buffer& out)
{
    out << STRLIT("<SIMPLEREQ>\n");
}

void XmlWriter::_appendSimpleReqElementEnd(
    Buffer& out)
{
    out << STRLIT("</SIMPLEREQ>\n");
}

//------------------------------------------------------------------------------
//
// _appendMethodCallElementBegin()
// _appendMethodCallElementEnd()
//
//     <!ELEMENT METHODCALL ((LOCALCLASSPATH|LOCALINSTANCEPATH),PARAMVALUE*)>
//     <!ATTLIST METHODCALL %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendMethodCallElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<METHODCALL NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendMethodCallElementEnd(
    Buffer& out)
{
    out << STRLIT("</METHODCALL>\n");
}

//------------------------------------------------------------------------------
//
// _appendIMethodCallElementBegin()
// _appendIMethodCallElementEnd()
//
//     <!ELEMENT IMETHODCALL (LOCALNAMESPACEPATH,IPARAMVALUE*)>
//     <!ATTLIST IMETHODCALL %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendIMethodCallElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<IMETHODCALL NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendIMethodCallElementEnd(
    Buffer& out)
{
    out << STRLIT("</IMETHODCALL>\n");
}

//------------------------------------------------------------------------------
//
// _appendIParamValueElementBegin()
// _appendIParamValueElementEnd()
//
//     <!ELEMENT IPARAMVALUE (VALUE|VALUE.ARRAY|VALUE.REFERENCE
//         |INSTANCENAME|CLASSNAME|QUALIFIER.DECLARATION
//         |CLASS|INSTANCE|VALUE.NAMEDINSTANCE)?>
//     <!ATTLIST IPARAMVALUE %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendIParamValueElementBegin(
    Buffer& out,
    const char* name)
{
    out << STRLIT("<IPARAMVALUE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendIParamValueElementEnd(
    Buffer& out)
{
    out << STRLIT("</IPARAMVALUE>\n");
}

//EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// _appendParamValueElementBegin()
// _appendParamValueElementEnd()
//
//     <!ELEMENT IPARAMVALUE (VALUE|VALUE.ARRAY|VALUE.REFERENCE
//         |INSTANCENAME|CLASSNAME|QUALIFIER.DECLARATION
//         |CLASS|INSTANCE|VALUE.NAMEDINSTANCE)?>
//     <!ATTLIST IPARAMVALUE %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendParamValueElementBegin(
    Buffer& out,
    const char* name)
{
    out << STRLIT("<PARAMVALUE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendParamValueElementEnd(
    Buffer& out)
{
    out << STRLIT("</PARAMVALUE>\n");
}
//EXP_PULL_END

//------------------------------------------------------------------------------
//
// _appendSimpleRspElementBegin()
// _appendSimpleRspElementEnd()
//
//     <!ELEMENT SIMPLERSP (METHODRESPONSE|IMETHODRESPONSE)>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendSimpleRspElementBegin(
    Buffer& out)
{
    out << STRLIT("<SIMPLERSP>\n");
}

void XmlWriter::_appendSimpleRspElementEnd(
    Buffer& out)
{
    out << STRLIT("</SIMPLERSP>\n");
}

//------------------------------------------------------------------------------
//
// _appendMethodResponseElementBegin()
// _appendMethodResponseElementEnd()
//
//     <!ELEMENT METHODRESPONSE (ERROR|IRETURNVALUE?)>
//     <!ATTLIST METHODRESPONSE %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendMethodResponseElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<METHODRESPONSE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendMethodResponseElementEnd(
    Buffer& out)
{
    out << STRLIT("</METHODRESPONSE>\n");
}

//------------------------------------------------------------------------------
//
// _appendIMethodResponseElementBegin()
// _appendIMethodResponseElementEnd()
//
//     <!ELEMENT IMETHODRESPONSE (ERROR|IRETURNVALUE?)>
//     <!ATTLIST IMETHODRESPONSE %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendIMethodResponseElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<IMETHODRESPONSE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendIMethodResponseElementEnd(
    Buffer& out)
{
    out << STRLIT("</IMETHODRESPONSE>\n");
}

//------------------------------------------------------------------------------
//
// _appendErrorElement()
//
//------------------------------------------------------------------------------

void XmlWriter::_appendErrorElement(
    Buffer& out,
    const CIMException& cimException)
{
    Tracer::traceCIMException(TRC_XML, Tracer::LEVEL2, cimException);

    out << STRLIT("<ERROR CODE=\"") << Uint32(cimException.getCode());
    out.append('"');

    String description = TraceableCIMException(cimException).getDescription();

    if (description != String::EMPTY)
    {
        out << STRLIT(" DESCRIPTION=\"");
        appendSpecial(out, description);
        out.append('"');
    }

    if (cimException.getErrorCount())
    {
        out << STRLIT(">");

        for (Uint32 i = 0, n = cimException.getErrorCount(); i < n; i++)
            appendInstanceElement(out, cimException.getError(i));

        out << STRLIT("</ERROR>");
    }
    else
        out << STRLIT("/>");
}

//----------------------------------------------------------------------
//
// appendParamTypeAndEmbeddedObjAttrib
// Appends the Param type and EmbeddedObject Info to the buffer
//     %EmbeddedObject; #IMPLIED
//     %ParamType;>
//
//---------------------------------------------------------------------

void XmlWriter::appendParamTypeAndEmbeddedObjAttrib(
    Buffer& out,
    const CIMType& type)
{

    // If the property type is CIMObject, then
    //   encode the property in CIM-XML as a string with the EmbeddedObject
    //   attribute (there is not currently a CIM-XML "object"
    //   datatype).
    //   Because of an error in Pegasus we were earlier outputting
    //   upper case "EMBEDDEDOBJECT" as the attribute name. The
    //   spec calls for mixed case "EmbeddedObject. Fixed in
    //   bug 7131 to output EmbeddedObject  attribute in upper
    //   case and mixed case. Receiver will ignore one or the
    //   other.
    //else
    //      output the real type
    if (type == CIMTYPE_OBJECT)
    {

        out << STRLIT(" PARAMTYPE=\"string\""
                      " EmbeddedObject=\"object\""
                      " EMBEDDEDOBJECT=\"object\"");
    }
    else if (type == CIMTYPE_INSTANCE)
    {
        out << STRLIT(" PARAMTYPE=\"string\""
                      " EmbeddedObject=\"instance\""
                      " EMBEDDEDOBJECT=\"instance\"");
    }
    else
    {
        out << STRLIT(" PARAM") << xmlWriterTypeStrings(type);
    }
}

//------------------------------------------------------------------------------
//
// appendReturnValueElement()
//
// <!ELEMENT RETURNVALUE (VALUE|VALUE.REFERENCE)>
// <!ATTLIST RETURNVALUE
//     %EmbeddedObject; #IMPLIED
//     %ParamType;>
//
//------------------------------------------------------------------------------

void XmlWriter::appendReturnValueElement(
    Buffer& out,
    const CIMValue& value)
{
    out << STRLIT("<RETURNVALUE");

    CIMType type = value.getType();

    appendParamTypeAndEmbeddedObjAttrib(out, type);

    out << STRLIT(">\n");

    // Add value.
    appendValueElement(out, value);
    out << STRLIT("</RETURNVALUE>\n");
}

//------------------------------------------------------------------------------
//
// _appendIReturnValueElementBegin()
// _appendIReturnValueElementEnd()
//
//      <!ELEMENT IRETURNVALUE (CLASSNAME*|INSTANCENAME*|VALUE*|
//          VALUE.OBJECTWITHPATH*|VALUE.OBJECTWITHLOCALPATH*|VALUE.OBJECT*|
//          OBJECTPATH*|QUALIFIER.DECLARATION*|VALUE.ARRAY?|VALUE.REFERENCE?|
//          CLASS*|INSTANCE*|VALUE.NAMEDINSTANCE*)>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendIReturnValueElementBegin(
    Buffer& out)
{
    out << STRLIT("<IRETURNVALUE>\n");
}

void XmlWriter::_appendIReturnValueElementEnd(
    Buffer& out)
{
    out << STRLIT("</IRETURNVALUE>\n");
}

//EXP_PULL_BEGIN
void XmlWriter::_appendIReturnValueElementWithNameBegin(
    Buffer& out,
    const char* name)
{
    out << STRLIT("<IRETURNVALUE NAME=\"") << name << STRLIT("\">\n");
}
//EXP_PULL_END

//------------------------------------------------------------------------------
//
// appendBooleanIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendBooleanIParameter(
    Buffer& out,
    const char* name,
    Boolean flag)
{
    _appendIParamValueElementBegin(out, name);
    out << STRLIT("<VALUE>");
    append(out, flag);
    out << STRLIT("</VALUE>\n");
    _appendIParamValueElementEnd(out);
}

//EXP_PULL_BEGIN
void XmlWriter::appendBooleanParameter(
    Buffer& out,
    const char* name,
    Boolean flag)
{
    _appendParamValueElementBegin(out, name);
    out << STRLIT("<VALUE>");
    append(out, flag);
    out << STRLIT("</VALUE>\n");
    _appendParamValueElementEnd(out);
}

void XmlWriter::appendBooleanIReturnValue(
    Buffer& out,
    const char* name,
    Boolean flag)
{
    _appendIReturnValueElementWithNameBegin(out, name);
    out << STRLIT("<VALUE>");
    append(out, flag);
    out << STRLIT("</VALUE>\n");
    _appendIReturnValueElementEnd(out);
}
void XmlWriter::appendUint32IParameter(
    Buffer& out,
    const char* name,
    Uint32 val)
{
    _appendIParamValueElementBegin(out, name);
    out << STRLIT("<VALUE>");
    append(out, val);
    out << STRLIT("</VALUE>\n");
    _appendIParamValueElementEnd(out);
}

// Can be used to generate either parameters with value,
// parameters with NULL (i.e. no value element) or
// no parameters output at all.
void XmlWriter::appendUint32ArgIParameter(
    Buffer& out,
    const char* name,
    const Uint32Arg& val,
    const Boolean required)
{
    if (!required && val.isNull())
    {
        return;
    }

    _appendIParamValueElementBegin(out, name);
    if (!val.isNull())
    {
        out << STRLIT("<VALUE>");
        append(out, val.getValue());
        out << STRLIT("</VALUE>\n");
    }
    _appendIParamValueElementEnd(out);
}

/* Output return value from Uint64Arg object as
   Value inside IRETURNVALUE
   If the state of the object is NULL output
   the parameter without value.  Else
   output the value
*/
void XmlWriter::appendUint64ReturnValue(
    Buffer& out,
    const char* name,
    const Uint64Arg& val)
{
    _appendIReturnValueElementBegin(out);
    out << STRLIT("<VALUE>");
    if (!val.isNull())
    {
        append(out, val.getValue());
    }
    out << STRLIT("</VALUE>\n");
    _appendIReturnValueElementEnd(out);
}
//EXP_PULL_END

//------------------------------------------------------------------------------
//
// appendStringIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendStringIParameter(
    Buffer& out,
    const char* name,
    const String& str)
{
    _appendIParamValueElementBegin(out, name);
    out << STRLIT("<VALUE>");
    appendSpecial(out, str);
    out << STRLIT("</VALUE>\n");
    _appendIParamValueElementEnd(out);
}

// EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// appendStringIParameterIfNotEmpty()
//
//------------------------------------------------------------------------------

void XmlWriter::appendStringIParameterIfNotEmpty(
    Buffer& out,
    const char* name,
    const String& str)
{
    if (str != String::EMPTY)
    {
        appendStringIParameter(out,name,str);
    }
}

//------------------------------------------------------------------------------
//
// appendStringParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendStringParameter(
    Buffer& out,
    const char* name,
    const String& str)
{
    _appendParamValueElementBegin(out, name);
    out << STRLIT("<VALUE>");
    appendSpecial(out, str);
    out << STRLIT("</VALUE>\n");
    _appendParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendStringIReturnValue()
//
//------------------------------------------------------------------------------

void XmlWriter::appendStringIReturnValue(
    Buffer& out,
    const char* name,
    const String& str)
{
    _appendIReturnValueElementWithNameBegin(out, name);
    out << STRLIT("<VALUE>");
    appendSpecial(out, str);
    out << STRLIT("</VALUE>\n");
    _appendIReturnValueElementEnd(out);
}
//EXP_PULL_END
//------------------------------------------------------------------------------
//
// appendClassNameIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendClassNameIParameter(
    Buffer& out,
    const char* name,
    const CIMName& className)
{
    _appendIParamValueElementBegin(out, name);

    //
    //  A NULL (unassigned) value for a parameter is specified by an
    //  <IPARAMVALUE> element with no subelement
    //
    if (!className.isNull ())
    {
        appendClassNameElement(out, className);
    }

    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendInstanceNameIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstanceNameIParameter(
    Buffer& out,
    const char* name,
    const CIMObjectPath& instanceName)
{
    _appendIParamValueElementBegin(out, name);
    appendInstanceNameElement(out, instanceName);
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendClassIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendClassIParameter(
    Buffer& out,
    const char* name,
    const CIMConstClass& cimClass)
{
    _appendIParamValueElementBegin(out, name);
    appendClassElement(out, cimClass);
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendInstanceIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstanceIParameter(
    Buffer& out,
    const char* name,
    const CIMConstInstance& instance)
{
    _appendIParamValueElementBegin(out, name);
    appendInstanceElement(out, instance);
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendNamedInstanceIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendNamedInstanceIParameter(
    Buffer& out,
    const char* name,
    const CIMInstance& namedInstance)
{
    _appendIParamValueElementBegin(out, name);
    appendValueNamedInstanceElement(out, namedInstance);
    _appendIParamValueElementEnd(out);
}

//----------------------------------------------------------
//
//  appendPropertyNameIParameter()
//
//     </IPARAMVALUE>
//     <IPARAMVALUE NAME="PropertyName"><VALUE>FreeSpace</VALUE></IPARAMVALUE>
//
//     USE: Create parameter for getProperty operation
//==========================================================
void XmlWriter::appendPropertyNameIParameter(
    Buffer& out,
    const CIMName& propertyName)
{
    _appendIParamValueElementBegin(out, "PropertyName");
    out << STRLIT("<VALUE>") << propertyName << STRLIT("</VALUE>\n");
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendPropertyValueIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendPropertyValueIParameter(
    Buffer& out,
    const char* name,
    const CIMValue& value)
{
    _appendIParamValueElementBegin(out, name);
    appendValueElement(out, value);
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendPropertyListIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendPropertyListIParameter(
    Buffer& out,
    const CIMPropertyList& propertyList)
{
    _appendIParamValueElementBegin(out, "PropertyList");

    //
    //  A NULL (unassigned) value for a parameter is specified by an
    //  <IPARAMVALUE> element with no subelement
    //
    if (!propertyList.isNull ())
    {
        out << STRLIT("<VALUE.ARRAY>\n");
        for (Uint32 i = 0; i < propertyList.size(); i++)
        {
            out << STRLIT("<VALUE>") << propertyList[i] << STRLIT("</VALUE>\n");
        }
        out << STRLIT("</VALUE.ARRAY>\n");
    }

    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// appendQualifierDeclarationIParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendQualifierDeclarationIParameter(
    Buffer& out,
    const char* name,
    const CIMConstQualifierDecl& qualifierDecl)
{
    _appendIParamValueElementBegin(out, name);
    appendQualifierDeclElement(out, qualifierDecl);
    _appendIParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatHttpErrorRspMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatHttpErrorRspMessage(
    const String& status,
    const String& cimError,
    const String& errorDetail)
{
    Buffer out;

    appendHttpErrorResponseHeader(out, status, cimError, errorDetail);

    return out;
}

// l10n - add content language support to the format methods below

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleMethodReqMessage()
//
//------------------------------------------------------------------------------

// ATTN-RK-P1-20020228: Need to complete copy elimination optimization
Buffer XmlWriter::formatSimpleMethodReqMessage(
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
    bool binaryResponse)
{
    Buffer out;
    Buffer tmp;
    CIMObjectPath localObjectPath = path;
    localObjectPath.setNameSpace(nameSpace.getString());
    localObjectPath.setHost(String::EMPTY);

    _appendMessageElementBegin(out, messageId);
    _appendSimpleReqElementBegin(out);
    _appendMethodCallElementBegin(out, methodName);
    appendLocalObjectPathElement(out, localObjectPath);
    for (Uint32 i=0; i < parameters.size(); i++)
    {
        appendParamValueElement(out, parameters[i]);
    }
    _appendMethodCallElementEnd(out);
    _appendSimpleReqElementEnd(out);
    _appendMessageElementEnd(out);

    appendMethodCallHeader(
        tmp,
        host,
        methodName,
        localObjectPath.toString(),
        authenticationHeader,
        httpMethod,
        httpAcceptLanguages,
        httpContentLanguages,
        out.size(),
        false,
        binaryResponse);
    tmp << out;

    return tmp;
}

Buffer XmlWriter::formatSimpleMethodRspMessage(
    const CIMName& methodName,
    const String& messageId,
    HttpMethod httpMethod,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& bodyParams,
    const Buffer& body,
    Uint64 serverResponseTime,
    Boolean isFirst,
    Boolean isLast)
{
    Buffer out;

    if (isFirst == true)
    {
        // NOTE: temporarily put zero for content length. the http code
        // will later decide to fill in the length or remove it altogether
        appendMethodResponseHeader(
            out, httpMethod, httpContentLanguages, 0, serverResponseTime);
        _appendMessageElementBegin(out, messageId);
        _appendSimpleRspElementBegin(out);
        _appendMethodResponseElementBegin(out, methodName);
    }

    if (body.size() != 0)
    {
        out << body;
    }

    if (isLast == true)
    {
        _appendMethodResponseElementEnd(out);
        _appendSimpleRspElementEnd(out);
        _appendMessageElementEnd(out);
    }

    return out;
}


//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleMethodErrorRspMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleMethodErrorRspMessage(
    const CIMName& methodName,
    const String& messageId,
    HttpMethod httpMethod,
    const CIMException& cimException)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleRspElementBegin(out);
    _appendMethodResponseElementBegin(out, methodName);
    _appendErrorElement(out, cimException);
    _appendMethodResponseElementEnd(out);
    _appendSimpleRspElementEnd(out);
    _appendMessageElementEnd(out);

    appendMethodResponseHeader(
        tmp,
        httpMethod,
        cimException.getContentLanguages(),
        out.size(),
        false);
    tmp << out;

    return tmp;
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleIMethodReqMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleIMethodReqMessage(
    const char* host,
    const CIMNamespaceName& nameSpace,
    const CIMName& iMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const String& authenticationHeader,
    const AcceptLanguageList& httpAcceptLanguages,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& body,
    bool binaryResponse)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleReqElementBegin(out);
    _appendIMethodCallElementBegin(out, iMethodName);
    appendLocalNameSpacePathElement(out, nameSpace.getString());
    out << body;
    _appendIMethodCallElementEnd(out);
    _appendSimpleReqElementEnd(out);
    _appendMessageElementEnd(out);

    appendMethodCallHeader(
        tmp,
        host,
        iMethodName,
        nameSpace.getString(),
        authenticationHeader,
        httpMethod,
        httpAcceptLanguages,
        httpContentLanguages,
        out.size(),
        false,
        binaryResponse);
    tmp << out;

    return tmp;
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleIMethodRspMessage()
//
//------------------------------------------------------------------------------

/*
    Formats a IMethod Response Message.  This function formats
    messages for chunking based on the isFirst and isLast parameters.
    If isFirst is set, it outputs headers and ResponseElement begin.
    It always sends the body.  If isLast is set,  it sends the rtnParams
    and the ResponseElement end followed by any parameter in the rtnParams.
    KS_PULL_TBD_TODO - Determine what we want to do with chunking. Since this
    This whole thing is based on the concept of each message sending a  single
    chunk and we will want in the future to modify so we can chose easily
    whether we want to chunk or not and send the chunks.
*/
Buffer XmlWriter::formatSimpleIMethodRspMessage(
    const CIMName& iMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& rtnParams,
    const Buffer& body,
    Uint64 serverResponseTime,
    Boolean isFirst,
    Boolean isLast)
{
    Buffer out;

    if (isFirst == true)
    {
        // NOTE: temporarily put zero for content length. the http code
        // will later decide to fill in the length or remove it altogether
        appendMethodResponseHeader(
            out, httpMethod, httpContentLanguages, 0, serverResponseTime);
        _appendMessageElementBegin(out, messageId);
        _appendSimpleRspElementBegin(out);
        _appendIMethodResponseElementBegin(out, iMethodName);

        // output the start of the return tag. Test if there is response data
        // by:
        // 1. there is data on the first chunk OR
        // 2. there is no data on the first chunk but isLast is false implying
        //    there is more non-empty data to come. If all subsequent chunks
        //    are empty, then this generates and empty response.
        if (body.size() != 0 || isLast == false)
            _appendIReturnValueElementBegin(out);
    }

    if (body.size() != 0)
    {
        out << body;
    }

    if (isLast == true)
    {
        if (body.size() != 0 || isFirst == false)
            _appendIReturnValueElementEnd(out);

        // EXP_PULL_BEGIN
        // Insert any return parameters.
        if (rtnParams.size() != 0)
        {
            out << rtnParams;
        }
        // EXP_PULL_END

        _appendIMethodResponseElementEnd(out);
        _appendSimpleRspElementEnd(out);
        _appendMessageElementEnd(out);
    }

    return out;
}


//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleIMethodErrorRspMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleIMethodErrorRspMessage(
    const CIMName& iMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const CIMException& cimException)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleRspElementBegin(out);
    _appendIMethodResponseElementBegin(out, iMethodName);
    _appendErrorElement(out, cimException);
    _appendIMethodResponseElementEnd(out);
    _appendSimpleRspElementEnd(out);
    _appendMessageElementEnd(out);

    appendMethodResponseHeader(tmp,
        httpMethod,
        cimException.getContentLanguages(),
        out.size(), false);
    tmp << out;

    return tmp;
}

//******************************************************************************
//
// Export Messages (used for indications)
//
//******************************************************************************

//------------------------------------------------------------------------------
//
// appendEMethodRequestHeader()
//
//     Build HTTP request header for export operation.
//
//------------------------------------------------------------------------------

void XmlWriter::appendEMethodRequestHeader(
    Buffer& out,
    const char* requestUri,
    const char* host,
    const CIMName& cimMethod,
    HttpMethod httpMethod,
    const String& authenticationHeader,
    const AcceptLanguageList& acceptLanguages,
    const ContentLanguageList& contentLanguages,
    Uint32 contentLength)
{
    char nn[] = { char('0' + (rand() % 10)), char('0' + (rand() % 10)), '\0' };

    if (httpMethod == HTTP_METHOD_M_POST)
    {
      out << STRLIT("M-POST ") << requestUri << STRLIT(" HTTP/1.1\r\n");
    }
    else
    {
      out << STRLIT("POST ") << requestUri << STRLIT(" HTTP/1.1\r\n");
    }
    out << STRLIT("HOST: ") << host << STRLIT("\r\n"
                  "Content-Type: application/xml; charset=utf-8\r\n");
    OUTPUT_CONTENTLENGTH(out, contentLength);

    if (acceptLanguages.size() > 0)
    {
        out << STRLIT("Accept-Language: ") << acceptLanguages << STRLIT("\r\n");
    }
    if (contentLanguages.size() > 0)
    {
        out << STRLIT("Content-Language: ") << contentLanguages <<
            STRLIT("\r\n");
    }

#ifdef PEGASUS_DEBUG
    // backdoor environment variable to turn OFF client requesting transfer
    // encoding. The default is on. to turn off, set this variable to zero.
    // This should be removed when stable. This should only be turned off in
    // a debugging/testing environment.

    static const char *clientTransferEncodingOff =
        getenv("PEGASUS_HTTP_TRANSFER_ENCODING_REQUEST");
    if (!clientTransferEncodingOff || *clientTransferEncodingOff != '0')
#endif
    out << STRLIT("TE: chunked, trailers\r\n");

    if (httpMethod == HTTP_METHOD_M_POST)
    {
        out << STRLIT("Man: http://www.dmtf.org/cim/mapping/http/v1.0; ns=");
        out << nn << STRLIT("\r\n");
        out << nn << STRLIT("-CIMExport: MethodRequest\r\n");
        out << nn << STRLIT("-CIMExportMethod: ") << cimMethod <<
            STRLIT("\r\n");
    }
    else
    {
        out << STRLIT("CIMExport: MethodRequest\r\n"
                      "CIMExportMethod: ") << cimMethod << STRLIT("\r\n");
    }

    if (authenticationHeader.size())
    {
        out << authenticationHeader << STRLIT("\r\n");
    }

    out << STRLIT("\r\n");
}

//------------------------------------------------------------------------------
//
// appendEMethodResponseHeader()
//
//     Build HTTP response header for export operation.
//
//------------------------------------------------------------------------------

void XmlWriter::appendEMethodResponseHeader(
    Buffer& out,
    HttpMethod httpMethod,
    const ContentLanguageList& contentLanguages,
    Uint32 contentLength)
{
    char nn[] = { char('0' + (rand() % 10)), char('0' + (rand() % 10)), '\0' };

    out << STRLIT("HTTP/1.1 " HTTP_STATUS_OK "\r\n"
                  "Content-Type: application/xml; charset=utf-8\r\n");
    OUTPUT_CONTENTLENGTH(out, contentLength);

    if (contentLanguages.size() > 0)
    {
        out << STRLIT("Content-Language: ") << contentLanguages <<
            STRLIT("\r\n");
    }
    if (httpMethod == HTTP_METHOD_M_POST)
    {
        out << STRLIT("Ext:\r\n"
                      "Cache-Control: no-cache\r\n"
                      "Man: http://www.dmtf.org/cim/mapping/http/v1.0; ns=");
        out << nn << STRLIT("\r\n");
        out << nn << STRLIT("-CIMExport: MethodResponse\r\n\r\n");
    }
    else
    {
        out << STRLIT("CIMExport: MethodResponse\r\n\r\n");
    }
}

//------------------------------------------------------------------------------
//
// _appendSimpleExportReqElementBegin()
// _appendSimpleExportReqElementEnd()
//
//     <!ELEMENT SIMPLEEXPREQ (EXPMETHODCALL)>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendSimpleExportReqElementBegin(
    Buffer& out)
{
    out << STRLIT("<SIMPLEEXPREQ>\n");
}

void XmlWriter::_appendSimpleExportReqElementEnd(
    Buffer& out)
{
    out << STRLIT("</SIMPLEEXPREQ>\n");
}

//------------------------------------------------------------------------------
//
// _appendEMethodCallElementBegin()
// _appendEMethodCallElementEnd()
//
//     <!ELEMENT EXPMETHODCALL (IPARAMVALUE*)>
//     <!ATTLIST EXPMETHODCALL %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendEMethodCallElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<EXPMETHODCALL NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendEMethodCallElementEnd(
    Buffer& out)
{
    out << STRLIT("</EXPMETHODCALL>\n");
}

//------------------------------------------------------------------------------
//
// _appendEParamValueElementBegin()
// _appendEParamValueElementEnd()
//
//     <!ELEMENT EXPPARAMVALUE (INSTANCE)>
//     <!ATTLIST EXPPARAMVALUE
//         %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendEParamValueElementBegin(
    Buffer& out,
    const char* name)
{
    out << STRLIT("<EXPPARAMVALUE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendEParamValueElementEnd(
    Buffer& out)
{
    out << STRLIT("</EXPPARAMVALUE>\n");
}

//------------------------------------------------------------------------------
//
// appendInstanceEParameter()
//
//------------------------------------------------------------------------------

void XmlWriter::appendInstanceEParameter(
    Buffer& out,
    const char* name,
    const CIMInstance& instance)
{
    _appendEParamValueElementBegin(out, name);
    appendInstanceElement(out, instance);
    _appendEParamValueElementEnd(out);
}

//------------------------------------------------------------------------------
//
// _appendSimpleExportRspElementBegin()
// _appendSimpleExportRspElementEnd()
//
//     <!ELEMENT SIMPLEEXPRSP (EXPMETHODRESPONSE)>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendSimpleExportRspElementBegin(
    Buffer& out)
{
    out << STRLIT("<SIMPLEEXPRSP>\n");
}

void XmlWriter::_appendSimpleExportRspElementEnd(
    Buffer& out)
{
    out << STRLIT("</SIMPLEEXPRSP>\n");
}

//------------------------------------------------------------------------------
//
// _appendEMethodResponseElementBegin()
// _appendEMethodResponseElementEnd()
//
//     <!ELEMENT EXPMETHODRESPONSE (ERROR|IRETURNVALUE?)>
//     <!ATTLIST EXPMETHODRESPONSE %CIMName;>
//
//------------------------------------------------------------------------------

void XmlWriter::_appendEMethodResponseElementBegin(
    Buffer& out,
    const CIMName& name)
{
    out << STRLIT("<EXPMETHODRESPONSE NAME=\"") << name << STRLIT("\">\n");
}

void XmlWriter::_appendEMethodResponseElementEnd(
    Buffer& out)
{
    out << STRLIT("</EXPMETHODRESPONSE>\n");
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleEMethodReqMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleEMethodReqMessage(
    const char* requestUri,
    const char* host,
    const CIMName& eMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const String& authenticationHeader,
    const AcceptLanguageList& httpAcceptLanguages,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& body)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleExportReqElementBegin(out);
    _appendEMethodCallElementBegin(out, eMethodName);
    out << body;
    _appendEMethodCallElementEnd(out);
    _appendSimpleExportReqElementEnd(out);
    _appendMessageElementEnd(out);

    appendEMethodRequestHeader(
        tmp,
        requestUri,
        host,
        eMethodName,
        httpMethod,
        authenticationHeader,
        httpAcceptLanguages,
        httpContentLanguages,
        out.size());
    tmp << out;

    return tmp;
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleEMethodRspMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleEMethodRspMessage(
    const CIMName& eMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const ContentLanguageList& httpContentLanguages,
    const Buffer& body)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleExportRspElementBegin(out);
    _appendEMethodResponseElementBegin(out, eMethodName);
    out << body;
    _appendEMethodResponseElementEnd(out);
    _appendSimpleExportRspElementEnd(out);
    _appendMessageElementEnd(out);

    appendEMethodResponseHeader(tmp,
        httpMethod,
        httpContentLanguages,
        out.size());
    tmp << out;

    return tmp;
}

//------------------------------------------------------------------------------
//
// XmlWriter::formatSimpleEMethodErrorRspMessage()
//
//------------------------------------------------------------------------------

Buffer XmlWriter::formatSimpleEMethodErrorRspMessage(
    const CIMName& eMethodName,
    const String& messageId,
    HttpMethod httpMethod,
    const CIMException& cimException)
{
    Buffer out;
    Buffer tmp;

    _appendMessageElementBegin(out, messageId);
    _appendSimpleExportRspElementBegin(out);
    _appendEMethodResponseElementBegin(out, eMethodName);
    _appendErrorElement(out, cimException);
    _appendEMethodResponseElementEnd(out);
    _appendSimpleExportRspElementEnd(out);
    _appendMessageElementEnd(out);

    appendEMethodResponseHeader(
        tmp,
        httpMethod,
        cimException.getContentLanguages(),
        out.size());
    tmp << out;

    return tmp;
}

//------------------------------------------------------------------------------
//
// XmlWriter::getNextMessageId()
//
//------------------------------------------------------------------------------

static IDFactory _messageIDFactory(1000);

String XmlWriter::getNextMessageId()
{
    char scratchBuffer[22];
    Uint32 n;
    const char * startP = Uint32ToString(scratchBuffer,
                                         _messageIDFactory.getID(),
                                         n);
    return String(startP, n);
}

//------------------------------------------------------------------------------
//
// XmlWriter::keyBindingTypeToString
//
//------------------------------------------------------------------------------
const StrLit XmlWriter::keyBindingTypeToString (CIMKeyBinding::Type type)
{
    switch (type)
    {
        case CIMKeyBinding::BOOLEAN:
            return STRLIT("boolean");

        case CIMKeyBinding::STRING:
            return STRLIT("string");

        case CIMKeyBinding::NUMERIC:
            return STRLIT("numeric");

        case CIMKeyBinding::REFERENCE:
        default:
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
    }

    return STRLIT("unknown");
}

PEGASUS_NAMESPACE_END
