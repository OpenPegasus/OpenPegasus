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

#include <cstdlib>
#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/CQL/CQLChainedIdentifier.h>
#include <Pegasus/CQL/CQLIdentifier.h>
#include <Pegasus/Query/QueryCommon/SubRange.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void print(CQLIdentifier &_id){
    cout << "Name = " << _id.getName().getString() << endl;
    if(_id.isScoped())
        cout << "Scope = " << _id.getScope() << endl;
    if(_id.isSymbolicConstant())
        cout << "Symbolic Constant = " << _id.getSymbolicConstantName() << endl;
    if(_id.isWildcard())
        cout << "CQLIdentifier = *" << endl;
    if(_id.isArray()){
        cout << "SubRanges: ";
        Array<SubRange> _ranges = _id.getSubRanges();
        for(Uint32 i = 0; i < _ranges.size(); i++){
            cout << _ranges[i].toString() << ",";
        }
        cout << endl;
    }
}

void drive_CQLIdentifier(){
    CQLIdentifier _ID1("ID1");
    PEGASUS_TEST_ASSERT(_ID1.getName() == "ID1");

    CQLIdentifier _ID2("ID2");
    PEGASUS_TEST_ASSERT(_ID1 != _ID2);

    CQLIdentifier _ID3("*");
    PEGASUS_TEST_ASSERT(_ID3.isWildcard());

    CQLIdentifier scopedID("SCOPE::IDENTIFIER");
    PEGASUS_TEST_ASSERT(scopedID.isScoped());
    PEGASUS_TEST_ASSERT(scopedID.getScope() == "SCOPE");

    CQLIdentifier _ID4("A::Name");
    CQLIdentifier _ID4a("A::Name");
    PEGASUS_TEST_ASSERT(_ID4 == _ID4a);

    CQLIdentifier symbolicConstantID("Name#OK");
    PEGASUS_TEST_ASSERT(symbolicConstantID.getName() == "Name");
    PEGASUS_TEST_ASSERT(symbolicConstantID.isSymbolicConstant());
    PEGASUS_TEST_ASSERT(symbolicConstantID.getSymbolicConstantName() == "OK");

   try{
    CQLIdentifier rangeID("SCOPE::Name[5,6,7]");
    // Basic query check
    PEGASUS_TEST_ASSERT(false);
    PEGASUS_TEST_ASSERT(rangeID.getName() == "Name");
        PEGASUS_TEST_ASSERT(rangeID.isArray());
        Array<SubRange> subRanges = rangeID.getSubRanges();
    PEGASUS_TEST_ASSERT(subRanges[0] == String("5"));
    PEGASUS_TEST_ASSERT(subRanges[1] == String("6"));
    PEGASUS_TEST_ASSERT(subRanges[2] == String("7"));
        PEGASUS_TEST_ASSERT(rangeID.getScope() == "SCOPE");
   }catch(CQLIdentifierParseException&)
   {
   }

   try
   {
     CQLIdentifier rangeID("SCOPE::Name[5..,6..,..7,4-5,..]");
     PEGASUS_TEST_ASSERT(false);
   }
   catch (QueryParseException&)
   {
   }

   try
   {
     CQLIdentifier rangeID1("Name[*]");
     PEGASUS_TEST_ASSERT(false);
   }
   catch (QueryParseException&)
   {
   }

   try
   {
     CQLIdentifier invalid("Name#OK[4-5]");
     PEGASUS_TEST_ASSERT(false);
   }
   catch (CQLIdentifierParseException&)
   {
   }


   try
   {
     CQLIdentifier invalid1("Name[4-5]#OK");
     PEGASUS_TEST_ASSERT(false);
   }
   catch (CQLIdentifierParseException&)
   {
   }
}

void drive_CQLChainedIdentifier()
{
  // Error cases
  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[*]");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (QueryParseException&)
  {
    // do nothing, exception is expected due to wildcard
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[1,3-5,7]");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (QueryParseException&)
  {
    // do nothing, exception is expected due to range
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[1..3]");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (QueryParseException&)
  {
    // do nothing, exception is expected due to range
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[]");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (QueryParseException&)
  {
    // do nothing, exception is expected due to missing index
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[3]#'ok'");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (CQLIdentifierParseException&)
  {
    // do nothing, exception is expected due to combo of array
    // and symbolic constant.
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::PROP[3");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (CQLIdentifierParseException&)
  {
    // do nothing, exception is expected due to missing ']'
  }

  try
  {
    CQLChainedIdentifier _CI("CLASS.B::EO.A::this-is-bogus");
   PEGASUS_TEST_ASSERT(false);
  }
  catch (CQLIdentifierParseException&)
  {
    // do nothing, exception is expected due to missing bad prop name
  }

  // Good case with all the bells and whistles, except wildcard
  //CQLChainedIdentifier _CI(
  //"FROMCLASS.SCOPE1::EO1.SCOPE2::EO2[1,3,5,7].SCOPE3::PROP#'ok'");
  CQLChainedIdentifier _CI(
          "FROMCLASS.SCOPE1::EO1.SCOPE2::EO2[3].SCOPE3::PROP#'ok'");

  Array<CQLIdentifier> _arr = _CI.getSubIdentifiers();
  PEGASUS_TEST_ASSERT(_arr.size() == 4);

  PEGASUS_TEST_ASSERT(_arr[0].getName() == "FROMCLASS");
  PEGASUS_TEST_ASSERT(!_arr[0].isScoped());
  PEGASUS_TEST_ASSERT(!_arr[0].isSymbolicConstant());
  PEGASUS_TEST_ASSERT(!_arr[0].isArray());
  PEGASUS_TEST_ASSERT(!_arr[0].isWildcard());

  PEGASUS_TEST_ASSERT(_arr[1].getName() == "EO1");
  PEGASUS_TEST_ASSERT(_arr[1].isScoped());
  PEGASUS_TEST_ASSERT(_arr[1].getScope() == "SCOPE1");
  PEGASUS_TEST_ASSERT(!_arr[1].isSymbolicConstant());
  PEGASUS_TEST_ASSERT(!_arr[1].isArray());
  PEGASUS_TEST_ASSERT(!_arr[1].isWildcard());

  PEGASUS_TEST_ASSERT(_arr[2].getName() == "EO2");
  PEGASUS_TEST_ASSERT(_arr[2].isScoped());
  PEGASUS_TEST_ASSERT(_arr[2].getScope() == "SCOPE2");
  PEGASUS_TEST_ASSERT(!_arr[2].isSymbolicConstant());
  PEGASUS_TEST_ASSERT(_arr[2].isArray());
  Array<SubRange> ranges = _arr[2].getSubRanges();
  PEGASUS_TEST_ASSERT(ranges.size() == 1);
  //PEGASUS_TEST_ASSERT(ranges[0] == String("1"));
  PEGASUS_TEST_ASSERT(ranges[0] == String("3"));
  //PEGASUS_TEST_ASSERT(ranges[2] == String("5"));
  //PEGASUS_TEST_ASSERT(ranges[3] == String("7"));
  PEGASUS_TEST_ASSERT(!_arr[2].isWildcard());

  PEGASUS_TEST_ASSERT(_arr[3].getName() == "PROP");
  PEGASUS_TEST_ASSERT(_arr[3].isScoped());
  PEGASUS_TEST_ASSERT(_arr[3].getScope() == "SCOPE3");
  PEGASUS_TEST_ASSERT(_arr[3].isSymbolicConstant());
  PEGASUS_TEST_ASSERT(_arr[3].getSymbolicConstantName() == "'ok'");
  PEGASUS_TEST_ASSERT(!_arr[3].isArray());
  PEGASUS_TEST_ASSERT(!_arr[3].isWildcard());

  // Good case with all the bells and whistles, and wildcard
  CQLChainedIdentifier _CI1("FROMCLASS.SCOPE1::EO1.SCOPE2::EO2[7].*");

  _arr = _CI1.getSubIdentifiers();
  PEGASUS_TEST_ASSERT(_arr.size() == 4);

  PEGASUS_TEST_ASSERT(!_arr[3].isScoped());
  PEGASUS_TEST_ASSERT(!_arr[3].isSymbolicConstant());
  PEGASUS_TEST_ASSERT(!_arr[3].isArray());
  PEGASUS_TEST_ASSERT(_arr[3].isWildcard());

  try{
    CQLChainedIdentifier _CI2("CLASS.B::EO.A::PROP[..3].SCOPE3::PROP#'ok'");
    PEGASUS_TEST_ASSERT(false);
  }catch(QueryParseException&)
  {
  }
  try{
        CQLChainedIdentifier _CI3("CLASS.B::EO.A::PROP[3..].SCOPE3::PROP#'ok'");
        PEGASUS_TEST_ASSERT(false);
  }catch(QueryParseException&)
  {
  }
}

int main(int, char *argv[] ){

  //BEGIN TESTS....

  drive_CQLIdentifier();
  drive_CQLChainedIdentifier();

  //END TESTS....

  cout << argv[0] << " +++++ passed all tests" << endl;

  return 0;
}

