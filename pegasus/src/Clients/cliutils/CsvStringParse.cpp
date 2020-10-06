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

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Config.h>
#include "CsvStringParse.h"

PEGASUS_NAMESPACE_BEGIN
 /* Define a string to parse for comma separated values and the
       separation character
    */
    csvStringParse::csvStringParse(
        const String& csvString,
        const int separator)
    {
        _inputString = csvString;
        _separator = separator;
        _idx = 0;
        _end = csvString.size();
    }

 /* determine if there is more to parse
       @return true if there is more to parse
    */
    Boolean csvStringParse::more()
    {
        return (_idx < _end)? true : false;
    }


/* get next string from input. Note that this will continue to
       return empty strings if you parse past the point where more()
       returns false.
       @return String
    */
    String csvStringParse::next()
    {
        String rtnValue;
        parsestate state = NOTINQUOTE;

        while ((_idx <= _end) && (_inputString[_idx]))
        {
            char idxchar = _inputString[_idx];
            switch (state)
            {
                case NOTINQUOTE:
                    switch (idxchar)
                    {
                        case '\\':
                            state = INSQUOTE;
                            break;

                        case '"':
                            state = INDQUOTE;
                            break;

                        default:
                            if (idxchar == _separator)
                            {
                                _idx++;
                                return rtnValue;
                            }
                            else
                            {
                                rtnValue.append(idxchar);
                            }
                            break;
                    }
                    break;

                // add next character and set NOTINQUOTE State
                case INSQUOTE:
                    rtnValue.append(idxchar);
                    state = NOTINQUOTE;
                    break;

                // append all but quote character
                case INDQUOTE:
                    switch (idxchar)
                    {
                        case '"':
                            state = NOTINQUOTE;
                            break;
                        default:
                            rtnValue.append(idxchar);
                            break;
                    }
            }
            _idx++;
        }   // end while

        return rtnValue;
    }

PEGASUS_NAMESPACE_END
