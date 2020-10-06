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

#ifndef Pegasus_CQLFactor_h
#define Pegasus_CQLFactor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/CQL/CQLPredicate.h>
#include <Pegasus/CQL/CQLFunction.h>


PEGASUS_NAMESPACE_BEGIN

// Forward declares
class CIMNamespaceName;
class CQLFactory;

class CQLFunctionRep
{
public:

    CQLFunctionRep();

    CQLFunctionRep(const CQLFunctionRep* rep);

    CQLFunctionRep(CQLIdentifier inOpType, Array<CQLPredicate> inParms);

    ~CQLFunctionRep();

    /**
     The getValue method validates the parms versus FunctionOpType.
     (A) resolves prarameter  types
     (B) number of parms
     and then actually executes the function.
     Returns a CQLValue object that has already been resolved.
     */
    CQLValue resolveValue(const CIMInstance& CI, const QueryContext& queryCtx);

    Array<CQLPredicate> getParms() const;

    FunctionOpType getFunctionType() const;

    String toString()const;

    void applyContext(const QueryContext& inContext);

    // Boolean operator==(const CQLFunctionRep& func)const;

    // Boolean operator!=(const CQLFunctionRep& func)const;

    friend class CQLFactory;

 private:
    CQLValue dateTimeToMicrosecond(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue stringToUint(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue stringToSint(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue stringToReal(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue stringToNumeric(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue upperCase(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue numericToString(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue referenceToString(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue className(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue nameSpaceName(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue nameSpaceType(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue hostPort(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue modelPath(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue classPath(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue objectPath(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue instanceToReference(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue currentDateTime() const;
    CQLValue dateTime(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue microsecondToTimestamp(const CIMInstance& CI,
           const QueryContext& queryCtx) const;
    CQLValue microsecondToInterval(const CIMInstance& CI,
           const QueryContext& queryCtx) const;

    // Used by the path functions
    CQLValue buildModelPath(const CIMObjectPath& objPath) const;
    CQLValue buildClassPath(const CIMObjectPath& objPath,
           const CIMNamespaceName& ns) const;
    CQLValue buildObjectPath(const CIMObjectPath& objPath,
           const CIMNamespaceName& ns) const;

    // Utility methods to convert enums into strings
    String functionTypeToString() const;

    // Member data
    FunctionOpType _funcOpType;
    Array<CQLPredicate> _parms;

};

PEGASUS_NAMESPACE_END

#endif
