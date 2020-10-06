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

#ifndef _CmpiObjectPath_h_
#define _CmpiObjectPath_h_

#include "cmpidt.h"
#include "cmpift.h"

#include "CmpiObject.h"
#include "CmpiBroker.h"
#include "CmpiStatus.h"
#include "CmpiData.h"
#include "CmpiBaseMI.h"
#include "Linkage.h"

/**
    The ObjectPath class represents the value of a Instance reference. It
    constains the namespace, classname and key properties.
    ObjectPath can also be used to represent a class reference.
    In that case key properties will be ignored.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiObjectPath : public CmpiObject
{
    friend class CmpiBroker;
    friend class CmpiResult;
    friend class CmpiInstance;
    friend class CmpiInstanceMI;
    friend class CmpiMethodMI;
    friend class CmpiAssociationMI;
    friend class CmpiPropertyMI;
    friend class CmpiIndicationMI;
    friend class CmpiData;
protected:

private:

    /**
        Constructor - Should not be called.
    */
    //CmpiObjectPath() {}
    void *makeObjectPath(CMPIBroker *mb, const char *ns, const char *cls);
    void *makeObjectPath(
        CMPIBroker *mb,
        const CmpiString& ns,
        const char *cls);
    CmpiBoolean doClassPathIsA(CMPIBroker *mb, const char *className) const;
public:

    /**
        Constructor - Creates an ObjectPath object with the classname
        from the input parameter.
        @param ns defining namespace or NULL
        @param cls defining classname or NULL
        @return The new ObjectPath object
    */
    CmpiObjectPath(const char *ns, const char *cls);
    CmpiObjectPath(const CmpiString &ns, const char *cls);

    /**
        Constructor used by MIDrivers to encapsulate CMPIObjectPathes.
    */
    CmpiObjectPath(CMPIObjectPath* c);

    /**
        Gets the encapsulated CMPIObjectPath.
    */
    CMPIObjectPath *getEnc() const;

    /**
        classPathIsA - Tests whether this CIM ObjectPath is a reference
        to a CIM class is of type &lt;className&gt;.
        @param className CIM classname to be tested for.
        @return True or False
    */
    CmpiBoolean classPathIsA(const char *className) const;

    /**
        getHostname - returns the hostname component of the
        ObjectPath
        @return CmpiString containing the hostname.
    */
    CmpiString getHostname() const;

    /**
        setHostname - Sets the hostname component.
        @param hn representing the hostname.
    */
    void setHostname(const char* hn);
    void setHostname(CmpiString hn);

    /**
        getNameSpace - returns the namespace component of the
        ObjectPath
        @return CmpiString containing the namespace.
    */
    CmpiString getNameSpace() const;

    /**
        setNameSpace - Sets the namespace component.
        @param ns representing the namespace.
    */
    void setNameSpace(const char* ns);
    void setNameSpace(CmpiString ns);

    /**
        getClassName - returns the classname component of the
        ObjectPath
        @return CmpiString containing the namespace.
    */
    CmpiString getClassName() const;

    /**
        setClassName - Sets the classname component.
        @param hn representing the hostname.
    */
    void setClassName(const char* hn);
    void setClassName(CmpiString hn);

    /**
        getKeyCount - Gets the number of key properties
        defined for this ObjectPath.
        @return Number of key properties of this ObjectPath.
    */
    unsigned int getKeyCount() const;

    /**
        getKey - Gets the CmpiData object representing the value
        associated with the key
        @param name key name.
        @return CmpiData value object associated with the key.
    */
    CmpiData getKey(const char* name) const;

    /**
        getKey - Gets the CmpiData object defined
        by the input index parameter.
        @param index Index into the key property array.
        @param name Optional output parameter returning the key name.
        @return CmpiData value object corresponding to the index.
    */
    CmpiData getKey(const int index, CmpiString *name=NULL) const;

    /**
        setKey - adds/replaces a key value defined by the input
        parameter to the ObjectPath
        @param name key name.
        @param data Type and Value to be added.
    */
    void setKey(const char* name, const CmpiData data);
};

#endif


