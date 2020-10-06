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

#ifndef SLPProv_H
#define SLPProv_H

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/../slp/slp_agent/peg_slp_agent.h>

PEGASUS_NAMESPACE_BEGIN

Boolean initFlag = false;

class SLPProvider: public CIMInstanceProvider, public CIMMethodProvider
{
    public:
        SLPProvider(void);
        ~SLPProvider(void);

        void initialize(CIMOMHandle & cimom);
        void terminate(void);

        void getInstance(
           const OperationContext & context,
           const CIMObjectPath & ref,
           const Boolean includeQualifiers,
           const Boolean includeClassOrigin,
           const CIMPropertyList & propertyList,
           InstanceResponseHandler & handler);

        void enumerateInstances(
           const OperationContext & context,
               const CIMObjectPath & ref,
               const Boolean includeQualifiers,
           const Boolean includeClassOrigin,
           const CIMPropertyList & propertyList,
           InstanceResponseHandler & handler);

        void enumerateInstanceNames(
           const OperationContext & context,
           const CIMObjectPath & ref,
           ObjectPathResponseHandler & handler);

        void modifyInstance(
               const OperationContext & context,
           const CIMObjectPath & ref,
           const CIMInstance & obj,
           const Boolean includeQualifiers,
           const CIMPropertyList & propertyList,
           ResponseHandler & handler);

        void createInstance(
           const OperationContext & context,
           const CIMObjectPath & ref,
           const CIMInstance & obj,
           ObjectPathResponseHandler & handler);

        void deleteInstance(
           const OperationContext & context,
           const CIMObjectPath & ref,
           ResponseHandler & handler);

        //Boolean tryterminate(void);

        virtual void invokeMethod(
            const OperationContext & context,
            const CIMObjectPath & objectReference,
            const CIMName & methodName,
            const Array<CIMParamValue> & inParameters,
            MethodResultResponseHandler & handler);

        static void updateProfileRegistration();
    protected:
        CIMInstance _buildInstanceSkeleton(const CIMName& className);

        void deregisterSLP();

        Boolean populateRegistrationData(const String &protocol,
            const String& IPAddress,
            const CIMInstance& instance_ObjMgr,
            const CIMInstance& instance_ObjMgrComm,
            const CIMClass& commMechClass,
            const String& registeredProfiles,
            const OperationContext & context);

        Boolean issueSLPRegistrations(const OperationContext & context);
        Uint32 populateSLPRegistrations(const OperationContext & context);
        String getNameSpaceInfo(
                   const CIMNamespaceName& nameSpace,
                   String& classInfo,
                   const OperationContext & context );

        void populateTemplateField(CIMInstance& instance,
            const String& attributeFieldName,
            const String& value,
            const String& instancePropertyName = String::EMPTY);

        void populateTemplateField(CIMInstance& instance,
            const String& attributeFieldName,
            const Array<String>& value,
            const String& instancePropertyName);

        String getRegisteredProfileList(const OperationContext & context);
   private:
        void _processLinkLocalAddress(String &ip6add2check);
        Boolean _checkProfileToAdvertise(const CIMInstance &, const Uint32 &);

       // Save instances of registration class
       // that have been registered.
       Array<CIMObjectPath> _instanceNames;
       Array<CIMInstance> _instances;
       CIMNamespaceName _nameSpace;

       slp_service_agent slp_agent;

       static SLPProvider* _this;

       // Save CIMOMHandle from initialization
       CIMOMHandle _cimomHandle;

       CIMNamespaceName _interopNamespace;

       // Workspace for building the SLP Template String
       String _currentSLPTemplateString;
       CIMInstance _currentSLPTemplateCIMInstance;
   };

PEGASUS_NAMESPACE_END

#endif
