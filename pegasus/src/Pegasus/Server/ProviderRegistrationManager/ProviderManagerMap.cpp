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

#include "ProviderManagerMap.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/String.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/Common/Dir.h>


PEGASUS_NAMESPACE_BEGIN


ProviderManagerMap& ProviderManagerMap::instance()
{
    static ProviderManagerMap singletonInst;
    return singletonInst;
}


ProviderManagerMap::ProviderManagerMap()
{
    initialize();
}

bool ProviderManagerMap::isValidProvMgrIfc(String &ifcType, String &ifcVersion)
{
    for (Uint32 ifc=0; ifc<_pmArray.size(); ifc++)
    {
        if (_pmArray[ifc].ifcName == ifcType)
        {
            if (ifcVersion.size()==0)
            {
                return true;
            }
            else
            {
                for (Uint32 ver=0; ver<_pmArray[ifc].ifcVersions.size(); ver++)
                {
                    if (_pmArray[ifc].ifcVersions[ver] == ifcVersion)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool ProviderManagerMap::getProvMgrPathForIfcType(
    const String &ifcType,
    const String &ifcVersion,
    Uint16 bitness,
    String &path)
{
    path.clear();
    for (Uint32 ifc=0; ifc<_pmArray.size(); ifc++)
    {
        if (_pmArray[ifc].ifcName == ifcType)
        {
            if (ifcVersion.size()==0)
            {
                path = bitness == PG_PROVMODULE_BITNESS_32 ?
                    _pmArray[ifc].path32 :  _pmArray[ifc].path;
                return true;
            }
            else for (Uint32 ver=0; ver<_pmArray[ifc].ifcVersions.size(); ver++)
            {
                if (_pmArray[ifc].ifcVersions[ver] == ifcVersion)
                {
                    path = bitness == PG_PROVMODULE_BITNESS_32 ?
                        _pmArray[ifc].path32 : _pmArray[ifc].path;
                    return true;
                }
            }
        }
    }
    return false;
}


void ProviderManagerMap::initialize()
{
    String libExt = FileSystem::getDynamicLibraryExtension();
    // first add the default:

    ProvMgrIfcInfo defaultPMEntry;
    defaultPMEntry.path.clear();
    defaultPMEntry.ifcName = "C++Default";
    defaultPMEntry.ifcVersions.append(String("2.1.0"));
    defaultPMEntry.ifcVersions.append(String("2.2.0"));
    defaultPMEntry.ifcVersions.append(String("2.3.0"));
    defaultPMEntry.ifcVersions.append(String("2.5.0"));
    defaultPMEntry.ifcVersions.append(String("2.6.0"));
    defaultPMEntry.ifcVersions.append(String("2.9.0"));
    _pmArray.append(defaultPMEntry);

    // now check for plugins

    String dirName = ConfigManager::getInstance()->getCurrentValue(
            "providerManagerDir");
    dirName = ConfigManager::getHomedPath(dirName);

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
        "Looking for ProviderManagers in %s.",
        (const char*)dirName.getCString()));

    // check to make sure that this ifc type is handled by one of the
    // provider managers in the directory
    String testname = String("providermanager")+libExt;
    for (Dir dir(dirName); dir.more(); dir.next())
    {
        String filename = dir.getName();
        String lowerFilename = filename;
        lowerFilename.toLower();
        if ((lowerFilename.subString(lowerFilename.size()-testname.size()) ==
                 testname) &&
            (lowerFilename !=
                 FileSystem::buildLibraryFileName("defaultprovidermanager")) &&
            (lowerFilename !=
                 FileSystem::buildLibraryFileName("pegprovidermanager")))
        {
            String fullPath = dirName + "/" + filename;
#ifdef PEGASUS_PROVIDER_MANAGER_32BIT_LIB_DIR
            String fullPath32 = PEGASUS_PROVIDER_MANAGER_32BIT_LIB_DIR;
            fullPath32 = fullPath32 + "/" + filename;
#else
            String fullPath32 = dirName + "32/" + filename;
#endif
            // found a file... assume it's a ProviderManager library
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Found file %s. Checking to see if it is a ProviderManager.",
                (const char*)fullPath.getCString()));
            DynamicLibrary dl(fullPath);
            if (!dl.load())
            {
                Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ProviderRegistrationManager.ProviderManagerMap."
                            "LOAD_ERROR",
                        "Error loading library $0: $1.",
                        fullPath, dl.getLoadErrorMessage()));
                continue;    // to the next file
            }

            Uint32 (*get_peg_ver)() =
                (Uint32(*)()) dl.getSymbol("getPegasusVersion");

            if (get_peg_ver == 0)
            {
                Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ProviderRegistrationManager.ProviderManagerMap."
                            "MISSING_GET_PG_VERSION",
                        "Library $0 does not contain expected function "
                            "'getPegasusVersion'.",
                        fullPath));
                continue;
            }

            Uint32 peg_ver = get_peg_ver();
            if (peg_ver != PEGASUS_VERSION_NUMBER)
            {
                Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ProviderRegistrationManager.ProviderManagerMap."
                            "WRONG_VERSION",
                        "Provider Manager $0 returned Pegasus "
                            "version $1.  Expected $2.",
                        fullPath, peg_ver, PEGASUS_VERSION_NUMBER));
                continue;
            }

            const char** (*get_ifc)() = (const char**(*)()) dl.getSymbol(
                "getProviderManagerInterfaceNames");
            const char** (*get_ver)(const char *) =
                (const char**(*)(const char *)) dl.getSymbol(
                    "getProviderManagerInterfaceVersions");
            if (get_ifc == 0)
            {
                Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ProviderRegistrationManager.ProviderManagerMap."
                            "MISSING_GET_IFC_NAMES",
                        "Provider Manager $0 does not contain expected "
                            "function 'getProviderManagerInterfaceNames'",
                        fullPath));
                continue;    // to the next file
            }
            if (get_ver == 0)
            {
                Logger::put_l(Logger::ERROR_LOG, System::CIMSERVER,
                    Logger::SEVERE,
                    MessageLoaderParms(
                        "Server.ProviderRegistrationManager.ProviderManagerMap."
                            "MISSING_GET_IFC_VERSIONS",
                        "Provider Manager $0 does not contain expected "
                            "function 'getProviderManagerInterfaceVersions'",
                        fullPath));
                continue;    // to the next file
            }

            const char ** ifcNames = get_ifc();
            if ((ifcNames!=NULL) && (*ifcNames!=NULL))
            {
                for (int i=0; ifcNames[i]!=NULL; i++)
                {
                    const char *ifcName = ifcNames[i];

                    ProvMgrIfcInfo entry;
                    entry.path = fullPath;
                    entry.path32 = fullPath32;
                    entry.ifcName = ifcName;

                    // now get the versions
                    const char ** ifcVersions = get_ver(ifcName);
                    for (int j=0; ifcVersions[j]!=NULL; j++)
                    {
                        entry.ifcVersions.append(String(ifcVersions[j]));
                        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                            "Adding Provider type %s version %s "
                            "handled by ProviderManager %s",
                             ifcName,ifcVersions[j],
                             (const char*)fullPath.getCString()));
                    }
                    _pmArray.append(entry);
                }
            }
        }
    }
    _bInitialized = true;
}

PEGASUS_NAMESPACE_END

