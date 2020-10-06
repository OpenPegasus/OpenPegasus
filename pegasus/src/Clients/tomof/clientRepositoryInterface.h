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
//
// This class is the interface to allow a common call to serve both the
// repository and the client interface.  Since these two interfaces were not
// designed with a common hiearchy, it substitutes the individual calls
// with any special characteristics for each call. Note that Bob Blair in
// the Compiler created much the same structure but with different operations.
//
// This class supports only the operations that tomof needs, which
// are
//     enumerateQualifiers()
//     enumerateClassNames()
//     enumerateClasses()
//     enumerateInstances()
//
//  ACTION: In the future we should regularalize this so we have one class
//  that can serve multiple programs. However, since the compiler extended
//  the repository definitions and uses different functions, it was easier
//  just to create a new class
//

#ifndef CLIENT_REPOSITORY_INTERFACE_H_
#define CLIENT_REPOSITORY_INTERFACE_H_

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMObjectPath.h>


PEGASUS_NAMESPACE_BEGIN

// Forward declarations
class CIMRepository;
class CIMClient;
class CIMClass;
class CIMQualifierDecl;
class CIMInstance;


class clientRepositoryInterface
{
 private:
  CIMRepository *_repository;
  CIMClient     *_client;

 public:
  enum _repositoryType { REPOSITORY_INTERFACE_LOCAL = 0,
                          REPOSITORY_INTERFACE_CLIENT
  };
  clientRepositoryInterface();

  virtual ~clientRepositoryInterface();

  void init(_repositoryType type, const String &location);

  Boolean ok() const { return _repository || _client; }

  virtual Array<CIMQualifierDecl> enumerateQualifiers(
      const CIMNamespaceName &nameSpace) const;

  virtual CIMClass getClass(
      const CIMNamespaceName& nameSpace,
      const CIMName& className,
      const Boolean localOnly,
      const Boolean includeQualifiers,
      const Boolean includeClassOrigin) const;

  virtual Array<CIMClass> enumerateClasses(
      const CIMNamespaceName& nameSpace,
      const CIMName& className,
      const Boolean deepInheritance,
      const Boolean localOnly,
      const Boolean includeQualifiers,
      const Boolean includeClassOrigin) const;

  virtual Array<CIMName> enumerateClassNames(
      const CIMNamespaceName& nameSpace,
      const CIMName& className,
      const Boolean deepInheritance);

  virtual Array<CIMObjectPath> enumerateInstanceNames(
      const CIMNamespaceName& nameSpace,
      const CIMName& className);

  virtual Array<CIMInstance> enumerateInstances(
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance = true,
    Boolean includeQualifiers = false,
    Boolean includeClassOrigin = false,
    const CIMPropertyList& propertyList = CIMPropertyList());

};

PEGASUS_NAMESPACE_END

#endif /* CLIENT_REPOSITORY_INTERFACE_H_ */


