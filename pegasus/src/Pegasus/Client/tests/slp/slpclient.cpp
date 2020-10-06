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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMServerDescription.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Client/CIMServerDiscovery.h>
#include <Pegasus/Client/WBEMSLPTemplate.h>
#include <string.h> // for strdup

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int argc, char** argv)
{
#ifdef PEGASUS_ENABLE_SLP
    try
    {
        CIMServerDiscovery disco;

        Array<Attribute> criteria;
        Attribute attr(
            PEG_WBEM_SLP_SERVICE_ID"="PEG_WBEM_SLP_SERVICE_ID_DEFAULT);
        Array<CIMServerDescription> connections;
        SLPClientOptions* opts = (SLPClientOptions*)NULL;
        if (argc == 2)
        {
            // argv[1] should be a DA address
            opts = new SLPClientOptions();
            opts->target_address = strdup(argv[1]);
            opts->scopes=strdup("DEFAULT");
            opts->spi=strdup("");
            opts->use_directory_agent = false;
        }
        connections = disco.lookup(opts);
        if ((SLPClientOptions*)NULL != opts)
        {
            delete opts;
        }

        for (Uint32 i = 0; i < connections.size(); i++)
        {
            cout << "\n======================================================"
                 << endl;
            cout << connections[i].getUrl() << endl;
            cout << "======================================================"
                 << endl;
            Array<Attribute> attributes = connections[i].getAttributes();
            for (Uint32 j = 0; j < attributes.size(); j++)
            {
                cout << "'" << attributes[j] << "'" << endl;
            }
        }

        if (connections.size() == 0)
        {
            if (criteria.size() > 0)
                cout << "Warning: No registered wbem connections found using "
                    "criteria: '" << attr << "'" << endl;
            else
                cout << "Warning: No registered wbem connections found."
                     << endl;
        }
    }
    catch (Exception& e)
    {
        cerr << "Error: " << e.getMessage() << endl;
        exit(1);
    }

    cout << "+++++ passed all tests" << endl;
#else
    cout << "+++++ PEGASUS_ENABLE_SLP *not* set during the pegasus build"
         << endl;
#endif // PEGASUS_ENABLE_SLP

    return 0;
}
