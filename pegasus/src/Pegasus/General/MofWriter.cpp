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
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMClassRep.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMInstanceRep.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMPropertyRep.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMMethodRep.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMParameterRep.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/CIMQualifierRep.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/CIMQualifierDeclRep.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMFlavor.h>
#include <Pegasus/Common/CIMScope.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include "MofWriter.h"


PEGASUS_NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//
// Helper functions for appendValueElement()
//
//------------------------------------------------------------------------------

inline void _mofWriter_appendValue(Buffer& out, Boolean x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Uint8 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _mofWriter_appendValue(Buffer& out, Sint8 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _mofWriter_appendValue(Buffer& out, Uint16 x)
{
    XmlWriter::append(out, Uint32(x));
}

inline void _mofWriter_appendValue(Buffer& out, Sint16 x)
{
    XmlWriter::append(out, Sint32(x));
}

inline void _mofWriter_appendValue(Buffer& out, Uint32 x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Sint32 x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Uint64 x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Sint64 x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Real32 x)
{
    XmlWriter::append(out, Real64(x));
}

inline void _mofWriter_appendValue(Buffer& out, Real64 x)
{
    XmlWriter::append(out, x);
}

inline void _mofWriter_appendValue(Buffer& out, Char16 x)
{
    XmlWriter::appendSpecial(out, x);
}

/** Convert the string back to MOF format and output it.
    The conversions are:
    \b // \x0008: backspace BS
    \t // \x0009: horizontal tab HT
    \n // \x000A: linefeed LF
    \f // \x000C: form feed FF
    \r // \x000D: carriage return CR
    \" // \x0022: double quote "
    \' // \x0027: single quote '
    \\ // \x005C: backslash \
    \x<hex> // where <hex> is one to four hex digits
    \X<hex> // where <hex> is one to four hex digits
*/
/* ATTN:KS - We need to account for characters greater than x'7f
*/
inline void _mofWriter_appendValue(Buffer& out, const String& x)
{
    out << "\"";
    const Uint32 size = x.size();
    for (Uint32 i = 0; i < size; i++)
    {
        switch (x[i])
        {
        case '\\':
                out.append("\\\\",2);
                break;

            case '\b':
                out.append("\\b",2);
                break;

            case '\t':
                out.append("\\t",2);
                break;

            case '\n':
                out.append("\\n",2);
                break;

            case '\f':
                out.append("\\f",2);
                break;

            case '\r':
                out.append("\\r",2);
                break;

           case '\'':
                out.append("\\'", 2);
                break;

            case '"':
                out.append("\\\"", 2);
                break;

            default:
                out<<x[i];
        }

    }
    out << "\"";
}

inline void _mofWriter_appendValue(Buffer& out, const CIMDateTime& x)
{
    _mofWriter_appendValue(out, x.toString());
}

inline void _mofWriter_appendValue(Buffer& out, const CIMObjectPath& x)
{
    _mofWriter_appendValue(out, x.toString());
}

inline void _mofWriter_appendValue(Buffer& out, const CIMObject& x)
{
    if(x.isClass())
    {
        CIMConstClass y = (CIMConstClass)x;
        MofWriter::appendClassElement(out, y);
    }
    else
    {
        MofWriter::appendInstanceElement(out, (CIMInstance)x);
    }
}

inline void _mofWriter_appendValue(Buffer& out, const CIMInstance& x)
{
    MofWriter::appendInstanceElement(out, x);
}

/** Array -
    arrayInitializer = "{" constantValue*( "," constantValue)"}"
*/
template<class T>

void _mofWriter_appendValueArrayMof(Buffer& out, const T* p, Uint32 size)
{
    bool isFirstEntry = true;
    // if there are any entries in the array output them
    if (size)
    {
        out << "{";
        while (size--)
        {
            // Put comma on all but first entry.
            if (!isFirstEntry)
            {
                out << ", ";
            }
            isFirstEntry = false;
            _mofWriter_appendValue(out, *p++);
        }
        out << "}";
    }
}

//------------------------------------------------------------------------------
//
// appendValueElement()
//
//------------------------------------------------------------------------------

void MofWriter::appendValueElement(
    Buffer& out,
    const CIMValue& value)
{
    // if the CIMValue is Null we return the Null indicator.
    if (value.isNull())
    {
        out << "null";
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
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT8:
            {
                Array<Uint8> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT8:
            {
                Array<Sint8> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT16:
            {
                Array<Uint16> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT16:
            {
                Array<Sint16> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT32:
            {
                Array<Uint32> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT32:
            {
                Array<Sint32> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_UINT64:
            {
                Array<Uint64> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_SINT64:
            {
                Array<Sint64> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL32:
            {
                Array<Real32> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REAL64:
            {
                Array<Real64> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_CHAR16:
            {
                Array<Char16> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_STRING:
            {
                Array<String> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_DATETIME:
            {
                Array<CIMDateTime> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                Array<CIMObjectPath> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }

            case CIMTYPE_OBJECT:
            {
                Array<CIMObject> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                Array<CIMInstance> a;
                value.get(a);
                _mofWriter_appendValueArrayMof(out, a.getData(), a.size());
                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        }
    }
    else
    {
        switch (value.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT8:
            {
                Uint8 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT8:
            {
                Sint8 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT16:
            {
                Uint16 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT16:
            {
                Sint16 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT32:
            {
                Uint32 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT32:
            {
                Sint32 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_UINT64:
            {
                Uint64 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_SINT64:
            {
                Sint64 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_REAL32:
            {
                Real32 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_REAL64:
            {
                Real64 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_CHAR16:
            {
                Char16 v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_STRING:
            {
                String v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_DATETIME:
            {
                CIMDateTime v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_REFERENCE:
            {
                CIMObjectPath v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }

            case CIMTYPE_OBJECT:
            {
                CIMObject v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                CIMInstance v;
                value.get(v);
                _mofWriter_appendValue(out, v);
                break;
            }
            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
        }
    }
}

//------------------------------------------------------------------------------
//
// appendValueReferenceElement()
//
//------------------------------------------------------------------------------

void MofWriter::appendValueReferenceElement(
    Buffer& out,
    const CIMObjectPath& reference)
{
    _mofWriter_appendValue(out, reference);
}

//------------------------------------------------------------------------------
//
// appendClassElement()
//
//    classDeclaration    =    [ qualifierList ]
//                             CLASS className [ alias ] [ superClass ]
//                             "{" *classFeature "}" ";"
//
//    superClass          =    ":" className
//
//    classFeature        =    propertyDeclaration | methodDeclaration
//
//------------------------------------------------------------------------------

void MofWriter::appendClassElement(
    Buffer& out,
    const CIMConstClass& cimClass)
{
    CheckRep(cimClass._rep);
    const CIMClassRep* rep = cimClass._rep;

    // Get and format the class qualifiers
    out << STRLIT("\n// ===================================================");
    out << STRLIT("\n// ") << rep->getClassName();
    out << STRLIT("\n// ===================================================\n");

    if (rep->getQualifierCount())
    {
        out.append('[');
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        {
            if (i > 0)
            {
                out << STRLIT(", \n");
            }
            MofWriter::appendQualifierElement(out, rep->getQualifier(i));
        }
        out << STRLIT("]\n");
    }
    else
    {
        out.append('\n');
    }

    // output class statement with new line.
    out << STRLIT("class ") << rep->getClassName();

    if (!rep->getSuperClassName().isNull())
    {
        out << STRLIT(" : ") << rep->getSuperClassName();
    }

    out << STRLIT("\n{");

    // format the Properties:
    for (Uint32 i = 0, n = rep->getPropertyCount(); i < n; i++)
    {
        // Generate MOF if this property not propagated
        // Note that the test is required only because
        // there is an error in getclass that does not
        // test the localOnly flag
        // The initial "false" indicates to format as property declaration.
        if (!rep->getProperty(i).getPropagated())
        {
            MofWriter::appendPropertyElement(true, out, rep->getProperty(i));
        }
    }

    // Format the Methods:  for non-propagated methods
    for (Uint32 i = 0, n = rep->getMethodCount(); i < n; i++)
    {
        if (!rep->getMethod(i).getPropagated())
        {
            MofWriter::appendMethodElement(out, rep->getMethod(i));
        }
    }

    // Class closing element:
    out << STRLIT("\n};\n");
}

void MofWriter::printClassElement(
    const CIMConstClass& cimclass,
    PEGASUS_STD(ostream)& os)
{
    Buffer tmp;
    appendClassElement(tmp, cimclass);
    os << tmp.getData() << PEGASUS_STD(endl);
}

//------------------------------------------------------------------------------
//
// appendInstanceElement()
//
//------------------------------------------------------------------------------

void MofWriter::appendInstanceElement(
    Buffer& out,
    const CIMConstInstance& instance)
{
    CheckRep(instance._rep);
    const CIMInstanceRep* rep = instance._rep;

    // Get and format the class qualifiers

    if (rep->getQualifierCount())
    {
        out.append('\n');
        out.append('[');
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        {
            if (i > 0)
            {
                out << STRLIT(", \n");
            }
            MofWriter::appendQualifierElement(out, rep->getQualifier(i));
        }
        out.append(']');
    }

    // Separate qualifiers from Class Name
    out.append('\n');

    // output class statement
    out << STRLIT("instance of ") << rep->getClassName();

    out << STRLIT("\n{");

    // format the Properties:
    for (Uint32 i = 0, n = rep->getPropertyCount(); i < n; i++)
    {
        // Generate MOF if this property not propagated
        // Note that the test is required only because
        // there is an error in getclass that does not
        // test the localOnly flag.
        // The false identifies this as value initializer, not
        // property definition.
        if (!rep->getProperty(i).getPropagated())
        {
            MofWriter::appendPropertyElement(false, out, rep->getProperty(i));
        }
    }

    // Class closing element:
    out << STRLIT("\n};\n");
}

//------------------------------------------------------------------------------
//
// appendPropertyElement()
//
//    The MOF for property declaration in a class and value presentation in
//    an instance are different.
//
//    The BNF for the property Declaration MOF is:
//
//        propertyDeclaration     =   [ qualifierList ] dataType propertyName
//                                    [ array ] [ defaultValue ] ";"
//
//        array                   =   "[" [positiveDecimalValue] "]"
//
//        defaultValue            =   "=" initializer
//
//    Format with qualifiers on one line and declaration on another. Start
//    with newline but none at the end.
//
//    Note that instances have a different format that propertyDeclarations:
//    instanceDeclaration = [ qualifiersList ] INSTANCE OF className | alias
//         "["valueInitializer "]" ";"
//    valueInitializer = [ qualifierList ] [ propertyName | referenceName ] "="
//                       initializer ";"
//
//------------------------------------------------------------------------------

void MofWriter::appendPropertyElement(
    bool isClassDeclaration,
    Buffer& out,
    const CIMConstProperty& property)
{
    CheckRep(property._rep);
    const CIMPropertyRep* rep = property._rep;

    //Output the qualifier list
    if (rep->getQualifierCount())
    {
        out.append('\n');
        out.append('[');
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        {
            if (i > 0)
            {
                out << STRLIT(", \n");
            }
            MofWriter::appendQualifierElement(out, rep->getQualifier(i));
        }
        out.append(']');
    }

    // Output the Type and name on a new line
    out << '\n';
    if (isClassDeclaration)
    {
        if (rep->getValue().getType() == CIMTYPE_REFERENCE)
        {
            out << rep->getReferenceClassName().getString();
            out << " REF";

        }
        else
        {
            out << cimTypeToString(rep->getValue().getType());
        }

        out.append(' ');
    }
    out << rep->getName();

    // If array put the Array indicator "[]" and possible size after name.
    if (isClassDeclaration)
    {
        if (rep->getValue().isArray())
        {
            if (rep->getArraySize())
            {
                char buffer[32];
                int n = sprintf(buffer, "[%u]", rep->getArraySize());
                out.append(buffer, n);
            }
            else
                out << STRLIT("[]");
        }
    }

    // If the property value is not Null, add value after "="
    if (!rep->getValue().isNull())
    {
        out << STRLIT(" = ");
        MofWriter::appendValueElement(out, rep->getValue());
    }
    else if (!isClassDeclaration)
    {
        out << STRLIT(" = NULL");
    }

    // Close the property MOF.
    // Do not add closing ; if instance or object.  The embedded code already
    // did that.
    if (property.getType() != CIMTYPE_INSTANCE &&
         property.getType() != CIMTYPE_OBJECT)
    {
        out.append(';');
    }
}

//------------------------------------------------------------------------------
//
// appendMethodElement()
//
//    methodDeclaration   =  [ qualifierList ] dataType methodName
//                           "(" [ parameterList ] ")" ";"
//
//    parameterList       =  parameter *( "," parameter )
//    Format with qualifiers on one line and declaration on another. Start
//    with newline but none at the end.
//
//------------------------------------------------------------------------------

void MofWriter::appendMethodElement(
    Buffer& out,
    const CIMConstMethod& method)
{
    CheckRep(method._rep);
    const CIMMethodRep* rep = method._rep;

    // Output the qualifier list starting on new line
    if (rep->getQualifierCount())
    {
        out.append('\n');
        out.append('[');
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        {
            if (i > 0)
            {
                out << STRLIT(", \n");
            }
            MofWriter::appendQualifierElement(out, rep->getQualifier(i));
        }
        out.append(']');
    }

    // output the type, MethodName and ParameterList, left enclosure
    out.append('\n');
    out << cimTypeToString(rep->getType());
    out.append(' ');
    out << rep->getName();
    out.append('(');

    // output the param list separated by commas.

    for (Uint32 i = 0, n = rep->getParameterCount(); i < n; i++)
    {
        // If not first, output comma separator
        if (i)
        {
            out << STRLIT(", ");
        }

        MofWriter::appendParameterElement(out, rep->getParameter(i));
    }

    // output the parameterlist and method terminator
    out << STRLIT(");");
}

//------------------------------------------------------------------------------
//
// appendParameterElement()
//
//    parameterList    =  parameter *( "," parameter )
//
//        parameter    =  [ qualifierList ] (dataType|objectRef) parameterName
//                        [ array ]
//
//        parameterName=  IDENTIFIER
//
//        array        =  "[" [positiveDecimalValue] "]"
//
//    Format on a single line.
//
//------------------------------------------------------------------------------

void MofWriter::appendParameterElement(
    Buffer& out,
    const CIMConstParameter& parameter)
{
    CheckRep(parameter._rep);
    const CIMParameterRep* rep = parameter._rep;

    // Output the qualifiers for the parameter
    if (rep->getQualifierCount())
    {
        out.append('[');
        for (Uint32 i = 0, n = rep->getQualifierCount(); i < n; i++)
        {
            if (i > 0)
            {
                out << STRLIT(", \n");
            }
            MofWriter::appendQualifierElement(out, rep->getQualifier(i));
        }
        out.append(']');
    }

    if (rep->getQualifierCount())
    {
        out.append(' ');
    }

    // Output the data type and name
    if (rep->getType() == CIMTYPE_REFERENCE)
    {
        out << rep->getReferenceClassName().getString();
        out << " REF";
    }
    else
    {
        out << cimTypeToString(rep->getType());
    }

    out.append(' ');
    out << rep->getName();

    if (rep->isArray())
    {
        //Output the array indicator "[ [arraysize] ]"
        if (rep->getArraySize())
        {
            char buffer[32];
            int n = sprintf(buffer, "[%u]", rep->getArraySize());
            out.append(buffer, n);
        }
        else
            out << STRLIT("[]");
    }
}

//------------------------------------------------------------------------------
//
// appendQualifierElement()
//
//    qualifier          = qualifierName [ qualifierParameter ] [ ":" 1*flavor]
//    Effective DSP 0004 V2.6 the ABNF rule [ ":" 1*flavor ] is deprecated
//    Removed flavor inserting code
//    because is was used for the concept of implicityly defined qualifier
//    types.
//    qualifierParameter = "(" constantValue ")" | arrayInitializer
//
//    arrayInitializer   = "{" constantValue*( "," constantValue)"}"
//
//------------------------------------------------------------------------------

void MofWriter::appendQualifierElement(
    Buffer& out,
    const CIMConstQualifier& qualifier)
{
    CheckRep(qualifier._rep);
    const CIMQualifierRep* rep = qualifier._rep;

    // Output Qualifier name
    out << rep->getName();

    /* If the qualifier is Boolean, we do not put out a value. This is
       the way MOF is shown.  Note that we should really be checking
       the qualifierdecl to compare with the default.
       Also if the value is Null, we do not put out a value because
       no value has been set.
    */
    if (!rep->getValue().isNull())
    {
        if (rep->getValue().getType() == CIMTYPE_BOOLEAN)
        {
            Boolean b;
            rep->getValue().get(b);
            if (!b)
            {
                out << STRLIT(" (false)");
            }
        }
        else
        {
            if (!rep->getValue().isArray())
            {
                out << STRLIT(" ( ");
            }
            else
            {
                out << STRLIT(" ");
            }

            MofWriter::appendValueElement(out, rep->getValue());

            if (!rep->getValue().isArray())
            {
                out << STRLIT(" )");
            }
        }
        //    Effective DSP 004 V2.6 the ABNF rule [ ":" 1*flavor ] deprecated
        //    so code that attached flavor information removed
    }
}

//------------------------------------------------------------------------------
//
// appendQualifierDeclElement()
//
//    qualifierDeclaration   =    QUALIFIER qualifierName qualifierType scope
//                                [ defaultFlavor ] ";"
//
//    qualifierName          =    IDENTIFIER
//
//    qualifierType          =    ":" dataType [ array ] [ defaultValue ]
//
//    scope                  =    "," SCOPE
//                                "(" metaElement *( "," metaElement ) ")"
//
//------------------------------------------------------------------------------

void MofWriter::appendQualifierDeclElement(
    Buffer& out,
    const CIMConstQualifierDecl& qualifierDecl)
{
    CheckRep(qualifierDecl._rep);
    const CIMQualifierDeclRep* rep = qualifierDecl._rep;

    out.append('\n');

    // output the "Qualifier" keyword and name
    out << STRLIT("Qualifier ") << rep->getName();

    // output the qualifiertype
    out << STRLIT(" : ") << cimTypeToString(rep->getValue().getType());

    // If array put the Array indicator "[]" and possible size after name.
    if (rep->getValue().isArray())
    {
        if (rep->getArraySize())
        {
            char buffer[32];
            int n = sprintf(buffer, "[%u]", rep->getArraySize());
            out.append(buffer, n);
        }
        else
        {
            out << STRLIT("[]");
        }
        if (!rep->getValue().isNull())
        {
            out << STRLIT(" = ");
            MofWriter::appendValueElement(out, rep->getValue());
        }
    }
    else
    {
        out << STRLIT(" = ");
        MofWriter::appendValueElement(out, rep->getValue());
    }

    // Output Scope Information
    String scopeString = MofWriter::getQualifierScope(rep->getScope());
    if (scopeString.size())
    {
        out << STRLIT(", Scope(") << scopeString << STRLIT(")");
    }
    // Output Flavor Information
    String flavorString = MofWriter::getQualifierFlavor(rep->getFlavor());
    if (flavorString.size())
    {
        out << STRLIT(", Flavor(") << flavorString << STRLIT(")");
    }

    // End each qualifier declaration with newline
    out << STRLIT(";\n");
}

//------------------------------------------------------------------------------
//
// getQualifierFlavor()
//
// Convert the Qualifier flavors to a string of MOF flavor keywords.
//
//   <pre>
//   Keyword            Function                             Default
//     EnableOverride  Qualifier is overridable.               yes
//     DisableOverride Qualifier cannot be overridden.         no
//     ToSubclass      Qualifier is inherited by any subclass. yes
//     Restricted      Qualifier applies only to the class     no
//                     in which it is declared
//     Translatable    Indicates the value of the qualifier
//                     can be specified in multiple languages  no
//     NOTE: There is an open issue with the keyword toinstance.
//
//     flavor            = ENABLEOVERRIDE | DISABLEOVERRIDE | RESTRICTED |
//                         TOSUBCLASS | TRANSLATABLE
//     DISABLEOVERRIDE   = "disableoverride"
//
//     ENABLEOVERRIDE    = "enableoverride"
//
//     RESTRICTED        = "restricted"
//
//     TOSUBCLASS        = "tosubclass"
//
//     TRANSLATABLE      = "translatable"
//    </pre>
//
//    The keyword toinstance is not in the CIM specification. For the moment
//    we are assuming that it is the same as the toSubclass. We had a choice
//    of using one entity for both or separating them and letting the
//    compiler set both.
//
//------------------------------------------------------------------------------

String MofWriter::getQualifierFlavor(const CIMFlavor & flavor)
{
    String flavorStr;

    // special case. Default flavors only, return nothing.
    if (flavor.equal(CIMFlavor(CIMFlavor::DEFAULTS)))
    {
        return flavorStr;
    }

    if ((flavor.hasFlavor (CIMFlavor::ENABLEOVERRIDE)))
    {
        flavorStr.append("EnableOverride, ");
    }

    if (((flavor.hasFlavor (CIMFlavor::DISABLEOVERRIDE))))
    {
        flavorStr.append("DisableOverride, ");
    }

    if ((flavor.hasFlavor (CIMFlavor::TOSUBCLASS)))
    {
        flavorStr.append("ToSubclass, ");
    }

    if (flavor.hasFlavor (CIMFlavor::RESTRICTED))
    {
        flavorStr.append("Restricted, ");
    }

    if (flavor.hasFlavor (CIMFlavor::TRANSLATABLE))
    {
        flavorStr.append("Translatable, ");
    }

    if (flavor.hasFlavor (CIMFlavor::TOINSTANCE))
    {
        flavorStr.append("ToInstance, ");
    }

    if (flavorStr.size())
    {
        flavorStr.remove(flavorStr.size() - 2);
    }

    return flavorStr;
}

//------------------------------------------------------------------------------
//
// getQualifierScope()
//
//------------------------------------------------------------------------------

String MofWriter::getQualifierScope (const CIMScope & scope)
{
    if (scope.equal (CIMScope::ANY))
    {
        return "any";
    }
    else
    {
        String tmp = scope.toString ();
        tmp.toLower();
        return tmp;
    }
}

PEGASUS_NAMESPACE_END
