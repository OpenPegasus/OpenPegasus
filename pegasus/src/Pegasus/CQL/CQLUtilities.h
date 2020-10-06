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
// Authors: David Rosckes (rosckes@us.ibm.com)
//          Bert Rivero (hurivero@us.ibm.com)
//          Chuck Carmack (carmack@us.ibm.com)
//          Brian Lucier (lucier@us.ibm.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//
//%/////////////////////////////////////////////////////////////////////////////
#ifndef Pegasus_CQLUtilities_h
#define Pegasus_CQLUtilities_h


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

// Forward declares
class String;

class CQLUtilities
{
public:
  static Uint64 stringToUint64(const String &stringNum);
  static Sint64 stringToSint64(const String &stringNum);
  static Real64 stringToReal64(const String &stringNum);

  // This will take a formated string representing a real number and if
  // it uses scientific notiation, then it will strip off leading 0's
  // in the exponent.  If the exponent only has 0's, then it will strip
  // the exponent symbol also.  (ie  "2.56E-00" becomes "2.56"; "2.56E-04"
  // becomes "2.56E-4".  If there is no exponent, then nothing is changed.
  // This will also remove a leading '+' sign from the exponent if it is
  // there.
  static String formatRealStringExponent(const String &realString);
  static const char  KEY[];
  static const char  MSG[];
private:
  // This is a helper function which checks if a string has a decimal
  // point in it.  If so, it calls it a real.  In the case of a badly
  // formated string, it will fail the format check in StringToReal64.
  static Boolean isReal(const String &numString);
};

PEGASUS_NAMESPACE_END
#endif
#endif
