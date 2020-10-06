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


/* Test program for the base64 class
*/


#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Base64.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/InternalException.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main(int, char** argv)
{
    Boolean verbose = (getenv("PEGASUS_TEST_VERBOSE")) ? true : false;
    Uint32 maxLength = 1000;

    // Test a simple decode to determine if decode correct
    {
        Buffer in;
        Buffer out;
        // Create an array of the character A 4 times.
        in.append('A');
        in.append('A');
        in.append('A');
        in.append('A');
        out = Base64::decode(in);
        // Should produce 3 bytes of zeros
        PEGASUS_TEST_ASSERT(in.size() == 4);
        out = Base64::decode(in);
        PEGASUS_TEST_ASSERT(out.size() == 3);
        // The result should be all zeros.
        for (Uint32 i = 0; i < out.size(); i++)
            if (out[i] != 0)
            {
                if(verbose)
                    cout <<"Decode error" << endl;
            }
    }
    // Test a simple decode to determine if decode correct
    {
        Buffer in;
        Buffer out;
        // Create an array of the character A 4 times.
        in.append('/');
        in.append('/');
        in.append('/');
        in.append('/');
        out = Base64::decode(in);
        // Should produce 3 bytes of -1s
        PEGASUS_TEST_ASSERT(in.size() == 4);
        out = Base64::decode(in);
        PEGASUS_TEST_ASSERT(out.size() == 3);
        // The result should be all zeros.
        for (Uint32 i = 0; i < out.size(); i++)
            if (out[i] != char(0xFF)) //char 255
            {
                if(verbose)
                 cout << "Decode error " << static_cast<int>(out[i]) << endl;
            }
    }



    // Execute a set of fixed coding tests. Start from Uint buffer
    // and determine if correct character set created
    {
        // Test for creation of char A
        Buffer in;
        Buffer out;

        // Create Array of 3 zeros.
        in.append(0);
        in.append(0);
        in.append(0);

        // confirm that the character A is created, 6 characters

        out = Base64::encode(in);
        PEGASUS_TEST_ASSERT(out.size() == 4);
        for (Uint32 i = 0; i <out.size(); i++ )
        {
            PEGASUS_TEST_ASSERT(out[i] == 'A');
        }
    }

    if(verbose)
        cout << "Code and decode tests. Test 1 - all characters in buffer"
                << endl;
    {
        // Repeat test for buffer size from 0 to 1000 characters long
        for (Uint32 i = 0; i < maxLength; i++)
        {
            //build an array i characters long with integers 0 - 63

            Buffer in;
            Buffer out;

            Uint8 k = 0;

            // Build the array from assending set of characters
            in.append(k++);
            Buffer temp = Base64::encode(in);
            out = Base64::decode(temp);

            PEGASUS_TEST_ASSERT(in == out);

            /* Detailed error testing if we need to turn it on

            if (in.size() != out.size())
            {
                cout << "size error" << endl;
            }
            for (Uint32 l = 0; l < out.size(); l++)
                if (in[l] != out[l])
                    cout << "Error in something" << endl;
            if (!(in == out))
               cout << "Problem with Base64 Equal test" << endl;
            */
        }
    }

    // Test with buffer of all zeros
    if(verbose)
        cout << "Test 2 -Test all zero buffer" << endl;
    {
        Buffer in;
        Buffer out;

        for (Uint32 i = 0; i < maxLength; i++)
        {
            in.append(0);
            Buffer temp = Base64::encode(in);
            out = Base64::decode(temp);
            PEGASUS_TEST_ASSERT(in == out);
        }
    }


    // Test with buffer of char 0xFF (255)
    if(verbose)
        cout << "Test 3 -- Test all 1s in buffer" << endl;
    {
        Buffer in;
        Buffer out;

        for (Uint32 i = 0; i < maxLength; i++)
        {
            in.append('\xFF');
            Buffer temp = Base64::encode(in);
            out = Base64::decode(temp);

            PEGASUS_TEST_ASSERT(in == out);
        }
    }

    // One more
    {
        Buffer in;
        Buffer out;

        for (Uint32 i = 0; i < maxLength; i++)
        {
            in.append('\xFF');
            out = Base64::decode(Base64::encode(in));

            PEGASUS_TEST_ASSERT(in == out);
        }

    }
    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;
}

