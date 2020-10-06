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


#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/ProviderException.h>

PEGASUS_USING_PEGASUS;

#include "OperatingSystemProvider.h"
#include "OperatingSystem.h"

PEGASUS_USING_STD;

#define OSP_DEBUG(X) // cout << "OperatingSystemProvider" <<  X << endl;

#define STANDARDOPERATINGSYSTEMCLASS CIMName ("CIM_OperatingSystem")
#define EXTENDEDOPERATINGSYSTEMCLASS CIMName ("PG_OperatingSystem")
#define CSCREATIONCLASSNAME CIMName ("CIM_UnitaryComputerSystem")

OperatingSystemProvider::OperatingSystemProvider()
{
}

OperatingSystemProvider::~OperatingSystemProvider()
{
}

void OperatingSystemProvider::getInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler &handler)
{
    Array<CIMKeyBinding> keys;
    CIMInstance instance;
    OperatingSystem os;
    CIMName className;
    String csName;
    String name;


    //-- make sure we're working on the right class
    className = ref.getClassName();
    if (!(className.equal (STANDARDOPERATINGSYSTEMCLASS)) &&
        !(className.equal (EXTENDEDOPERATINGSYSTEMCLASS)))
    {
        throw CIMNotSupportedException(
            "OperatingSystemProvider does not support class " +
                className.getString());
    }

    //-- make sure we're the right instance
    int keyCount;
    CIMName keyName;

    keyCount = 4;
    keys = ref.getKeyBindings();

    if ((unsigned int)keys.size() != (unsigned int)keyCount)
        throw CIMInvalidParameterException("Wrong number of keys");

    // doesn't seem as though this code will handle duplicate keys,
    // but it appears as though the CIMOM strips those out for us.
    // Despite test cases, don't get invoked with 2 keys of the same
    // name.

    if (!os.getCSName(csName))
    {
        throw CIMOperationFailedException("OperatingSystemProvider "
                       "Can't determine name of computer system");
    }

    if (!os.getName(name))
    {
        throw CIMOperationFailedException("OperatingSystemProvider "
                       "Can't determine name of Operating System");
    }

    for (unsigned int ii = 0; ii < keys.size(); ii++)
    {
         keyName = keys[ii].getName();

         if ((keyName.equal("CSCreationClassName")) &&
             String::equalNoCase(
                 keys[ii].getValue(),
                 CSCREATIONCLASSNAME.getString()))
         {
             keyCount--;
         }
         else if ((keyName.equal ("CSName")) &&
                  String::equalNoCase(keys[ii].getValue(), csName))
         {
             keyCount--;
         }
         else if ((keyName.equal ("CreationClassName")) &&
                  String::equalNoCase(
                      keys[ii].getValue(),
                      STANDARDOPERATINGSYSTEMCLASS.getString()))
         {
             keyCount--;
         }
         else if ((keyName.equal ("Name")) &&
                  String::equalNoCase(keys[ii].getValue(), name))
         {
             keyCount--;
         }
         else
         {
             throw CIMInvalidParameterException(
                 "OperatingSystemProvider unrecognized key " +
                     keyName.getString());
         }
     }

     if (keyCount)
     {
        throw CIMInvalidParameterException("Wrong keys");
     }

    OSP_DEBUG("losp-> getInstance got the right keys");

    handler.processing();

    //-- fill 'er up...
    instance = _build_instance(ref);
    instance.setPath(ref);

    OSP_DEBUG("losp-> getInstance built an instance");

    handler.deliver(instance);
    handler.complete();

    OSP_DEBUG("losp-> getInstance done");
    return;
}

void OperatingSystemProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    CIMName className;
    CIMInstance instance;
    CIMObjectPath newref;

    className = ref.getClassName();

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    if (className.equal (EXTENDEDOPERATINGSYSTEMCLASS))
    {
        handler.processing();
        newref = _fill_reference(ref.getNameSpace(), className);
        instance = _build_instance(ref);
        instance.setPath(newref);
        handler.deliver(instance);
        handler.complete();
    }
    else if (className.equal (STANDARDOPERATINGSYSTEMCLASS))
    {
        handler.processing();
        handler.complete();
    }
    else
    {
        throw CIMNotSupportedException("OperatingSystemProvider "
                "does not support class " + className.getString());
    }
    return;
}

void OperatingSystemProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath &ref,
    ObjectPathResponseHandler& handler)
{
    CIMObjectPath newref;
    CIMName className;

    // only return instances when enumerate on our subclass, CIMOM
    // will call us as natural part of recursing through subtree on
    // enumerate - if we return instances on enumerate of our superclass,
    // there would be dups
    className = ref.getClassName();
    if (className.equal (STANDARDOPERATINGSYSTEMCLASS))
    {
        handler.processing();
        handler.complete();
        return;
    }
    else if (!className.equal (EXTENDEDOPERATINGSYSTEMCLASS))
    {
        throw CIMNotSupportedException("OperatingSystemProvider "
                       "does not support class " + className.getString());
    }

    // so we know it is for EXTENDEDOPERATINGSYSTEMCLASS
    handler.processing();
    // in terms of the class we use, want to set to what was requested
    newref = _fill_reference(ref.getNameSpace(), className);
    handler.deliver(newref);
    handler.complete();

    return;
}

void OperatingSystemProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const CIMInstance& instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "OperatingSystemProvider does not support modifyInstance");
}

void OperatingSystemProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    const CIMInstance& instanceObject,
    ObjectPathResponseHandler& handler )
{
    throw CIMNotSupportedException(
        "OperatingSystemProvider does not support createInstance");
}

void OperatingSystemProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& ref,
    ResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "OperatingSystemProvider does not support deleteInstance");
}

void OperatingSystemProvider::initialize(CIMOMHandle& handle)
{
   _cimomhandle = handle;  // save off for future use

   // call platform-specific routines to get certain values

}


void OperatingSystemProvider::terminate()
{
    delete this;
}


CIMInstance OperatingSystemProvider::_build_instance(
    const CIMObjectPath& objectReference)
{
    CIMInstance instance(objectReference.getClassName());
    OperatingSystem os;
    CIMName className;
    String stringValue;
    Uint16 uint16Value;
    CIMDateTime cimDateTimeValue;
    Sint16 sint16Value;
    Uint32 uint32Value;
    Uint64 uint64Value;
    Boolean booleanValue;

    className = objectReference.getClassName();

    //-- fill in all the blanks
    instance.addProperty(CIMProperty(
        "CSCreationClassName", CSCREATIONCLASSNAME.getString()));

    if (os.getCSName(stringValue))
    {
        instance.addProperty(CIMProperty("CSName", stringValue));
    }

    instance.addProperty(CIMProperty(
        "CreationClassName", STANDARDOPERATINGSYSTEMCLASS.getString()));

    if (os.getName(stringValue))
    {
        instance.addProperty(CIMProperty("Name", stringValue));
    }

    if (os.getCaption(stringValue))
    {
        instance.addProperty(CIMProperty("Caption", stringValue));
    }

    if (os.getDescription(stringValue))
    {
        instance.addProperty(CIMProperty("Description", stringValue));
    }

    if (os.getInstallDate(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("InstallDate", cimDateTimeValue));
    }

    if (os.getStatus(stringValue))
    {
        instance.addProperty(CIMProperty("Status", stringValue));
    }

    if (os.getOSType(uint16Value))
    {
        instance.addProperty(CIMProperty("OSType", uint16Value));
    }

    if (os.getOtherTypeDescription(stringValue))
    {
        instance.addProperty(CIMProperty("OtherTypeDescription", stringValue));
    }

    if (os.getVersion(stringValue))
    {
        instance.addProperty(CIMProperty("Version", stringValue));
    }

    if (os.getLastBootUpTime(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("LastBootUpTime", cimDateTimeValue));
    }

    if (os.getLocalDateTime(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("LocalDateTime", cimDateTimeValue));
    }

    if (os.getCurrentTimeZone(sint16Value))
    {
        instance.addProperty(CIMProperty("CurrentTimeZone", sint16Value));
    }

    if (os.getNumberOfLicensedUsers(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfLicensedUsers",uint32Value));
    }

    if (os.getNumberOfUsers(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfUsers", uint32Value));
    }

    if (os.getNumberOfProcesses(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfProcesses", uint32Value));
    }

    if (os.getMaxNumberOfProcesses(uint32Value))
    {
        instance.addProperty(CIMProperty("MaxNumberOfProcesses", uint32Value));
    }

    if (os.getTotalSwapSpaceSize(uint64Value))
    {
        instance.addProperty(CIMProperty("TotalSwapSpaceSize", uint64Value));
    }

    if (os.getTotalVirtualMemorySize(uint64Value))
    {
        instance.addProperty(
            CIMProperty("TotalVirtualMemorySize", uint64Value));
    }

    if (os.getFreeVirtualMemory(uint64Value))
    {
        instance.addProperty(CIMProperty("FreeVirtualMemory", uint64Value));
    }

    if (os.getFreePhysicalMemory(uint64Value))
    {
       instance.addProperty(CIMProperty("FreePhysicalMemory", uint64Value));
    }

    if (os.getTotalVisibleMemorySize(uint64Value))
    {
       instance.addProperty(CIMProperty("TotalVisibleMemorySize", uint64Value));
    }

    if (os.getSizeStoredInPagingFiles(uint64Value))
    {
        instance.addProperty(
            CIMProperty("SizeStoredInPagingFiles", uint64Value));
    }

    if (os.getFreeSpaceInPagingFiles(uint64Value))
    {
        instance.addProperty(
            CIMProperty("FreeSpaceInPagingFiles", uint64Value));
    }

    if (os.getMaxProcessMemorySize(uint64Value))
    {
        instance.addProperty(CIMProperty("MaxProcessMemorySize", uint64Value));
    }

    if (os.getDistributed(booleanValue))
    {
        instance.addProperty(CIMProperty("Distributed", booleanValue));
    }

    if (os.getMaxProcsPerUser(uint32Value))
    {
        instance.addProperty(CIMProperty("MaxProcessesPerUser", uint32Value));
    }

    if (className.equal (EXTENDEDOPERATINGSYSTEMCLASS))
    {
        if (os.getSystemUpTime(uint64Value))
        {
            instance.addProperty(CIMProperty("SystemUpTime",uint64Value ));
        }

        if (os.getOperatingSystemCapability(stringValue))
        {
            instance.addProperty(CIMProperty("OperatingSystemCapability",
                                              stringValue));
        }
    }
    return instance;
}

CIMObjectPath OperatingSystemProvider::_fill_reference(
    const CIMNamespaceName &nameSpace,
    const CIMName &className)
{
    Array<CIMKeyBinding> keys;
    OperatingSystem os;
    String csName;
    String name;

    if (!os.getCSName(csName))
    {
        throw CIMOperationFailedException("OperatingSystemProvider "
                  "can't determine name of computer system");
    }

    if (!os.getName(name))
    {
        throw CIMOperationFailedException("OperatingSystemProvider "
                  "can't determine name of Operating System");
    }

    keys.append(CIMKeyBinding(
        "CSCreationClassName",
        CSCREATIONCLASSNAME.getString(),
        CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("CSName", csName, CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding(
        "CreationClassName",
        STANDARDOPERATINGSYSTEMCLASS.getString(),
        CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("Name", name, CIMKeyBinding::STRING));

    return CIMObjectPath(csName, nameSpace, className, keys);
}


void OperatingSystemProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& objectReference,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    MethodResultResponseHandler& handler)
{
    throw CIMNotSupportedException(
        "OperatingSystemProvider does not support invokeMethod");
}

