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
//
// Author: Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//
// Modified By:
//      Chip Vincent (cvincent@us.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Config/ConfigManager.h>

#include <iostream>

PEGASUS_USING_PEGASUS;

PEGASUS_USING_STD;

static Boolean verbose;

void testParseBooleanValue()
{
    PEGASUS_TEST_ASSERT(ConfigManager::parseBooleanValue("true"));
    PEGASUS_TEST_ASSERT(ConfigManager::parseBooleanValue("True"));
    PEGASUS_TEST_ASSERT(ConfigManager::parseBooleanValue("TRUE"));
    PEGASUS_TEST_ASSERT(!ConfigManager::parseBooleanValue("true!"));
    PEGASUS_TEST_ASSERT(!ConfigManager::parseBooleanValue("t"));
    PEGASUS_TEST_ASSERT(!ConfigManager::parseBooleanValue("false"));
    PEGASUS_TEST_ASSERT(!ConfigManager::parseBooleanValue("1"));
}

void testListenAddressParsing(ConfigManager *_config)
{
    _config->useConfigFiles = false;
    if(_config ->initCurrentValue(
        String("listenAddress"),
        String("127.0.0.1,::1")))
    {

        String value = _config ->getCurrentValue("listenAddress");
        Array<String> ips =
            DefaultPropertyOwner::parseAndGetListenAddress(value);
        Array<HostAddress> interfaces = ConfigManager::getListenAddress(value);
        PEGASUS_TEST_ASSERT(interfaces.size() == ips.size());
        for(Uint32 i = 0, n = interfaces.size(); i < n ; ++i)
        {
            PEGASUS_TEST_ASSERT(ips[i] == interfaces[i].getHost());
        }
        if(verbose)
        {
            cout << "listenAddress = ";
            for(Uint32 i = 0, n = interfaces.size(); i < n ; ++i)
            {
                cout << interfaces[i].getHost() << ",";
            }
            cout << endl;
        }
    }
    _config->useConfigFiles = true;
}

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        ConfigManager * _config = 0;

        _config = ConfigManager::getInstance();

        if(_config == 0)
        {
            throw Exception("ConfigManager::getInstance() failed.");
        }

        _config->useConfigFiles = true;

        _config->mergeConfigFiles();

        Array<String> propertyNames;

        _config->getAllPropertyNames(propertyNames, true);

        for(Uint32 i = 0, n = propertyNames.size(); i < n; i++)
        {
            Array<String> info;

            _config->getPropertyInfo(propertyNames[i], info);

            if(verbose)
            {
                cout << "property name = " << propertyNames[i] << endl;

                /*
                cout << "property info = ";

                for(Uint32 j = 0, m = info.size(); j < m; j++)
                {
                    cout << info[j] << " ";
                }

                cout << endl;
                */
            }

            String currentValue = _config->getCurrentValue(propertyNames[i]);

            if(verbose)
            {
                cout << "current value = " << currentValue << endl;
            }

            String plannedValue = _config->getPlannedValue(propertyNames[i]);

            if(verbose)
            {
                cout << "planned value = " << plannedValue << endl;
            }
        }

        testParseBooleanValue();

        //test for listenAddress parsing
        testListenAddressParsing(_config);

    }
    catch(Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;

        return(1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return(0);
}
