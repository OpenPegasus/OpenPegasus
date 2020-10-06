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


#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/XmlGenerator.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/Constants.h>
#include <cstdio>
#include <iostream>
#include <cstring>

#include "RsURI.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMNamespaceName RsURI::_defaultNamespace;
String RsURI::_defaultNamespaceEncoded;

static int _compare(const void* p1, const void* p2)
{
    const CIMName* prop1 = (CIMName*) p1;
    const CIMName* prop2 = (CIMName*) p2;

    return String::compareNoCase(prop1->getString(), prop2->getString());
}

RsURI::RsURI()
{
}

RsURI::RsURI(const String& uri) :
    _classNamePos(PEG_NOT_FOUND)
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::RsURI()");

    _authority = String::EMPTY;

    // Parse a path-absolute URI as given by
    // RFC 3986, Section 4.2

    Uint32 prevSegment = 7; // assume the URI starts with "/cimrs/"
    Uint32 nextSegment;

    while ((nextSegment = uri.find(prevSegment, '/')) != PEG_NOT_FOUND)
    {
        PEGASUS_ASSERT(nextSegment - prevSegment > 0);
        _path.append(uri.subString(prevSegment, nextSegment - prevSegment));
        prevSegment = nextSegment + 1;
    }

    // Add last segment to path
    Uint32 lastSegment;
    if ((lastSegment = uri.size() - prevSegment) > 0)
    {
        String last = uri.subString(prevSegment, lastSegment);
        Uint32 queryStringPos = last.find('?');

        if (queryStringPos == PEG_NOT_FOUND)
        {
            _path.append(last);
        }
        else
        {
            if (queryStringPos > 0)
                _path.append(last.subString(0, queryStringPos));

            _queryString = last.subString(queryStringPos + 1,
                last.size() - queryStringPos + 1);
        }
    }

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "Query String: [%s]",
        (const char*)_queryString.getCString()));

#ifdef PEGASUS_DEBUG
    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsURI uri: [%s]",
        (const char*)uri.getCString()));

    for (Uint32 x = 0; x < _path.size(); x++)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI path segment [%d]: [%s]",
            x,
            (const char*)_path[x].getCString()));
    }
#endif

    _uri = uri;

    PEG_METHOD_EXIT();
}

RsURI::~RsURI()
{
}


String& RsURI::getString()
{
    return _uri;
}



String RsURI::getNamespaceName(Boolean encoded)
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::getNamespaceName()");

    // namespace value is the second segment in path
    // e.g. /cimrs/root%2Fcimv2/

    if (_namespaceName.isNull() && _path.size() > 0)
    {
        _namespaceEncodedName = _path[0];
        _namespaceName = XmlReader::decodeURICharacters(_path[0]);
    }
    PEG_METHOD_EXIT();
    return (encoded)? _namespaceEncodedName : _namespaceName.getString();
}


Boolean RsURI::hasClassesPath()
{
    if (_namespaceName.isNull())
    {
        getNamespaceName();
    }
    return (_classNamePos > 0);

}

Boolean RsURI::hasClassPath()
{
    if (_className.isNull())
    {
        getClassName();
    }

    return !(_className.isNull());
}

CIMName RsURI::getClassName()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::getClassName()");

    // class name is always the third segment in path
    // e.g. /cimrs/root%2Fcimv2/ACME_RegisteredProfile

    const Uint32 pathSize = _path.size();

    if (_className.isNull() && pathSize > 1 && CIMName::legal(_path[1]))
    {
        _className = CIMName(_path[1]);

        // if the class name is enum, then this is not the class name.
        // in such cases the class name is in the query string
        if(String::compare(_path[1], "enum") == 0)
        {
            Uint32 classPos = _queryString.find("class=");
            if (classPos == PEG_NOT_FOUND)
            {
                _className = "";
                PEG_METHOD_EXIT();
                return _className;
            }

            classPos += 6; // properties= <- start behind =
            Uint32 nextParamPos = _queryString.find(classPos, '&');
            if(nextParamPos == PEG_NOT_FOUND)
            {
                nextParamPos = _queryString.size();
            }

            _className = _queryString.subString(
                classPos,
                nextParamPos - classPos);
        }
    }
    PEG_METHOD_EXIT();
    return _className;
}

Boolean RsURI::hasEnum()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::hasEnum()");

    // enum is as shown
    // e.g. /cimrs/root%2Fcimv2/enum?class=ACME_RegisteredProfile

    Boolean res = false;

    if (_path.size() > 1)
    {
        res = (String::compareNoCase(_path[1], "enum") == 0);
    }
    PEG_METHOD_EXIT();
    return res;
}

Boolean RsURI::hasCreate()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::hasCreate()");

    // enum is as shown
    // e.g. /cimrs/root%2Fcimv2/create?class=ACME_RegisteredProfile

    Boolean res = false;

    if (_path.size() > 1)
    {
        res = (String::compareNoCase(_path[1], "create") == 0);
    }
    PEG_METHOD_EXIT();
    return res;
}

Boolean RsURI::hasInstancePath()
{
    // /cimrs/root%2Fcimv2/ACME_RegisteredProfile/key

    // see if we can get the classname first
    if (_className.isNull())
    {
        getClassName();
    }

    // if got a class name, then see if there are keys specified?
    Boolean res = false;
    if(!_className.isNull())
    {
        if(_path.size() > 2)
            res = true;
    }

    return res;
}

Boolean RsURI::hasInstancesPath()
{
    // /cimrs/root%2Fcimv2/ACME_RegisteredProfile
    if (_className.isNull())
    {
        getClassName();
    }

    return (!_className.isNull());
}

Boolean RsURI::hasAssociationPath()
{
    // if there is an expand or refer and these is a single dot in the value,
    // then true;
    return(_checkQuerystring() &&
            _navHopCount() == 1);
}

Boolean RsURI::hasReferencesPath()
{
    // if there is an expand or refer and these is NO dot in the value,
    // then true;
    return(_checkQuerystring() &&
            _navHopCount() == 0);
}

Uint32 RsURI::_navHopCount()
{
    Uint32 startPos = PEG_NOT_FOUND;
    Uint32 countDot = 0;

    // <instancepath>?expand=<associationclass>.<ResultRole>.<>.<>
    // <instancepath>?refer=<associationclass>.<ResultRole>.<>.<>

    if(_checkQuerystring())
    {
        // check to see if there is exactly one .
        //in the value of expand or refer
        Uint32 endPos;
        endPos = _queryString.find(startPos, '&');
        if(endPos == PEG_NOT_FOUND)
            endPos = _queryString.size() - 1;

        startPos = _queryString.find(startPos, '.');
        while(startPos < endPos)
        {
            countDot++;
            startPos = _queryString.find(startPos + 1, '.');
        }
    }
    return countDot;
}

String RsURI::getNavString()
{
    // test expand first
    String paramValue = _findStringParameter("expand");

    if(paramValue.size() == 0)
    {
        paramValue = _findStringParameter("refer");
    }

    return paramValue;
}

Boolean RsURI::hasMethodPath()
{
    // /cimrs/root%2Fcimv2/ACME_RegisteredProfile/key/methodName
    return (hasInstancePath() && _path.size() > 3);
}

CIMName RsURI::getMethodName()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::getMethodName()");

    if (_methodName.isNull() && _path.size() > 3 && CIMName::legal(_path[3]))
    {
        _methodName = CIMName(_path[3]);
    }

    PEG_METHOD_EXIT();
    return _methodName;
}


CIMObjectPath RsURI::getAssociationPath(const CIMClass& cimClass)
{
    if (!hasAssociationPath())
    {
        throw Exception("Association path requested but URI has "
            "none defined.");
    }

    return hasInstancePath() ? getInstancePath(cimClass) :
        cimClass.getPath();
}

CIMObjectPath RsURI::getReferencePath(const CIMClass& cimClass)
{
    if (!hasReferencesPath())
    {
        throw Exception("Reference path requested but URI has "
            "none defined.");
    }

    return hasInstancePath() ? getInstancePath(cimClass) :
        cimClass.getPath();
}


CIMObjectPath RsURI::getMethodPath(const CIMClass& cimClass)
{
    if (!hasMethodPath())
    {
        throw Exception("Method path requested but URI has "
            "none defined.");
    }

    return hasInstancePath() ? getInstancePath(cimClass) :
        cimClass.getPath();
}

CIMObjectPath RsURI::getInstancePath(const CIMClass& cimClass)
{
    return RsURI::_getInstancePath(cimClass, 0);
}

Boolean RsURI::hasQualifier()
{
    return (_queryString.size() > 0 &&
            (_queryString.find("IncludeQualifiers=true") != PEG_NOT_FOUND));
}

Boolean RsURI::hasDeepInheritance()
{
    return (_queryString.size() > 0 &&
        (_queryString.find("DeepInheritance=true") != PEG_NOT_FOUND));
}

CIMName RsURI::getAssociationClassName()
{
    // test expand first
    String paramValue = _findStringParameter("expand");
    if(paramValue.size() == 0)
    {
        paramValue = _findStringParameter("refer");
    }

    Uint32 endPos = paramValue.find(0, '.');
    String assocClass;
    if(endPos != PEG_NOT_FOUND)
    {
        assocClass = paramValue.subString(0, endPos);
    }
    return CIMName(assocClass);
}

CIMName RsURI::getAssociatedClassName()
{
    return _findNameParameter(String("AssociatedClass"));
}

String RsURI::getSourceRoleName()
{
    return _findStringParameter(String("SourceRole"));
}

String RsURI::getAssociatedRoleName()
{
    // <instancepath>?expand=<associationclass>.<AssociatedRole>

    // test expand first
    String paramValue = _findStringParameter("expand");
    String assocedRole;
    if(paramValue.size() == 0)
    {
        paramValue = _findStringParameter("refer");
    }

    Uint32 endPos = paramValue.find(0, '.');
    if(endPos != PEG_NOT_FOUND)
    {
        assocedRole = paramValue.subString(endPos+1,
                          paramValue.size() - endPos - 1);
    }
    return assocedRole;

}

String RsURI::getParamValues(
    CIMConstMethod& method,
    Array<CIMParamValue>& inParms)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,"getParamValues");

    if (_queryString.size() == 0)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,"Parameter string empty"));

        PEG_METHOD_EXIT();
        return String();
    }

    Array<CIMName> pNames;
    Array<String> pValues;

    Uint32 currentPos = 0;
    Uint32 endPos = _queryString.size();
    Uint32 nextPos = _queryString.find("=");
    while ((nextPos != PEG_NOT_FOUND) && (nextPos < endPos))
    {
        CIMName pName(_queryString.subString(currentPos, nextPos-currentPos));
        cout << "Name = "<<pName.getString()<<endl;
        pNames.append(pName);
        currentPos = nextPos+1;
        nextPos = _queryString.find(currentPos,'?');
        if (nextPos == PEG_NOT_FOUND)
        {
            nextPos = endPos;
        }
        String pValue(_queryString.subString(currentPos, nextPos-currentPos));
        cout << "Value = "<<pValue<<endl;
        pValues.append(pValue);
        currentPos = nextPos+1;
        if (currentPos < endPos)
        {
            nextPos = _queryString.find(currentPos,'=');
        }

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::getParamValues() Name: [%s], Value: [%s]",
            (const char*)pName.getString().getCString(),
            (const char*)pValue.getCString()));
    }


    for (Uint32 x = 0; x < pNames.size(); x++)
    {
        Uint32 index = method.findParameter(pNames[x]);
        CIMConstParameter p = method.getParameter(index);
        CIMType pType = p.getType();

        CIMValue pVal = XmlReader::stringToValue(0,
                                pValues[x].getCString(),pType);
        inParms.append(CIMParamValue(pNames[x].getString(),pVal));
    }

    PEG_METHOD_EXIT();
    return String();
}

CIMPropertyList RsURI::getPropertyList()
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::getPropertyList()");

    if (_queryString.size() == 0)
    {
        PEG_METHOD_EXIT();
        return CIMPropertyList();
    }

    Uint32 paramsPos = _queryString.find("properties");
    if (paramsPos == PEG_NOT_FOUND)
    {
        PEG_METHOD_EXIT();
        return CIMPropertyList();
    }

    paramsPos += 11; // properties= <- start behind =
    Uint32 nextParamPos;
    Array<CIMName> properties;

    while ((nextParamPos = _queryString.find(paramsPos, ',')) != PEG_NOT_FOUND)
    {
        properties.append(_queryString.subString(
            paramsPos, nextParamPos - paramsPos));

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::getPropertyList() added Property [%s] (%d-%d)",
            (const char*)_queryString.subString(
                paramsPos, nextParamPos - paramsPos).getCString(),
                paramsPos, nextParamPos));

        paramsPos = nextParamPos + 1;
    }

    if (_queryString.size() > paramsPos && _queryString[paramsPos] != '&')
    {
        properties.append(_queryString.subString(
            paramsPos, _queryString.size() - paramsPos));

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::getPropertyList() added last Property [%s] (%d-%d)",
            (const char*)_queryString.subString(
                paramsPos, _queryString.size() - paramsPos).getCString(),
            paramsPos, _queryString.size() - paramsPos));
    }

    PEG_METHOD_EXIT();
    return CIMPropertyList(properties);
}

CIMObjectPath RsURI::_getInstancePath(const CIMClass& cimClass, Uint32 refPos)
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::getInstancePath()");

    if (!hasInstancePath())
    {
        throw Exception("Instance path requested but URI has "
            "none defined.");
    }

    String unDecoded(_path[2]);

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsURI::getInstanceName() decoded URI characters"));

    CIMObjectPath objPath;
    Array<CIMKeyBinding> keyBindings;
    Array<CIMName> keyNames;
    cimClass.getKeyNames(keyNames);

    Uint32 prev = 0;
    Uint32 next;

    qsort((void*)keyNames.getData(), keyNames.size(),
        sizeof(CIMName), _compare);

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsURI::getInstanceName() keyNames.size = %d",
        keyNames.size()));

    if (unDecoded.find(0, ';'))
    {

        for (Uint32 pos = 0; pos < keyNames.size(); pos++)
        {
            String value;
            next = unDecoded.find(prev, '+');

            if (next == PEG_NOT_FOUND)
            {
                if ((next = unDecoded.find(prev, ';')) == PEG_NOT_FOUND)
                {
                    next = unDecoded.size();// - prev; // last iteration
                }
            }

            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsURI::getInstanceName() pos = [%d], value = [%s]",
                    pos+1, (const char*)(unDecoded.subString(prev,
                                         next - prev).getCString())));

            value = XmlReader::decodeURICharacters(
                    unDecoded.subString(prev, next - prev));

            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsURI::getInstanceName() pos = [%d]",
                    pos+1));

            _appendKeybinding(
                keyBindings,
                cimClass,
                keyNames[pos],
                value,
                refPos);

            prev = next + 1;
        }
    }
    objPath.set(
        String(),
        CIMNamespaceName(),
        cimClass.getClassName().getString(),
        keyBindings);

    PEG_METHOD_EXIT();
    return objPath;
}

void RsURI::_appendKeybinding(
    Array<CIMKeyBinding>& keyBindings,
    const CIMClass& cimClass,
    CIMName& keyName, String value, Uint32 refPos)
{
    Uint32 propertyPos = cimClass.findProperty(keyName);
    if (propertyPos == PEG_NOT_FOUND)
    {
        // This shouldn't happen - keyName is given
        // by the CIM class
        throw Exception("Key has no property.");
    }

    CIMConstProperty property = cimClass.getProperty(propertyPos);
    CIMName key = property.getName();

    if (property.getType() == CIMTYPE_REFERENCE)
    {
        Uint32 refValPos = value.find(';');
        String refVal;
        if (refValPos == PEG_NOT_FOUND)
        {
            refVal = value.subString(
                refValPos + 1,
                value.size() - refValPos - 1);
        }
        else
        {
            refVal = value.subString(refPos, refValPos);
        }

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::_appendKeybinding() reference: [%s]",
            (const char*)refVal.getCString()));

        RsURI refUri(refVal);

        CIMClass refClass = _repository->getClass(
            refUri.getNamespaceName(),
            refUri.getClassName(),
            false /*localOnly*/);

        CIMObjectPath refPath = refUri.getInstancePath(refClass);

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::_appendKeybinding() refPath: %s",
            (const char*)refPath.toString().getCString()));

        CIMKeyBinding newKeyBinding( key, refPath);
        keyBindings.append(newKeyBinding);
    }
    else
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::_appendKeybinding() Key [%s] Value [%s] Type [%s]",
            (const char*)key.getString().getCString(),
            (const char*)value.getCString(),
            cimTypeToString(property.getType())));

        CIMKeyBinding newKeyBinding( key, value);
        keyBindings.append(newKeyBinding);
    }
}


Buffer RsURI::fromObjectPath(
    const CIMObjectPath& objPath,
    Boolean useAbsoluteUri)
{
    Buffer uri(2048);

    if (useAbsoluteUri)
    {
        uri.append(STRLIT_ARGS("/cimrs/"));

        if (objPath.getNameSpace().getString().size() > 0)
        {
            String nameSpace = objPath.getNameSpace().getString();
            // replace all / with %2F to encode the /
            Uint32 prev = 0;
            while(1)
            {
                Uint32 pos = nameSpace.find(prev, '/');
                if(pos == PEG_NOT_FOUND)
                {
                    uri << nameSpace.subString(prev);
                    break;
                }
                uri << nameSpace.subString(prev, pos - prev);
                uri << "%2F";
                prev = pos + 1;
            }
            //uri << objPath.getNameSpace().getString();
        }
        else
        {
            uri << RsURI::_defaultNamespaceEncoded; 
        }

        uri.append(STRLIT_ARGS("/"));
        uri << objPath.getClassName().getString();
        uri.append(STRLIT_ARGS("/"));
    }

    const Array<CIMKeyBinding>& keyBindings = objPath.getKeyBindings();

    for (Uint32 i = 0, n = keyBindings.size(); i < n; i++)
    {
        CIMKeyBinding::Type type = keyBindings[i].getType();

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsURI::fromObjectPath() Key Value [%s] Type [%d]",
            (const char*)keyBindings[i].getValue().getCString(),
            type));

        if (type == CIMKeyBinding::STRING)
        {
            // encodeURICharacters for a Buffer is much
            // faster than for a string
            Buffer buf(2048);
            buf << keyBindings[i].getValue();

            uri << XmlGenerator::encodeURICharacters(buf.getData());
            if (i < n - 1)
            {
                uri.append('+');
            }
        }
        else if (type == CIMKeyBinding::REFERENCE)
        {
            Buffer refBuf;
            CIMObjectPath ref(keyBindings[i].getValue());
            if (ref.getNameSpace().getString().size() == 0)
            {
                ref.setNameSpace(objPath.getNameSpace());
            }

            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsURI::fromObjectPath() associationId = %d, "
                    "namespace %s / %s / %s",
                useAbsoluteUri,
                (const char*)ref.getNameSpace().getString().getCString(),
                (const char*)objPath.getNameSpace().
                getString().getCString(),
                (const char*)RsURI::_defaultNamespace.
                getString().getCString()));

            refBuf = RsURI::fromObjectPath(ref, true);

            if (useAbsoluteUri)
            {
                uri << XmlGenerator::encodeURICharacters(refBuf.getData());
            }
            else
            {
                uri.append(refBuf.getData(), refBuf.size());
            }

            if (i < n - 1)
            {
                uri.append('+');
            }
        }
        else
        {
            uri << keyBindings[i].getValue();
            if (i < n - 1)
            {
                uri.append('+');
            }
        }


    }

    return uri;
}

void RsURI::setRepository(CIMRepository* repo)
{
    _repository = repo;
}


Boolean RsURI::hasDefaultNamespace()
{
    return RsURI::_defaultNamespace.getString().size() > 0;
}

void RsURI::setDefaultNamespace(CIMNamespaceName n)
{
    RsURI::_defaultNamespace = n;
}

void RsURI::setDefaultNamespaceEncoded(String n)
{
    RsURI::_defaultNamespaceEncoded = n;
}

CIMName RsURI::_findNameParameter(const String& key)
{
    String value = _findStringParameter(key);
    if (value.size() > 0)
    {
        return CIMName(value);
    }
    else
    {
        return CIMName();
    }
}

String RsURI::_findStringParameter(const String& key)
{
    PEG_METHOD_ENTER(TRC_RSSERVER, "RsURI::_findStringParameter()");

    if (_queryString.size() == 0)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4, "Query string empty passed"));

        PEG_METHOD_EXIT();
        return String();
    }

    Uint32 nameStartPos = _queryString.find(key);
    if (nameStartPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "Query component not found"));

        PEG_METHOD_EXIT();
        return String();
    }
    nameStartPos += key.size() + 1;

    Uint32 nameEndPos = _queryString.find(nameStartPos, '&');
    if (nameEndPos == PEG_NOT_FOUND)
    {
        nameEndPos = _queryString.size();
    }

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
        "RsURI::_findNameParameter(String &s) Name: [%s], Value: [%s]",
        (const char*)key.getCString(),
        (const char*)_queryString.subString(nameStartPos,
            nameEndPos).getCString()));

    PEG_METHOD_EXIT();
    return _queryString.subString(nameStartPos, nameEndPos);
}

PEGASUS_NAMESPACE_END
