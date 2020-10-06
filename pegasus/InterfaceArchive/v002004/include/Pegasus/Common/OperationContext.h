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
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/AcceptLanguages.h>  // l10n
#include <Pegasus/Common/ContentLanguages.h> // l10n
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class OperationContextRep;

/**
Contains context information from clients in objects referred to as containers.

<p>The OperationContext class carries information about
the context in which the client program issued the request. The OperationContext 
class contains containers that specify types of information. 
Currently, the only supported container is the container that specifies
the identity of the user. Providers must use the user container
to determine whether the requested operation should be 
permitted on behalf of
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
        container is used. Currently, only the container
        carrying the user identity of the client is available.</p>
    */
    class PEGASUS_COMMON_LINKAGE Container
    {
    public:

        /** Destructs Container.
        */
        virtual ~Container(void);

        /** REVIEWERS: Insert description here. What parameters 
        need to be described, if any?
        */
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
    /** REVIEWERS:Insert description here. What parameters need descriptions?
    */
    OperationContext(void);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param context Specifies the name of the OperationContext container.
    */
    OperationContext(const OperationContext & context);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    */
    virtual ~OperationContext(void);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param context Specifies the name of the OperationContext container.
    */
    OperationContext & operator=(const OperationContext & context);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    */
    void clear(void);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param containerName Specifies the name of the String container.
    */
    
    const Container & get(const String& containerName) const;

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param container Specifies the name of the container.
    */
    void set(const Container & container);

    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param container Specifies the name of the container.
    */
    void insert(const Container & container);


    /** REVIEWERS:Insert description here. What parameters need descriptions?
    @param containerName Specifies the name of the container.
    */
    void remove(const String& containerName);

protected:
    OperationContextRep* _rep;

};


class IdentityContainerRep;

/// Insert description here. 
class PEGASUS_COMMON_LINKAGE IdentityContainer
    : virtual public OperationContext::Container
{
public:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
        @param container Specifies the name of the Container.
    */
    IdentityContainer(const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
        @param container Specifies the name of the IdentityContainer.
    */
    IdentityContainer(const IdentityContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
        @param userName Specifies the name of the String.
    */
    IdentityContainer(const String & userName);

    /** REVIEWERS: Insert description here.
    */
    virtual ~IdentityContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
        @param container Specifies the name of the IdentityContainer.
    */
    IdentityContainer & operator=(const IdentityContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    String getUserName(void) const;

protected:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    IdentityContainerRep* _rep;

private:
    IdentityContainer();    // Unimplemented
};


class SubscriptionInstanceContainerRep;

class PEGASUS_COMMON_LINKAGE SubscriptionInstanceContainer
    : virtual public OperationContext::Container
{
public:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceContainer
        (const SubscriptionInstanceContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceContainer(const CIMInstance & subscriptionInstance);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual ~SubscriptionInstanceContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param operator Specifies the name of the SubscriptionInstanceContainer
    */
    SubscriptionInstanceContainer & operator=
        (const SubscriptionInstanceContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. 
    */
    CIMInstance getInstance(void) const;

protected:
    SubscriptionInstanceContainerRep* _rep;

private:
    SubscriptionInstanceContainer();    // Unimplemented
};

class SubscriptionFilterConditionContainerRep;

class PEGASUS_COMMON_LINKAGE SubscriptionFilterConditionContainer
    : virtual public OperationContext::Container
{
public:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionFilterConditionContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param conainer REVIEWERS: Insert description here.
    */
    SubscriptionFilterConditionContainer
        (const SubscriptionFilterConditionContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param filterCondition REVIEWERS: Insert description here.
    @param queryLanguage REVIEWERS: Insert description here.
    */
    SubscriptionFilterConditionContainer
        (const String & filterCondition, 
        const String & queryLanguage);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual ~SubscriptionFilterConditionContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container REVIEWERS: Insert description here.
    */
    SubscriptionFilterConditionContainer & operator=
        (const SubscriptionFilterConditionContainer & container);

    /** REVIEWERS: Insert description here. 
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. 
    */
    String getFilterCondition(void) const;

    /** REVIEWERS: Insert description here. 
    */
    String getQueryLanguage(void) const;

protected:
    SubscriptionFilterConditionContainerRep* _rep;

private:
    SubscriptionFilterConditionContainer();    // Unimplemented
};

    /**REVIEWERS: Insert class description here.
    */
class SubscriptionInstanceNamesContainerRep;

    /**REVIEWERS: Insert class description here.
    */
class PEGASUS_COMMON_LINKAGE SubscriptionInstanceNamesContainer
    : virtual public OperationContext::Container
{
public:

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceNamesContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceNamesContainer
        (const SubscriptionInstanceNamesContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionInstanceNamesContainer
        (const Array<CIMObjectPath> & subscriptionInstanceNames);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual ~SubscriptionInstanceNamesContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container REVIEWERS: Insert description here.
    */
    SubscriptionInstanceNamesContainer & operator=
        (const SubscriptionInstanceNamesContainer & container);

    /** REVIEWERS: Insert description here. 
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    Array<CIMObjectPath> getInstanceNames(void) const;

protected:
    SubscriptionInstanceNamesContainerRep* _rep;

private:
    SubscriptionInstanceNamesContainer();    // Unimplemented
};

    ///Insert class description here. 
class PEGASUS_COMMON_LINKAGE TimeoutContainer : virtual public OperationContext::Container
{
   public:

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      static const String NAME;
      
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      TimeoutContainer(const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      TimeoutContainer(Uint32 timeout);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      virtual void destroy(void);
      
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
      Uint32 getTimeOut(void) const;
   protected:
      Uint32 _value;
   private:
      TimeoutContainer(void);
};


// l10n
/////////////////////////////////////////////////////////////////////////////
// Start - Containers used for globalization
/////////////////////////////////////////////////////////////////////////////

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

class AcceptLanguageListContainerRep;

/** <I><B>Experimental Interface</B></I><BR>
 */
class PEGASUS_COMMON_LINKAGE AcceptLanguageListContainer
    : virtual public OperationContext::Container
{
public:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguageListContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguageListContainer
        (const AcceptLanguageListContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguageListContainer(const AcceptLanguages & languages);
    
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual ~AcceptLanguageListContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguageListContainer & operator=
        (const AcceptLanguageListContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguages getLanguages(void) const;

protected:
   AcceptLanguageListContainerRep* _rep;

private:
    AcceptLanguageListContainer();    // Unimplemented
};

    /** REVIEWERS: Insert class description here. 
    */
class SubscriptionLanguageListContainerRep;

    /** <I><B>Experimental Interface</B></I><BR>
    * REVIEWERS: Insert class description here. 
    */
class PEGASUS_COMMON_LINKAGE SubscriptionLanguageListContainer
    : virtual public OperationContext::Container
{
public:

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;
    
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionLanguageListContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionLanguageListContainer
        (const SubscriptionLanguageListContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionLanguageListContainer(const AcceptLanguages & languages);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual ~SubscriptionLanguageListContainer(void); 
     
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the SubscriptionLanguageListContainer.
    */    
    SubscriptionLanguageListContainer & operator=
        (const SubscriptionLanguageListContainer & container);

    /** REVIEWERS: Insert description here. 
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    AcceptLanguages getLanguages(void) const;

protected:
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    SubscriptionLanguageListContainerRep* _rep;

private:
    SubscriptionLanguageListContainer();    // Unimplemented
};    

    /** REVIEWERS: Insert class description here. 
    */
class ContentLanguageListContainerRep;
    /** <I><B>Experimental Interface</B></I><BR>
    * REVIEWERS: Insert class description here. 
    */
class PEGASUS_COMMON_LINKAGE ContentLanguageListContainer
    : virtual public OperationContext::Container
{
public:

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the OperationContext container.
    */
    ContentLanguageListContainer
        (const OperationContext::Container & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the ContentLanguageListContainer.
    */
    ContentLanguageListContainer
        (const ContentLanguageListContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param languages Specifies the name of the ContentLanguages container.
    */
    ContentLanguageListContainer(const ContentLanguages & languages);

    /** REVIEWERS: Insert description here. 
    */
    virtual ~ContentLanguageListContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the ContentLanguageListContainer.
    */
    ContentLanguageListContainer & operator=
        (const ContentLanguageListContainer & container);

    /** REVIEWERS: Insert description here. 
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. 
    */
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. 
    */
    ContentLanguages getLanguages(void) const;

protected:
    /** REVIEWERS: Insert description here. 
    */
   ContentLanguageListContainerRep* _rep;

private:
    ContentLanguageListContainer();    // Unimplemented
};

#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES

/////////////////////////////////////////////////////////////////////////////
// End - Containers used for globalization
/////////////////////////////////////////////////////////////////////////////

class SnmpTrapOidContainerRep;

class PEGASUS_COMMON_LINKAGE SnmpTrapOidContainer
    : virtual public OperationContext::Container
{
public:

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    static const String NAME;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the container.  
    */
    SnmpTrapOidContainer
        (const OperationContext::Container & container);
    
    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the SnmpTrapOidContainer container.   
    */
    SnmpTrapOidContainer
        (const SnmpTrapOidContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param snmpTrapOid Specifies the name of the String container.
    */
    SnmpTrapOidContainer(const String & snmpTrapOid);

    /** REVIEWERS: Insert description here. 
    */
    virtual ~SnmpTrapOidContainer(void);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    @param container Specifies the name of the SnmpTrapOidContainer.
    */
    SnmpTrapOidContainer & operator=
        (const SnmpTrapOidContainer & container);

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual String getName(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */
    virtual OperationContext::Container * clone(void) const;

    /** REVIEWERS: Insert description here. What parameters need descriptions?
    */    
    virtual void destroy(void);

    /** REVIEWERS: Insert description here. 
    */
    String getSnmpTrapOid(void) const;

protected:
    SnmpTrapOidContainerRep* _rep;

private:
    SnmpTrapOidContainer();    // Unimplemented
};


PEGASUS_NAMESPACE_END

#endif
