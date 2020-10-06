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

#include <Pegasus/Common/PegasusAssert.h>
#include <cstdio>
#include <Pegasus/Config/ConfigFileHandler.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

int main()
{
    try
    {
        Array<CIMName> all;
        Array<String> values;
        String value;

        String currentFile;
        String plannedFile;
        const char* tmpDir = getenv ("PEGASUS_TMP");
        if (tmpDir == NULL)
        {
            currentFile = ".";
            plannedFile = ".";
        }
        else
        {
            currentFile = tmpDir;
            plannedFile = tmpDir;
        }
        currentFile.append("/cimserver_current.conf");
        plannedFile.append("/cimserver_planned.conf");

        ConfigFileHandler _config (currentFile, plannedFile, true);

        _config.loadAllConfigProperties();

        _config.getAllCurrentPropertyNames(all);

        for (Uint32 i = 0; i < all.size(); i++)
        {
            _config.getCurrentValue(all[i], value);
            _config.getPlannedValue(all[i], value);
        }

        all.clear();

        _config.getAllPlannedProperties(all, values);

        for (Uint32 i = 0; i < all.size(); i++)
        {
            _config.getPlannedValue(all[i], value);
        }

        all.clear();
        values.clear();

        _config.getAllCurrentProperties(all, values);

        for (Uint32 i = 0; i < all.size(); i++)
        {
            _config.getCurrentValue(all[i], value);
        }

    }
    catch (Exception& e)
    {
        cerr << "Exception: " << e.getMessage() << endl;
        exit(1);
    }

    cout << "+++++ passed all tests" << endl;

    return 0;
}

