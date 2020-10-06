//%LICENSE////////////////////////////////////////////////////////////////
////
//// Licensed to The Open Group (TOG) under one or more contributor license
//// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
//// this work for additional information regarding copyright ownership.
//// Each contributor licenses this file to you under the OpenPegasus Open
//// Source License; you may not use this file except in compliance with the
//// License.
////
//// Permission is hereby granted, free of charge, to any person obtaining a
//// copy of this software and associated documentation files (the "Software"),
//// to deal in the Software without restriction, including without limitation
//// the rights to use, copy, modify, merge, publish, distribute, sublicense,
//// and/or sell copies of the Software, and to permit persons to whom the
//// Software is furnished to do so, subject to the following conditions:
////
//// The above copyright notice and this permission notice shall be included
//// in all copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
/////////////////////////////////////////////////////////////////////////////
#ifndef Pegasus_CsvStringParse_h
#define Pegasus_CsvStringParse_h

#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Config.h>
#include <Clients/cliutils/Linkage.h>

PEGASUS_NAMESPACE_BEGIN
/******************************************************************************
   Parser for comma-separated-strings (csv). This parser takes into
   account quoted strings the " character and returns everything
   within a quoted block in the string in one batch.  It also
   considers the backslash "\" escape character to escape single
   double quotes.
   Example:
     csvStringParse x (inputstring, ',');
     while (x.more())
        rtnString = x.next();
******************************************************************************/
class PEGASUS_CLIUTILS_LINKAGE csvStringParse
{
public:
    /* Define a string to parse for comma separated values and the
       separation character
    */
    csvStringParse(const String& csvString, const int separator);

    /* determine if there is more to parse
       @return true if there is more to parse
    */
    Boolean more();

    /* get next string from input. Note that this will continue to
       return empty strings if you parse past the point where more()
       returns false.
       @return String
    */
    String next();

private:
    enum parsestate {INDQUOTE, INSQUOTE, NOTINQUOTE};
    Uint32 _idx;
    int _separator;
    Uint32 _end;
    String _inputString;
};
PEGASUS_NAMESPACE_END

#endif
