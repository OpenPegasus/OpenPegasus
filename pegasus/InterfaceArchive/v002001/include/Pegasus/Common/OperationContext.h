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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_OperationContext_h
#define Pegasus_OperationContext_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class OperationContextRep;

/**
Context information from client.

<p>The <tt>OperationContext</tt> class carries information about
the context in which the client program issued the request.
Currently, the identity of the user is the only supported
information. Providers must use this to determine whether
the requested operation should be permitted on behalf of
the issuing user.

For example, providers can get the user name information from the
IdentityContext in an OperationContext as shown below:

    <PRE>
    IdentityContainer container(context.get(IdentityContainer::NAME));
    String userName = container.getUserName();
    </PRE>
</p>
*/
class PEGASUS_COMMON_LINKAGE OperationContext
{
public:
    /**
        An element of client context information.
    
        <p>The <tt>Container</tt> object carries the information that
        the provider may access. A container name determines which
        container is being referenced. Currently, only the container
        carrying the user identity of the client is available.</p>
    */
    class PEGASUS_COMMON_LINKAGE Container
    {
    public:

        ///
        virtual ~Container(void);

        ///
        virtual String getName(void) const = 0;


        /** Makes a copy of the Container object. Caller is responsible 
            for deleting dynamically allocated memory by calling 
            destroy() method.
        */
        virtual Container * clone(void) const = 0;

        /** Cleans up the object, including dynamically allocated memory.
            This should only be used to clean up memory allocated using 
            the clone() method.
        */
        virtual void destroy(void) = 0;

    };

public:
    ///
    OperationContext(void);

    ///
    OperationContext(const OperationContext & context);

    ///
    virtual ~OperationContext(void);

    ///
    OperationContext & operator=(const OperationContext & context);

    /// Removes all containers in the current object.
    void clear(void);

    ///
    const Container & get(const String& containerName) const;

    ///
    void set(const Container & container);

    ///
    void insert(const Container & container);

    ///
    void remove(const String& containerName);

protected:
    OperationContextRep* _rep;

};


class IdentityContainerRep;

///
class PEGASUS_COMMON_LINKAGE IdentityContainer
    :
      virtual
              public OperationContext::Container
{
public:
    static const String NAME;

    ///
    IdentityContainer(const OperationContext::Container & container);
    ///
    IdentityContainer(const IdentityContainer & container);
    ///
    IdentityContainer(const String & userName);
    ///
    virtual ~IdentityContainer(void);
    ///
    IdentityContainer & operator=(const IdentityContainer & container);
    ///
    virtual String getName(void) const;
    ///
    virtual OperationContext::Container * clone(void) const;
    ///
    virtual void destroy(void);
    ///
    String getUserName(void) const;

protected:
    IdentityContainerRep* _rep;

};

PEGASUS_NAMESPACE_END

#endif
