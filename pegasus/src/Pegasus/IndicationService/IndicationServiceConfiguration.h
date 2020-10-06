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

#ifndef Pegasus_IndicationServiceConfiguration_h
#define Pegasus_IndicationServiceConfiguration_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Repository/CIMRepository.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The IndicationServiceConfiguration class manages the Indications
    profile configuration instances. This class does not store any
    instances in the repository. Instances are generated dynamically.
*/

class PEGASUS_SERVER_LINKAGE IndicationServiceConfiguration
{
public:

    IndicationServiceConfiguration(CIMRepository *repository);
    ~IndicationServiceConfiguration(void);

    CIMInstance getInstance (
        const CIMNamespaceName & nameSpace,
        const CIMObjectPath & instanceName,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList & propertyList = CIMPropertyList());

    Array <CIMInstance> enumerateInstancesForClass (
        const CIMNamespaceName & nameSpace,
        const CIMName & className,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList & propertyList = CIMPropertyList());

    Array <CIMObjectPath> enumerateInstanceNamesForClass (
        const CIMNamespaceName & nameSpace,
        const CIMName & className);

    Uint16 getEnabledState()
    {
       return _enabledState;
    }
    Uint16 getHealthState()
    {
       return _healthState;
    }
    void setEnabledState(Uint16 enabledState)
    {
       _enabledState = enabledState;
    }
    void setHealthState(Uint16 healthState)
    {
       _healthState = healthState;
    }

private:

    CIMInstance _buildInstanceSkeleton(
        const CIMNamespaceName &nameSpace,
        const CIMName &className,
        CIMClass &returnedClass);

    void _setPropertyValue(
        CIMInstance &instance,
        const CIMName &propertyName,
        const CIMValue &value);

    void _setIntervalPropertyValues(CIMInstance& instance);

    CIMInstance _getIndicationServiceInstance(
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList &propertyList);

    CIMInstance _getIndicationServiceCapabilitiesInstance(
        Boolean includeQualifiers,
        Boolean includeClassOrigin,
        const CIMPropertyList &propertyList);

    // Used for getting the Class defintions from the Repository.
    CIMRepository * _cimRepository;
    Uint16 _enabledState;
    Uint16 _healthState;
};

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_IndicationServiceConfiguration_h */
