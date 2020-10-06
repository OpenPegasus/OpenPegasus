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

#ifndef _CmpiInstance_h_
#define _CmpiInstance_h_

#include "cmpidt.h"
#include "cmpift.h"

//#include "CmpiImpl.h"

#include "CmpiObject.h"
#include "CmpiStatus.h"
#include "CmpiObjectPath.h"
#include "CmpiBroker.h"
#include "CmpiData.h"
#include "Linkage.h"

class CmpiObjectPath;
class CmpiData;

/**
    This class represents the instance of a CIM class. It is used manipulate
    instances and their parts.
*/

class PEGASUS_CMPI_PROVIDER_LINKAGE CmpiInstance : public CmpiObject
{
    friend class CmpiBroker;
    friend class CmpiResult;
    friend class CmpiInstanceMI;
    friend class CmpiMethodMI;
    friend class CmpiData;
protected:

    /**
        Protected constructor used by MIDrivers to encapsulate CMPIInstance.
    */
    CmpiInstance(const CMPIInstance* enc);

    /**
        Gets the encapsulated CMPIInstance.
    */
    CMPIInstance *getEnc() const;

private:

    /**
        Constructor - Should not be called
    */
    CmpiInstance();

    /**
        Internal make function
    */
    void *makeInstance(CMPIBroker *mb, const CmpiObjectPath& cop);

    CmpiBoolean doInstanceIsA(CMPIBroker *mb, const char *className) const;

public:
    /**
        Constructor - Creates an Instance object with the classname
        from the input parameter.
        @param op defining classname and namespace
        @return The new Instance object
    */
    CmpiInstance(const CmpiObjectPath& op);

    /**
        instanceIsA - Tests whether this CIM Instance is of type
        &lt; className&gt;.
        @param className CIM classname to be tested for.
        @return True or False
    */
    CmpiBoolean instanceIsA(const char *className) const;

    /**
        getPropertyCount - Gets the number of Properties
        defined for this Instance.
        @return Number of Properties of this instance.
    */
    unsigned int getPropertyCount() const;

    /**
        getProperty - Gets the CmpiData object representing the value
        associated with the property name
        @param name Property name.
        @return CmpiData value object associated with the property.
    */
    CmpiData getProperty(const char* name) const;

    /**
        getProperty - Gets the CmpiData object defined
        by the input index parameter.
        @param index Index into the Property array.
        @param name Optional output parameter returning the property name.
        @return CmpiData value object corresponding to the index.
    */
    CmpiData getProperty(const int index, CmpiString *name=NULL) const;

    /**
        setProperty - adds/replaces a property value defined by the input
        parameter to the Instance
        @param name Property name.
        @param data Type and Value to be added.
    */
    void setProperty(const char* name, const CmpiData data);

    /**
        setPropertyFilter - specifies the list of properties that is accepted
        for setProperty operestions.
        @param properties List of properties
        @param keys List of key properties (required)
    */

    void setPropertyFilter(const char** properties, const char** keys);

    /**
        getObjectPath - generates an ObjectPath out of the
        namespace, classname and key propeties of this Instance
        @return the generated ObjectPath.
    */
    CmpiObjectPath getObjectPath() const;
};

#endif
