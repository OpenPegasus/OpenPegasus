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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Server/ProviderRegistrationManager/\
ProviderRegistrationManager.h>

#include "ProviderIndicationCountTable.h"

PEGASUS_NAMESPACE_BEGIN

Uint32 ProviderIndicationCountTable::_ProviderIndicationCountHashFunc::hash(
    const String& key)
{
    Uint32 hashCode = 0;

    const Uint16* p = (const Uint16*)key.getChar16Data();
    Uint32 keySize = key.size();

    if (keySize > 1)
    {
        hashCode = p[0] + p[keySize/2] + 3*p[keySize - 1];
    }

    return hashCode;
}

ProviderIndicationCountTable::ProviderIndicationCountTable()
{
}

ProviderIndicationCountTable::~ProviderIndicationCountTable()
{
}

void ProviderIndicationCountTable::clear()
{
    _table.clear();
}

void ProviderIndicationCountTable::insertEntry(
    const CIMInstance& providerInstance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::insertEntry");

    String providerModuleName;
    String providerName;
    getProviderKeys(providerInstance, providerModuleName, providerName);

    String providerKey = _generateKey(providerModuleName, providerName);
    _ProviderIndicationCountTableEntry entry;

    WriteLock lock(_tableLock);

    if (!_table.lookup(providerKey, entry))
    {
        //
        // The entry is not in the table yet; insert a new entry.
        //
        _ProviderIndicationCountTableEntry newEntry;
        newEntry.providerModuleName = providerModuleName;
        newEntry.providerName = providerName;
        newEntry.indicationCount = 0;
        newEntry.orphanIndicationCount = 0;

        PEGASUS_FCT_EXECUTE_AND_ASSERT(
            true,
            _table.insert(providerKey, newEntry));
    }

    PEG_METHOD_EXIT();
}

void ProviderIndicationCountTable::incrementEntry(
    const CIMInstance& providerInstance,
    Boolean isOrphan)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::incrementEntry");

    String providerModuleName;
    String providerName;
    getProviderKeys(providerInstance, providerModuleName, providerName);

    String providerKey = _generateKey(providerModuleName, providerName);
    _ProviderIndicationCountTableEntry* entry = 0;

    WriteLock lock(_tableLock);

    if (_table.lookupReference(providerKey, entry))
    {
        entry->indicationCount++;

        if (isOrphan)
        {
            entry->orphanIndicationCount++;
        }
    }

    PEG_METHOD_EXIT();
}

void ProviderIndicationCountTable::removeEntry(
    const CIMInstance& providerInstance)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::removeEntry");

    String providerModuleName;
    String providerName;
    getProviderKeys(providerInstance, providerModuleName, providerName);
    String providerKey = _generateKey(providerModuleName, providerName);

    WriteLock lock(_tableLock);
    _table.remove(providerKey);

    PEG_METHOD_EXIT();
}

void ProviderIndicationCountTable::removeModuleEntries(
    const String& providerModuleName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::removeModuleEntries");

    {
        WriteLock lock(_tableLock);
        Array<String> keysToRemove;

        // First collect a list of ProviderIndicationCountTable entries for
        // this provider module.
        for (_ProviderIndicationCountTable::Iterator i = _table.start(); i; i++)
        {
            if (i.value().providerModuleName == providerModuleName)
            {
                keysToRemove.append(i.key());
            }
        }

        // Now remove the entries, outside the Iterator scope.
        for (Uint32 i = 0; i < keysToRemove.size(); i++)
        {
            PEGASUS_FCT_EXECUTE_AND_ASSERT(true,_table.remove(keysToRemove[i]));
        }
    }

    PEG_METHOD_EXIT();
}

void ProviderIndicationCountTable::getProviderKeys(
    const CIMInstance& providerInstance,
    String& providerModuleName,
    String& providerName)
{
    Array<CIMKeyBinding> keys = providerInstance.getPath().getKeyBindings();

    for (Uint32 i = 0; i < keys.size(); i++)
    {
        if (keys[i].getName() == PEGASUS_PROPERTYNAME_NAME)
        {
            providerName = keys[i].getValue();
        }
        else if (keys[i].getName() == _PROPERTY_PROVIDERMODULENAME)
        {
            providerModuleName = keys[i].getValue();
        }
    }
}

Array<ProviderIndicationCountTable::_ProviderIndicationCountTableEntry>
    ProviderIndicationCountTable::_getAllEntries()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::_getAllEntries");

    Array <_ProviderIndicationCountTableEntry> providerIndicationCountEntries;

    //
    // Iterate through the ProviderIndicationCountTable to get all the entries.
    //

    {
        ReadLock lock(_tableLock);
        for (_ProviderIndicationCountTable::Iterator i = _table.start(); i; i++)
        {
            providerIndicationCountEntries.append(i.value());
        }
    }

    PEG_METHOD_EXIT();
    return providerIndicationCountEntries;
}

String ProviderIndicationCountTable::_generateKey(
    const String& providerModuleName,
    const String& providerName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::_generateKey");

    String providerIndicationCountKey(providerName);
    providerIndicationCountKey.append(providerModuleName);
    providerIndicationCountKey.append(":");

    char buffer[22];
    Uint32 length;
    const char* providerNameSize =
        Uint32ToString(buffer, providerName.size(), length);
    providerIndicationCountKey.append(providerNameSize, length);

    PEG_METHOD_EXIT();
    return providerIndicationCountKey;
}

Array<CIMInstance>
    ProviderIndicationCountTable::enumerateProviderIndicationDataInstances()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::"
            "enumerateProviderIndicationDataInstances");

    Array<CIMInstance> instances;

    //
    // get entire provider indication count table entries
    //
    Array<_ProviderIndicationCountTableEntry> indicationCountEntries =
        _getAllEntries();

    for (Uint32 i = 0; i < indicationCountEntries.size(); i++)
    {
        CIMInstance providerIndDataInstance = _buildProviderIndDataInstance(
            indicationCountEntries[i]);
        instances.append(providerIndDataInstance);
    }

    PEG_METHOD_EXIT();
    return instances;
}

Array<CIMObjectPath>
    ProviderIndicationCountTable::enumerateProviderIndicationDataInstanceNames()
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::"
            "enumerateProviderIndicationDataInstanceNames");

    Array<CIMObjectPath> instanceNames;

    //
    // get entire provider indication count table entries
    //
    Array<_ProviderIndicationCountTableEntry> indicationCountEntries =
        _getAllEntries();

    for (Uint32 i = 0; i < indicationCountEntries.size(); i++)
    {
        CIMObjectPath path = _buildProviderIndDataInstanceName(
            indicationCountEntries[i]);

        instanceNames.append(path);
    }

    PEG_METHOD_EXIT();
    return instanceNames;
}

CIMInstance ProviderIndicationCountTable::getProviderIndicationDataInstance(
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::getProviderIndicationDataInstance");

    //
    // Gets provider module name and provider name from the provider indication
    // data instance object path
    //
    String providerModuleName;
    String providerName;
    Array<CIMKeyBinding> keys = instanceName.getKeyBindings();

    for (Uint32 i = 0; i < keys.size(); i++)
    {
        if (keys[i].getName() == _PROPERTY_PROVIDERNAME)
        {
            providerName = keys[i].getValue();
        }
        else if (keys[i].getName() == _PROPERTY_PROVIDERMODULENAME)
        {
            providerModuleName = keys[i].getValue();
        }
    }

    String providerKey = _generateKey(providerModuleName, providerName);

    _ProviderIndicationCountTableEntry entry;

    WriteLock lock(_tableLock);

    if (_table.lookup(providerKey, entry))
    {
        CIMInstance providerIndDataInstance =
            _buildProviderIndDataInstance(entry);

        PEG_METHOD_EXIT();
        return providerIndDataInstance;
    }

    PEG_METHOD_EXIT();
    throw CIMObjectNotFoundException(instanceName.toString());
}

CIMObjectPath ProviderIndicationCountTable::_buildProviderIndDataInstanceName(
    const _ProviderIndicationCountTableEntry& indicationCountEntry)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::_buildProviderIndDataInstanceName");

    CIMObjectPath instanceName;
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding(
        "ProviderModuleName",
        indicationCountEntry.providerModuleName,
        CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding(
        "ProviderName",
        indicationCountEntry.providerName,
        CIMKeyBinding::STRING));

    instanceName.setClassName(PEGASUS_CLASSNAME_PROVIDERINDDATA);
    instanceName.setKeyBindings(keyBindings);

    PEG_METHOD_EXIT();
    return instanceName;
}

CIMInstance ProviderIndicationCountTable::_buildProviderIndDataInstance(
    const _ProviderIndicationCountTableEntry& indicationCountEntry)
{
    PEG_METHOD_ENTER(TRC_INDICATION_SERVICE,
        "ProviderIndicationCountTable::_buildProviderIndDataInstance");

    CIMInstance providerIndDataInstance(PEGASUS_CLASSNAME_PROVIDERINDDATA);
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderModuleName"),
        indicationCountEntry.providerModuleName));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("ProviderName"),
        indicationCountEntry.providerName));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("IndicationCount"),
        indicationCountEntry.indicationCount));
    providerIndDataInstance.addProperty(CIMProperty(
        CIMName("OrphanIndicationCount"),
        indicationCountEntry.orphanIndicationCount));

    CIMObjectPath path = _buildProviderIndDataInstanceName(
        indicationCountEntry);
    providerIndDataInstance.setPath(path);

    PEG_METHOD_EXIT();
    return providerIndDataInstance;
}

PEGASUS_NAMESPACE_END
