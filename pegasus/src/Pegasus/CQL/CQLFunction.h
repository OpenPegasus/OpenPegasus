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

#ifndef Pegasus_CQLFunction_h
#define Pegasus_CQLFunction_h

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLValue.h>
#include <Pegasus/CQL/CQLPredicate.h>

PEGASUS_NAMESPACE_BEGIN

class CQLFactory;
class CQLFunctionRep;

 /** The Enum is private, the definition is public.
      */
enum  FunctionOpType
{
  UNKNOWN,
  DATETIMETOMICROSECOND,
  STRINGTOUINT,
  STRINGTOSINT,
  STRINGTOREAL,
  STRINGTONUMERIC,
  UPPERCASE,
  NUMERICTOSTRING,
  REFERENCETOSTRING,
  CLASSNAME,
  NAMESPACENAME,
  NAMESPACETYPE,
  HOSTPORT,
  MODELPATH,
  CLASSPATH,
  OBJECTPATH,
  INSTANCETOREFERENCE,
  CURRENTDATETIME,
  DATETIME,
  MICROSECONDTOTIMESTAMP,
  MICROSECONDTOINTERVAL
};
/**
   CQLFunction objects are populated by the
   Bison code.

   Supported functions are in accordance with the
   DMTF CQL Specification.
   TODO:  THIS LIST IS SUBJECT TO CHANGE

    classname( <expr> )
    classname( )
    count(*)
    count( distinct * )
    count( distinct <expr> )
    something for createarray
    something for datetime
    something for hostname
    max( <expr> )
    mean( <expr> )
    median( <expr> )
    min( <expr> )
    something for modelpath
    something for namespacename
    something for namespacepath
    something for objectpath
    something for scheme
    sum( <expr> )
    something for userinfo
    uppercase( <expr> )

  */


class CQLFunction
{
public:

    CQLFunction();

    CQLFunction(const CQLFunction& inFunc);

//    CQLFunction(FunctionOpType inFunctionOpType,
//                  Array<CQLExpression> inParms);

    CQLFunction(CQLIdentifier inOpType, Array<CQLPredicate> inParms);

    ~CQLFunction();
    /**
       The getValue method validates the parms versus FunctionOpType.
               (A) resolves prarameter  types
               (B) number of parms
        and then actually executes the function.
        Returns a CQLValue object that has already been resolved.
      */
    CQLValue resolveValue(const CIMInstance& CI, const QueryContext& queryCtx);

    Array<CQLPredicate> getParms()const;

    FunctionOpType getFunctionType()const;

    String toString()const;

    void applyContext(const QueryContext& inContext);

    CQLFunction& operator=(const CQLFunction& rhs);

    // Boolean operator==(const CQLFunction& func)const;

    // Boolean operator!=(const CQLFunction& func)const;

    friend class CQLFactory;

private:

    CQLFunctionRep *_rep;

};

PEGASUS_NAMESPACE_END
#endif
#endif
