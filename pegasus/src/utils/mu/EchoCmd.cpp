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
// Author: Michael E. Brasher
//
//%=============================================================================

#include "EchoCmd.h"
#include <iostream>

int EchoCmd(const vector<string>& args)
{
    for (size_t i = 1; i < args.size(); i++)
    {
        cout << args[i];

        if (i + 1 != args.size())
            cout << ' ';
    }

    cout << endl;

    return 0;
}


// EchoWithEscapeCmd (echo-e) supports the following
// limited set of escape sequences.
//       \a     alert (BEL)
//       \b     backspace
//       \c     suppress trailing newline
//       \n     new line
//       \r     carriage return
//       \t     horizontal tab

int EchoWithEscapeCmd(const vector<string>& args)
{
    for (size_t i = 1; i < args.size(); i++)
    {
        string text = args[i];
        int textLen = static_cast<int>(text.size());

        int j = 0;
        while (j < textLen - 1)
        {
             if ((text[j] == '\\') && (text[j+1] != '\\'))
             {
                   j++;
                   switch (text[j])
                   {
                       case 'a':
                           cout << "\a";
                           break;
                       case 'b':
                           cout << "\b";
                           break;
                       case 'c':
                           return 0;
                       case 'n':
                           cout << "\n";
                           break;
                       case 'r':
                           cout << "\r";
                           break;
                       case 't':
                           cout << "\t";
                           break;
                       default:
                           cout << "\\" << text[j];
                    }
             }
             else
             {
                 cout << text[j];
             }
             j++;
        }
        if (j == textLen - 1)
           cout << text[j];

        if (i + 1 != args.size())
           cout << ' ';
     }
     cout << endl;
     return 0;
}
