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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_RsURI_h
#define Pegasus_RsURI_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/RSServer/Linkage.h>


PEGASUS_NAMESPACE_BEGIN

/*
 *RsURI class represnts the the request URI in the CIM-RS request
 *
*/

class PEGASUS_RSSERVER_LINKAGE RsURI
{
public:
    RsURI();
    RsURI(const String&);
    virtual ~RsURI();


    String getNamespaceName(Boolean encoded = false);
    CIMName getClassName();

    Boolean hasClassesPath();
    Boolean hasClassPath();
    Boolean hasInstancesPath();
    Boolean hasInstancePath();
    Boolean hasAssociationPath();
    Boolean hasReferencesPath();
    Boolean hasMethodPath();
    Boolean hasEnum();
    Boolean hasCreate();

    CIMObjectPath getAssociationPath(const CIMClass&);
    CIMObjectPath getReferencePath(const CIMClass& cimClass);
    CIMObjectPath getMethodPath(const CIMClass& cimClass);


    void setRepository(CIMRepository*);
    CIMObjectPath getInstancePath(const CIMClass&);
    CIMName getMethodName();
    String getParamValues(CIMConstMethod&, Array<CIMParamValue>&);
    Boolean hasDeepInheritance();
    Boolean hasQualifier();
    CIMPropertyList getPropertyList();
    CIMName getAssociationClassName();
    CIMName getAssociatedClassName();
    String getSourceRoleName();
    String getAssociatedRoleName();
    String getNavString();

    String& getString();


    static Buffer fromObjectPath(const CIMObjectPath&, Boolean);

    static Boolean hasDefaultNamespace();
    static void setDefaultNamespace(CIMNamespaceName);
    static void setDefaultNamespaceEncoded(String n);

private:
    CIMRepository* _repository;
    String _authority;
    Array<String> _path;
    String _queryString;

    CIMNamespaceName _namespaceName;
    String _namespaceEncodedName;
    CIMName _className;
    CIMName _methodName;
    Uint32 _classNamePos;
    String _uri;
    static CIMNamespaceName _defaultNamespace;
    static String _defaultNamespaceEncoded; // '/' is encoded


    CIMObjectPath _getInstancePath(const CIMClass&, Uint32);
    void _appendKeybinding(Array<CIMKeyBinding>& keyBindings,
                           const CIMClass& cimClass,
                           CIMName& keyName,
                           String value,
                           Uint32 refPos);
    CIMName _findNameParameter(const String&);
    String _findStringParameter(const String&);
    Uint32 _navHopCount();

    inline Boolean _checkQuerystring()
    {
        return (_queryString.size() > 0 &&
               ((_queryString.find("expand=") != PEG_NOT_FOUND) ||
               (_queryString.find("refer=") != PEG_NOT_FOUND)));
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_RsURI_h */
