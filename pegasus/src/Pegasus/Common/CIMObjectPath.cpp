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
#include <cstring>
#include <iostream>
#include "HashTable.h"
#include "CIMObjectPath.h"
#include "CIMName.h"
#include "CIMValue.h"
#include "XmlReader.h"
#include <Pegasus/Common/StringConversion.h>
#include "ArrayInternal.h"
#include "HostLocator.h"
#include "System.h"
#include "CIMKeyBindingRep.h"
#include "CIMObjectPathRep.h"

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T CIMKeyBinding
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T CIMObjectPath
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

// ATTN: KS May 2002 P0 Add resolve method to CIMObjectPath.
// Add a resolve method to this class to verify that the
// reference is correct (that the class name corresponds to a real
// class and that the property names are really keys and that all keys
// of the class or used. Also be sure that there is a valid conversion
// between the string value and the value of that property).
//
// ATTN: also check to see that the reference refers to a class that is the
// same or derived from the _className member.

////////////////////////////////////////////////////////////////////////////////
//
// Local routines:
//
////////////////////////////////////////////////////////////////////////////////

static String _escapeSpecialCharacters(const String& str)
{
    String result;

    for (Uint32 i = 0, n = str.size(); i < n; i++)
    {
        switch (str[i])
        {
            case '\\':
                result.append("\\\\");
                break;

            case '"':
                result.append("\\\"");
                break;

            default:
                result.append(str[i]);
        }
    }

    return result;
}

static int _compare(const void* p1, const void* p2)
{
    const CIMKeyBinding* kb1 = (const CIMKeyBinding*)p1;
    const CIMKeyBinding* kb2 = (const CIMKeyBinding*)p2;

    return String::compareNoCase(
        kb1->getName().getString(),
        kb2->getName().getString());
}

static void _Sort(Array<CIMKeyBinding>& x)
{
    CIMKeyBinding* data = (CIMKeyBinding*)x.getData();
    Uint32 size = x.size();

    //
    //  If the key is a reference, the keys in the reference must also be
    //  sorted
    //
    for (Uint32 k = 0; k < size; k++)
    {
        CIMKeyBinding& kb = data[k];

        if (kb.getType() == CIMKeyBinding::REFERENCE)
        {
            CIMObjectPath tmp(kb.getValue());
            Array<CIMKeyBinding> keyBindings = tmp.getKeyBindings();
            _Sort(keyBindings);
            tmp.setKeyBindings(keyBindings);
            kb.setValue(tmp.toString());
        }
    }

    if (size < 2)
        return;

    qsort((void*)data, size, sizeof(CIMKeyBinding), _compare);
}

////////////////////////////////////////////////////////////////////////////////
//
// CIMKeyBinding
//
////////////////////////////////////////////////////////////////////////////////

CIMKeyBinding::CIMKeyBinding()
{
    _rep = new CIMKeyBindingRep();
}

CIMKeyBinding::CIMKeyBinding(const CIMKeyBinding& x)
{
    _rep = new CIMKeyBindingRep(*x._rep);
}

CIMKeyBinding::CIMKeyBinding(
    const CIMName& name,
    const String& value,
    Type type)
{
    _rep = new CIMKeyBindingRep(name, value, type);
}

CIMKeyBinding::CIMKeyBinding(const CIMName& name, const CIMValue& value)
{
    if (value.isArray())
    {
        throw TypeMismatchException();
    }

    String kbValue = value.toString();
    Type kbType;

    switch (value.getType())
    {
    case CIMTYPE_BOOLEAN:
        kbType = BOOLEAN;
        break;
    case CIMTYPE_CHAR16:
    case CIMTYPE_STRING:
    case CIMTYPE_DATETIME:
        kbType = STRING;
        break;
    case CIMTYPE_REFERENCE:
        kbType = REFERENCE;
        break;
//  case CIMTYPE_REAL32:
//  case CIMTYPE_REAL64:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        // From PEP 194: EmbeddedObjects cannot be keys.
        throw TypeMismatchException();
        break;
    default:
        kbType = NUMERIC;
        break;
    }

    _rep = new CIMKeyBindingRep(name, kbValue, kbType);
}

CIMKeyBinding::~CIMKeyBinding()
{
    delete _rep;
}

CIMKeyBinding& CIMKeyBinding::operator=(const CIMKeyBinding& x)
{
    *_rep = *x._rep;
    return *this;
}

const CIMName& CIMKeyBinding::getName() const
{
    return _rep->_name;
}

void CIMKeyBinding::setName(const CIMName& name)
{
    _rep->_name = name;
}

const String& CIMKeyBinding::getValue() const
{
    return _rep->_value;
}

void CIMKeyBinding::setValue(const String& value)
{
    _rep->_value = value;
}

CIMKeyBinding::Type CIMKeyBinding::getType() const
{
    return _rep->_type;
}

void CIMKeyBinding::setType(CIMKeyBinding::Type type)
{
    _rep->_type = type;
}

Boolean CIMKeyBinding::equal(CIMValue value)
{
    if (value.isArray())
    {
        return false;
    }

    CIMValue kbValue;

    try
    {
        switch (value.getType())
        {
        case CIMTYPE_CHAR16:
            if (getType() != STRING) return false;
            kbValue.set(getValue()[0]);
            break;
        case CIMTYPE_DATETIME:
            if (getType() != STRING) return false;
            kbValue.set(CIMDateTime(getValue()));
            break;
        case CIMTYPE_STRING:
            if (getType() != STRING) return false;
            kbValue.set(getValue());
            break;
        case CIMTYPE_REFERENCE:
            if (getType() != REFERENCE) return false;
            kbValue.set(CIMObjectPath(getValue()));
            break;
        case CIMTYPE_BOOLEAN:
            if (getType() != BOOLEAN) return false;
            kbValue = XmlReader::stringToValue(0, getValue().getCString(),
                                               value.getType());
            break;
//      case CIMTYPE_REAL32:
//      case CIMTYPE_REAL64:
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
            // From PEP 194: EmbeddedObjects cannot be keys.
            return false;
        default:  // Numerics
            if (getType() != NUMERIC) return false;
            kbValue = XmlReader::stringToValue(0, getValue().getCString(),
                                               value.getType());
            break;
        }
    }
    catch (Exception&)
    {
        return false;
    }

    return value.equal(kbValue);
}

Boolean operator==(const CIMKeyBinding& x, const CIMKeyBinding& y)
{
    // Check that the names and types match
    if (!(x.getName().equal(y.getName())) ||
        !(x.getType() == y.getType()))
    {
        return false;
    }

    switch (x.getType())
    {
    case CIMKeyBinding::REFERENCE:
        try
        {
            // References should be compared as CIMObjectPaths
            return (CIMObjectPath(x.getValue()) == CIMObjectPath(y.getValue()));
        }
        catch (Exception&)
        {
            // If CIMObjectPath parsing fails, just compare strings
            return String::equal(x.getValue(), y.getValue());
        }
    case CIMKeyBinding::BOOLEAN:
        // Case-insensitive comparison is sufficient for booleans
        return String::equalNoCase(x.getValue(), y.getValue());
    case CIMKeyBinding::NUMERIC:
        // Note: This comparison assumes XML syntax for integers
        // First try comparing as unsigned integers
        {
            Uint64 xValue;
            Uint64 yValue;
            if (StringConversion::stringToUnsignedInteger(
                    x.getValue().getCString(), xValue) &&
                StringConversion::stringToUnsignedInteger(
                    y.getValue().getCString(), yValue))
            {
                return (xValue == yValue);
            }
        }
        // Next try comparing as signed integers
        {
            Sint64 xValue;
            Sint64 yValue;
            if (StringConversion::stringToSignedInteger(
                    x.getValue().getCString(), xValue) &&
                StringConversion::stringToSignedInteger(
                    y.getValue().getCString(), yValue))
            {
                return (xValue == yValue);
            }
        }
        // Note: Keys may not be real values, so don't try comparing as reals
        // We couldn't parse the numbers, so just compare the strings
        return String::equal(x.getValue(), y.getValue());
    default:  // CIMKeyBinding::STRING
        return String::equal(x.getValue(), y.getValue());
    }

    PEGASUS_UNREACHABLE(return false;)
}


////////////////////////////////////////////////////////////////////////////////
//
// CIMObjectPath
//
////////////////////////////////////////////////////////////////////////////////

template<class REP>
inline void Ref(REP* rep)
{
        rep->_refCounter++;
}

template<class REP>
inline void Unref(REP* rep)
{
    if (rep->_refCounter.decAndTestIfZero())
        delete rep;
}

CIMObjectPath::CIMObjectPath()
{
    _rep = new CIMObjectPathRep();
}

CIMObjectPath::CIMObjectPath(const CIMObjectPath& x)
{
    _rep = x._rep;
    Ref(_rep);
}

CIMObjectPath::CIMObjectPath(const String& objectName)
{
    // Test the objectName out to see if we get an exception
    CIMObjectPath tmpRef;
    tmpRef.set(objectName);

    _rep = tmpRef._rep;
    Ref(_rep);
}

CIMObjectPath::CIMObjectPath(
    const String& host,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const Array<CIMKeyBinding>& keyBindings)
{
    // Test the objectName out to see if we get an exception
    CIMObjectPath tmpRef;
    tmpRef.set(host, nameSpace, className, keyBindings);
    _rep = tmpRef._rep;
    Ref(_rep);
}

CIMObjectPath::~CIMObjectPath()
{
    Unref(_rep);
}

CIMObjectPath& CIMObjectPath::operator=(const CIMObjectPath& x)
{
    if (x._rep != _rep)
    {
        Unref(_rep);
        _rep = x._rep;
        Ref(_rep);
    }
    return *this;
}

static inline CIMObjectPathRep* _copyOnWriteCIMObjectPathRep(
    CIMObjectPathRep* rep)
{
    if (rep->_refCounter.get() > 1)
    {
        CIMObjectPathRep* tmpRep= new CIMObjectPathRep(*rep);
        Unref(rep);
        return tmpRep;
    }
    else
    {
        return rep;
    }
}

void CIMObjectPath::clear()
{
    // If there is more than one reference
    // remove reference and get a new shiny empty representation
    if (_rep->_refCounter.get() > 1)
    {
        Unref(_rep);
        _rep = new CIMObjectPathRep();
    }
    else
    {
        // If there is only one reference
        // no need to copy the data, we own it
        // just clear the fields
        _rep->_host.clear();
        _rep->_nameSpace.clear();
        _rep->_className.clear();
        _rep->_keyBindings.clear();
    }
}

void CIMObjectPath::set(
    const String& host,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const Array<CIMKeyBinding>& keyBindings)
{
    if ((host != String::EMPTY) && !CIMObjectPathRep::isValidHostname(host))
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.INVALID_HOSTNAME",
            "$0, reason:\"invalid hostname\"",
            host);

        throw MalformedObjectNameException(mlParms);
    }

    _rep = _copyOnWriteCIMObjectPathRep(_rep);

    _rep->_host.assign(host);
    _rep->_nameSpace = nameSpace;
    _rep->_className = className;
    _rep->_keyBindings = keyBindings;
    _Sort(_rep->_keyBindings);
}

Boolean _parseHostElement(
    const String& objectName,
    char*& p,
    String& host)
{
    // See if there is a host name (true if it begins with "//"):
    // Host is of the form <hostname>:<port> and begins with "//"
    // and ends with "/":

    if (p[0] != '/' || p[1] != '/')
    {
        return false;
    }

    p += 2;

    char* slash = strchr(p, '/');
    if (!slash)
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.MISSING_SLASH_AFTER_HOST",
            "$0, reason:\"missing slash after hostname\"",
            objectName);
        throw MalformedObjectNameException(mlParms);
    }

    String hostname = String(p, (Uint32)(slash - p));
    if (!CIMObjectPathRep::isValidHostname(hostname))
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.INVALID_HOSTNAME",
            "$0, reason:\"invalid hostname\"",
            objectName);
        throw MalformedObjectNameException(mlParms);
    }
    host = hostname;

    // Do not step past the '/'; it will be consumed by the namespace parser
    p = slash;

    return true;
}

Boolean _parseNamespaceElement(
    const String& objectName,
    char*& p,
    CIMNamespaceName& nameSpace)
{
    // If we don't find a valid namespace name followed by a ':', we
    // assume we're not looking at a namespace name.

    char* colon = strchr(p, ':');
    if (!colon)
    {
        return false;
    }

    // A ':' as part of a keybinding value should not be interpreted as
    // a namespace delimiter.  Since keybinding pairs follow the first '.'
    // in the object path string, the ':' delimiter only counts if it
    // appears before the '.'.

    char* dot = strchr(p, '.');
    if (dot && (dot < colon))
    {
        return false;
    }

    //----------------------------------------------------------------------
    // Validate the namespace path.  Namespaces must match the following
    // regular expression: "[A-Za-z_]+(/[A-Za-z_]+)*"
    //----------------------------------------------------------------------

    String namespaceName = String(p, (Uint32)(colon - p));
    if (!CIMNamespaceName::legal(namespaceName))
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.INVALID_NAMESPACE",
            "$0, reason:\"invalid namespace name\"",
            objectName);
        throw MalformedObjectNameException(mlParms);
    }
    nameSpace = namespaceName;

    p = colon+1;
    return true;
}

/**
    ATTN-RK: The DMTF specification for the string form of an
    object path makes it impossible for a parser to distinguish
    between a key values of String type and Reference type.

    Given the ambiguity, this implementation takes a guess at the
    type of a quoted key value.  If the value can be parsed into
    a CIMObjectPath with at least one key binding, the type is
    set to REFERENCE.  Otherwise, the type is set to STRING.
    Note: This algorithm appears to be in line with what the Sun
    WBEM Services implementation does.

    To be totally correct, it would be necessary to retrieve the
    class definition and look up the types of the key properties
    to determine how to interpret the key values.  This is clearly
    too inefficient for internal transformations between
    CIMObjectPaths and String values.
*/
void _parseKeyBindingPairs(
    const String& objectName,
    char*& p,
    Array<CIMKeyBinding>& keyBindings)
{
    // Get the key-value pairs:

    while (*p)
    {
        // Get key part:

        char* equalsign = strchr(p, '=');
        if (!equalsign)
        {
            MessageLoaderParms mlParms(
                "Common.CIMObjectPath.INVALID_KEYVALUEPAIR",
                "$0, reason:\"invalid key-value pair, missing equal sign\"",
                objectName);
            throw MalformedObjectNameException(mlParms);
        }

        *equalsign = 0;

        if (!CIMName::legal(p))
        {
            MessageLoaderParms mlParms(
                "Common.CIMObjectPath.INVALID_KEYNAME",
                "$0, reason:\"invalid key-value pair, invalid key name:$1\"",
                objectName,
                p);
            throw MalformedObjectNameException(mlParms);
        }

        CIMName keyName (p);

        // Get the value part:

        String valueString;
        p = equalsign + 1;
        CIMKeyBinding::Type type;

        if (*p == '"')
        {
            // Could be CIMKeyBinding::STRING or CIMKeyBinding::REFERENCE

            p++;

            Buffer keyValueUTF8(128);

            while (*p && *p != '"')
            {
                if (*p == '\\')
                {
                    p++;

                    if ((*p != '\\') && (*p != '"'))
                    {
                        MessageLoaderParms mlParms(
                            "Common.CIMObjectPath.INVALID_KEYVALUE",
                            "$0, reason:\"invalid key-value pair, "
                                "malformed value\"",
                            objectName);
                        throw MalformedObjectNameException(mlParms);
                    }
                }

                keyValueUTF8.append(*p++);
            }

            if (*p++ != '"')
            {
                MessageLoaderParms mlParms(
                    "Common.CIMObjectPath.INVALID_KEYVALUEPAIR_MISSINGQUOTE",
                    "$0, reason:\"invalid key-value pair, "
                        "missing quote in key value\"",
                    objectName);
                throw MalformedObjectNameException(mlParms);
            }

            // Convert the UTF-8 value to a UTF-16 String

            valueString.assign(
                (const char*)keyValueUTF8.getData(),
                keyValueUTF8.size());

            /*
                Guess at the type of this quoted key value.  If the value
                can be parsed into a CIMObjectPath with at least one key
                binding, the type is assumed to be a REFERENCE.  Otherwise,
                the type is set to STRING.  (See method header for details.)
             */
            type = CIMKeyBinding::STRING;

            /* Performance shortcut will check for
               equal sign instead of doing the full
               CIMObjectPath creation and exception handling
            */
            if (strchr(keyValueUTF8.getData(), '='))
            {
                // found an equal sign, high probability for a reference
                try
                {
                    CIMObjectPath testForPath(valueString);
                    if (testForPath.getKeyBindings().size() > 0)
                    {
                        // We've found a reference value!
                        type = CIMKeyBinding::REFERENCE;
                    }
                }
                catch (const Exception &)
                {
                    // Not a reference value; leave type as STRING
                }
            }
        }
        else if (toupper(*p) == 'T' || toupper(*p) == 'F')
        {
            type = CIMKeyBinding::BOOLEAN;

            char* r = p;
            Uint32 n = 0;

            while (*r && *r != ',')
            {
                *r = toupper(*r);
                r++;
                n++;
            }

            if (!(((strncmp(p, "TRUE", n) == 0) && n == 4) ||
                  ((strncmp(p, "FALSE", n) == 0) && n == 5)))
            {
                MessageLoaderParms mlParms(
                    "Common.CIMObjectPath.INVALID_BOOLVALUE",
                    "$0, reason:\"invalid key-value pair, "
                        "value should be TRUE or FALSE\"",
                    objectName);
                throw MalformedObjectNameException(mlParms);
            }

            valueString.assign(p, n);

            p = p + n;
        }
        else
        {
            type = CIMKeyBinding::NUMERIC;

            char* r = p;
            Uint32 n = 0;

            while (*r && *r != ',')
            {
                r++;
                n++;
            }

            Boolean isComma = false;
            if (*r)
            {
                *r = '\0';
                isComma = true;
            }

            if (*p == '-')
            {
                Sint64 x;
                if (!StringConversion::stringToSignedInteger(p, x))
                {
                    MessageLoaderParms mlParms(
                        "Common.CIMObjectPath.INVALID_NEGATIVNUMBER_VALUE",
                        "$0, reason:\"invalid key-value pair, "
                            "invalid negative number value $1\"",
                        objectName,
                        p);
                    throw MalformedObjectNameException(mlParms);
                }
            }
            else
            {
                Uint64 x;
                if (!StringConversion::stringToUnsignedInteger(p, x))
                {
                    MessageLoaderParms mlParms(
                        "Common.CIMObjectPath.INVALID_NEGATIVNUMBER_VALUE",
                        "$0, reason:\"invalid key-value pair, "
                            "invalid number value $1\"",
                        objectName,
                        p);
                    throw MalformedObjectNameException(mlParms);
                }
            }

            valueString.assign(p, n);

            if (isComma)
            {
                *r = ',';
            }

            p = p + n;
        }

        keyBindings.append(CIMKeyBinding(keyName.getString (), valueString,
            type));

        if (*p)
        {
            if (*p++ != ',')
            {
                MessageLoaderParms mlParms(
                    "Common.CIMObjectPath.INVALID_KEYVALUEPAIR_MISSCOMMA",
                    "$0, reason:\"invalid key-value pair, "
                        "next key-value pair has to start with comma\"",
                    objectName);
                throw MalformedObjectNameException(mlParms);
            }
        }
    }

    _Sort(keyBindings);
}

void CIMObjectPath::set(const String& objectName)
{
    // the clear automatically ensures
    // we have our own copy of the representation
    clear();

    //--------------------------------------------------------------------------
    // We will extract components from an object name. Here is an sample
    // object name:
    //
    //     //atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter"
    //--------------------------------------------------------------------------

    // Convert to a C String first:

    CString pCString = objectName.getCString();
    char* p = const_cast<char*>((const char*) pCString);
    Boolean gotHost;
    Boolean gotNamespace;

    gotHost = _parseHostElement(objectName, p, _rep->_host);
    gotNamespace = _parseNamespaceElement(objectName, p, _rep->_nameSpace);

    if (gotHost && !gotNamespace)
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.MISSING_NAMESPACE",
            "$0, reason:\"host specified, missing namespace\"",
            objectName);
        throw MalformedObjectNameException(mlParms);
    }

    // Extract the class name:

    char* dot = strchr(p, '.');

    if (!dot)
    {
        if (!CIMName::legal(p))
        {
            MessageLoaderParms mlParms(
                "Common.CIMObjectPath.INVALID_CLASSNAME",
                "$0, reason:\"class name $1 not a legal CIM name\"",
                objectName,
                p);
            throw MalformedObjectNameException(mlParms);
        }

        // ATTN: remove this later: a reference should only be able to hold
        // an instance name.

        _rep->_className = CIMName (p);
        return;
    }

    String className = String(p, (Uint32)(dot - p));
    if (!CIMName::legal(className))
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.INVALID_CLASSNAME",
            "$0, reason:\"class name $1 not a legal CIM name\"",
            objectName,
            className);
        throw MalformedObjectNameException(mlParms);
    }
    _rep->_className = className;

    // Advance past dot:

    p = dot + 1;

    _parseKeyBindingPairs(objectName, p, _rep->_keyBindings);
}

CIMObjectPath& CIMObjectPath::operator=(const String& objectName)
{
    // set will call clear, which will cause copyOnWrite if necessary
    set(objectName);
    return *this;
}

const String& CIMObjectPath::getHost() const
{
    return _rep->_host;
}

void CIMObjectPath::setHost(const String& host)
{
    if ((host != String::EMPTY) &&
        (host != System::getHostName()) &&
        !CIMObjectPathRep::isValidHostname(host))
    {
        MessageLoaderParms mlParms(
            "Common.CIMObjectPath.INVALID_HOSTNAME",
            "$0, reason:\"invalid hostname\"",
            host);
        throw MalformedObjectNameException(mlParms);
    }
    _rep = _copyOnWriteCIMObjectPathRep(_rep);

    _rep->_host = host;
}

const CIMNamespaceName& CIMObjectPath::getNameSpace() const
{
    return _rep->_nameSpace;
}

void CIMObjectPath::setNameSpace(const CIMNamespaceName& nameSpace)
{
    _rep = _copyOnWriteCIMObjectPathRep(_rep);
   _rep->_nameSpace = nameSpace;
}

const CIMName& CIMObjectPath::getClassName() const
{
    return _rep->_className;
}

void CIMObjectPath::setClassName(const CIMName& className)
{
    _rep = _copyOnWriteCIMObjectPathRep(_rep);
    _rep->_className = className;
}

const Array<CIMKeyBinding>& CIMObjectPath::getKeyBindings() const
{
    return _rep->_keyBindings;
}

void CIMObjectPath::setKeyBindings(const Array<CIMKeyBinding>& keyBindings)
{
    _rep = _copyOnWriteCIMObjectPathRep(_rep);
    _rep->_keyBindings = keyBindings;
    _Sort(_rep->_keyBindings);
}

String CIMObjectPath::toString() const
{
    String objectName;

    // Get the host:

    if (_rep->_host.size())
    {
        objectName = "//";
        objectName.append(_rep->_host);
        objectName.append("/");
    }

    // Get the namespace (if we have a host name, we must write namespace):

    if (!_rep->_nameSpace.isNull() || _rep->_host.size())
    {
        objectName.append(_rep->_nameSpace.getString ());
        objectName.append(":");
    }

    // Get the class name:

    objectName.append(getClassName().getString ());

    //
    //  ATTN-CAKG-P2-20020726:  The following condition does not correctly
    //  distinguish instanceNames from classNames in every case
    //  The instanceName of a singleton instance of a keyless class has no
    //  key bindings. See BUG_3302
    //
    if (_rep->_keyBindings.size () != 0)
    {
        objectName.append('.');

        // Append each key-value pair:

        const Array<CIMKeyBinding>& keyBindings = getKeyBindings();

        for (Uint32 i = 0, n = keyBindings.size(); i < n; i++)
        {
            objectName.append(keyBindings[i].getName().getString ());
            objectName.append('=');

            const String& value = _escapeSpecialCharacters(
                keyBindings[i].getValue());

            CIMKeyBinding::Type type = keyBindings[i].getType();

            if (type == CIMKeyBinding::STRING ||
                type == CIMKeyBinding::REFERENCE)
                objectName.append('"');

            objectName.append(value);

            if (type == CIMKeyBinding::STRING ||
                type == CIMKeyBinding::REFERENCE)
                objectName.append('"');

            if (i + 1 != n)
                objectName.append(',');
        }
    }

    return objectName;
}

String CIMObjectPath::_toStringCanonical() const
{
    CIMObjectPath ref;
    *ref._rep = *this->_rep;

    // Normalize hostname by changing to lower case
    ref._rep->_host.toLower(); // ICU_TODO:

    // Normalize namespace by changing to lower case
    if (!ref._rep->_nameSpace.isNull())
    {
        String nameSpaceLower = ref._rep->_nameSpace.getString();
        nameSpaceLower.toLower(); // ICU_TODO:
        ref._rep->_nameSpace = nameSpaceLower;
    }

    // Normalize class name by changing to lower case
    if (!ref._rep->_className.isNull())
    {
        String classNameLower = ref._rep->_className.getString();
        classNameLower.toLower(); // ICU_TODO:
        ref._rep->_className = classNameLower;
    }

    for (Uint32 i = 0, n = ref._rep->_keyBindings.size(); i < n; i++)
    {
        // Normalize key binding name by changing to lower case
        if (!ref._rep->_keyBindings[i]._rep->_name.isNull())
        {
            String keyBindingNameLower =
                ref._rep->_keyBindings[i]._rep->_name.getString();
            keyBindingNameLower.toLower(); // ICU_TODO:
            ref._rep->_keyBindings[i]._rep->_name = keyBindingNameLower;
        }

        // Normalize the key value
        switch (ref._rep->_keyBindings[i]._rep->_type)
        {
        case CIMKeyBinding::REFERENCE:
            try
            {
                // Convert reference to CIMObjectPath and recurse
                ref._rep->_keyBindings[i]._rep->_value =
                    CIMObjectPath(ref._rep->_keyBindings[i]._rep->_value).
                        _toStringCanonical();
            }
            catch (Exception&)
            {
                // Leave value unchanged if the CIMObjectPath parsing fails
            }
            break;
        case CIMKeyBinding::BOOLEAN:
            // Normalize the boolean string by changing to lower case
            ref._rep->_keyBindings[i]._rep->_value.toLower(); // ICU_TODO:
            break;
        case CIMKeyBinding::NUMERIC:
            // Normalize the numeric string by converting to integer and back
            Uint64 uValue;
            Sint64 sValue;
            // First try converting to unsigned integer
            if (StringConversion::stringToUnsignedInteger(
                    ref._rep->_keyBindings[i]._rep->_value.getCString(),
                        uValue))
            {
                char buffer[32];  // Should need 21 chars max
                sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", uValue);
                ref._rep->_keyBindings[i]._rep->_value = String(buffer);
            }
            // Next try converting to signed integer
            else if (StringConversion::stringToSignedInteger(
                         ref._rep->_keyBindings[i]._rep->_value.getCString(),
                             sValue))
            {
                char buffer[32];  // Should need 21 chars max
                sprintf(buffer, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", sValue);
                ref._rep->_keyBindings[i]._rep->_value = String(buffer);
            }
            // Leave value unchanged if it cannot be converted to an integer
            break;
        default:  // CIMKeyBinding::STRING
            // No normalization required for STRING
            break;
        }
    }

    // Note: key bindings are sorted when set in the CIMObjectPath

    return ref.toString();
}

Boolean CIMObjectPath::identical(const CIMObjectPath& x) const
{
    return
        (_rep == x._rep) ||
        (String::equalNoCase(_rep->_host, x._rep->_host) &&
         _rep->_nameSpace.equal(x._rep->_nameSpace) &&
         _rep->_className.equal(x._rep->_className) &&
         (_rep->_keyBindings == x._rep->_keyBindings));
}

Uint32 CIMObjectPath::makeHashCode() const
{
    return HashFunc<String>::hash(_toStringCanonical());
}

Boolean operator==(const CIMObjectPath& x, const CIMObjectPath& y)
{
    return x.identical(y);
}

Boolean operator!=(const CIMObjectPath& x, const CIMObjectPath& y)
{
    return !operator==(x, y);
}

PEGASUS_NAMESPACE_END
