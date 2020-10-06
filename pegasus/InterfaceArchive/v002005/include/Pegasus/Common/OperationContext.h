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
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class OperationContextRep;

/**
    An OperationContext object holds information about the context of an
    operation, using various Container classes.  The Container subclasses
    define the set of information that may be available in the
    OperationContext.
*/
class PEGASUS_COMMON_LINKAGE OperationContext
{
public:

    /**
        A Container subclass object holds a piece of context information
        for an operation.
    */
    class PEGASUS_COMMON_LINKAGE Container
    {
    public:

        /**
            Destructs the Container.
        */
        virtual ~Container();

        /**
            Returns the unique name for a Container type.
            @return The String name of the Container type.
        */
        virtual String getName() const = 0;

        /**
            Makes a copy of the Container object.  The caller is responsible 
            for cleaning up the copy by calling destroy() method.
            @return A pointer to the new Container object.
        */
        virtual Container* clone() const = 0;

        /**
            Cleans up a Container object that was created by the clone()
            method.
        */
        virtual void destroy() = 0;
    };

    /**
        Constructs an empty OperationContext object.
    */
    OperationContext();

    /**
        Constructs a copy of an OperationContext object.  The newly
        constructed OperationContext object is independent from the source
        object.
        @param context The OperationContext object to copy.
    */
    OperationContext(const OperationContext& context);

    /**
        Destructs the OperationContext.
    */
    virtual ~OperationContext();

    /**
        Assigns the value of the specified OperationContext object to this
        OperationContext.  As a result, this OperationContext object will
        contain the same set of Containers as in the specified object.
        @param context The OperationContext object to copy.
    */
    OperationContext& operator=(const OperationContext& context);

    /**
        Removes all the Containers from the OperationContext.
    */
    void clear();

    /**
        Retrieves the specified Container object from the OperationContext.
        @param containerName The name of the Container type to retrieve.
        @return A reference to the specified Container object.
        @exception Exception if the OperationContext does not contain the
        specified Container type.
    */
    const Container& get(const String& containerName) const;

    /**
        Replaces an OperationContext Container with the specified Container
        object of the same type.
        @param container The Container to set in the OperationContext.
        @exception Exception if the OperationContext does not contain the
        specified Container type.
    */
    void set(const Container& container);

    /**
        Inserts a Container into the OperationContext.
        @param container The Container to insert into the OperationContext.
        @exception Exception if the OperationContext already contains a
        Container of this type.
    */
    void insert(const Container& container);

    /**
        Removes a Container from the OperationContext.
        @param containerName The name of the Container type to remove from
        the OperationContext.
        @exception Exception if the OperationContext does not contain the
        specified Container type.
    */
    void remove(const String& containerName);

protected:
    OperationContextRep* _rep;
};


class IdentityContainerRep;

/**
    An IdentityContainer object holds the identity of a user associated with
    an operation.  For example, a provider must use this Container to
    determine whether to perform an operation on the behalf of the requesting
    user.
*/
class PEGASUS_COMMON_LINKAGE IdentityContainer
    : virtual public OperationContext::Container
{
public:
    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs an IdentityContainer object from the specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not an IdentityContainer object.
    */
    IdentityContainer(const OperationContext::Container& container);

    /**
        Constructs a copy of the specified IdentityContainer.
        @param container The IdentityContainer object to copy.
    */
    IdentityContainer(const IdentityContainer& container);

    /**
        Constructs an IdentityContainer with a specified user name.
        @param userName A String user name for this identity.
    */
    IdentityContainer(const String& userName);

    /**
        Destructs the IdentityContainer.
    */
    virtual ~IdentityContainer();

    /**
        Assigns the value of the specified IdentityContainer object to this
        object.
        @param container The IdentityContainer object to copy.
    */
    IdentityContainer& operator=(const IdentityContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this IdentityContainer object.  The caller is
        responsible for cleaning up the copy by calling destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up an IdentityContainer object that was created by the
        clone() method.
    */
    virtual void destroy();

    /**
        Gets the user name from the IdentityContainer object.
        @return A String containing the user name identity.
    */
    String getUserName() const;

protected:
    IdentityContainerRep* _rep;

private:
    IdentityContainer();    // Unimplemented
};


class SubscriptionInstanceContainerRep;

/**
    A SubscriptionInstanceContainer object holds a CIMInstance associated
    with an indication subscription.
*/
class PEGASUS_COMMON_LINKAGE SubscriptionInstanceContainer
    : virtual public OperationContext::Container
{
public:
    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs a SubscriptionInstanceContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a SubscriptionInstanceContainer object.
    */
    SubscriptionInstanceContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified SubscriptionInstanceContainer.
        @param container The SubscriptionInstanceContainer object to copy.
    */
    SubscriptionInstanceContainer(
        const SubscriptionInstanceContainer& container);

    /**
        Constructs a SubscriptionInstanceContainer with the specified
        subscription instance.
        @param subscriptionInstance The subscription instance to be held by
        this Container.
    */
    SubscriptionInstanceContainer(const CIMInstance& subscriptionInstance);

    /**
        Destructs the SubscriptionInstanceContainer.
    */
    virtual ~SubscriptionInstanceContainer();

    /**
        Assigns the value of the specified SubscriptionInstanceContainer
        object to this object.
        @param container The SubscriptionInstanceContainer object to copy.
    */
    SubscriptionInstanceContainer& operator=(
        const SubscriptionInstanceContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this SubscriptionInstanceContainer object.  The
        caller is responsible for cleaning up the copy by calling destroy()
        method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a SubscriptionInstanceContainer object that was created
        by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the subscription instance from the SubscriptionInstanceContainer.
        @return A CIMInstance representing a subscription.
    */
    CIMInstance getInstance() const;

protected:
    SubscriptionInstanceContainerRep* _rep;

private:
    SubscriptionInstanceContainer();    // Unimplemented
};


class SubscriptionFilterConditionContainerRep;

/**
    A SubscriptionFilterConditionContainer object holds the filter condition
    and query language associated with an indication subscription.  The
    filter condition is equivalent to only the "WHERE" clause of a filter
    query.
*/
class PEGASUS_COMMON_LINKAGE SubscriptionFilterConditionContainer
    : virtual public OperationContext::Container
{
public:
    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs a SubscriptionFilterConditionContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a SubscriptionFilterConditionContainer object.
    */
    SubscriptionFilterConditionContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified
        SubscriptionFilterConditionContainer.
        @param container The SubscriptionFilterConditionContainer object
        to copy.
    */
    SubscriptionFilterConditionContainer(
        const SubscriptionFilterConditionContainer& container);

    /**
        Constructs a SubscriptionFilterConditionContainer with the specified
        filter condition and query language.
        @param filterCondition The query condition String associated with an
        indication subscription filter.
        @param queryLanguage The query language String associated with an
        indication subscription filter.
    */
    SubscriptionFilterConditionContainer(
        const String& filterCondition, 
        const String& queryLanguage);

    /**
        Destructs the SubscriptionFilterConditionContainer.
    */
    virtual ~SubscriptionFilterConditionContainer();

    /**
        Assigns the value of the specified SubscriptionFilterConditionContainer
        object to this object.
        @param container The SubscriptionFilterConditionContainer object to
        copy.
    */
    SubscriptionFilterConditionContainer& operator=(
        const SubscriptionFilterConditionContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this SubscriptionFilterConditionContainer object.
        The caller is responsible for cleaning up the copy by calling
        destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a SubscriptionFilterConditionContainer object that was
        created by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the filter query condition from the
        SubscriptionFilterConditionContainer.  Note that the filter query
        condition is equivalent to only the "WHERE" clause of a filter query.
        @return The query condition String associated with an indication
        subscription filter.
    */
    String getFilterCondition() const;

    /**
        Gets the query language from the SubscriptionFilterConditionContainer.
        @return The query language String associated with an indication
        subscription filter.
    */
    String getQueryLanguage() const;

protected:
    SubscriptionFilterConditionContainerRep* _rep;

private:
    SubscriptionFilterConditionContainer();    // Unimplemented
};


class SubscriptionFilterQueryContainerRep;

/**
    A SubscriptionFilterQueryContainer object holds the query filter
    and query language associated with an indication subscription, as well
    as the source namespace of the filter.  The query filter contains the
    whole query string ("SELECT" statement) from the subscription filter
    instance.  (This differs from the filter condition string in
    SubscriptionFilterConditionContainer, which only contains the "WHERE"
    clause of the filter.)
*/
class PEGASUS_COMMON_LINKAGE SubscriptionFilterQueryContainer
    : virtual public OperationContext::Container
{
public:
    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs a SubscriptionFilterQueryContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a SubscriptionFilterQueryContainer object.
    */
    SubscriptionFilterQueryContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified SubscriptionFilterQueryContainer.
        @param container The SubscriptionFilterQueryContainer object to copy.
    */
    SubscriptionFilterQueryContainer(
        const SubscriptionFilterQueryContainer& container);

    /**
        Constructs a SubscriptionFilterQueryContainer with the specified
        filter query, query language, and source namespace.
        @param filterQuery The filter query String associated with an
        indication subscription filter.
        @param queryLanguage The query language String associated with an
        indication subscription filter.
        @param sourceNameSpace The CIMNamespaceName of the source namespace
        associated with an indication subscription filter.
    */
    SubscriptionFilterQueryContainer(
       const String& filterQuery,
       const String& queryLanguage,
       const CIMNamespaceName& sourceNameSpace);

    /**
        Destructs the SubscriptionFilterQueryContainer.
    */
    virtual ~SubscriptionFilterQueryContainer();

    /**
        Assigns the value of the specified SubscriptionFilterQueryContainer
        object to this object.
        @param container The SubscriptionFilterQueryContainer object to copy.
    */
    SubscriptionFilterQueryContainer& operator=(
        const SubscriptionFilterQueryContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this SubscriptionFilterQueryContainer object.
        The caller is responsible for cleaning up the copy by calling
        destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a SubscriptionFilterQueryContainer object that was
        created by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the filter query from the SubscriptionFilterQueryContainer.
        @return The query String associated with an indication subscription
        filter.
    */
    String getFilterQuery() const;

    /**
        Gets the filter query language from the
        SubscriptionFilterQueryContainer.
        @return The query language String associated with an indication
        subscription filter.
    */
    String getQueryLanguage() const;

    /**
        Gets the source namespace from the SubscriptionFilterQueryContainer.
        @return The source namespace associated with an indication
        subscription filter.
    */
    CIMNamespaceName getSourceNameSpace() const;

protected:
    SubscriptionFilterQueryContainerRep* _rep;

private:
    SubscriptionFilterQueryContainer();    // Unimplemented
};


class SubscriptionInstanceNamesContainerRep;

/**
    A SubscriptionInstanceNamesContainer object holds a list of subscription
    instance names.  This can be used to limit the set of subscriptions that
    are considered targets for an indication when it is generated, which is
    necessary for a provider to generate localized indications or to
    implement a subscription's repeat notification policy.
*/
class PEGASUS_COMMON_LINKAGE SubscriptionInstanceNamesContainer
    : virtual public OperationContext::Container
{
public:

    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs a SubscriptionInstanceNamesContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a SubscriptionInstanceNamesContainer object.
    */
    SubscriptionInstanceNamesContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified SubscriptionInstanceNamesContainer.
        @param container The SubscriptionInstanceNamesContainer object to copy.
    */
    SubscriptionInstanceNamesContainer(
        const SubscriptionInstanceNamesContainer& container);

    /**
        Constructs a SubscriptionInstanceNamesContainer with the specified
        list of subscription instance names.
        @param subscriptionInstanceNames A CIMObjectPath Array with the names
        of indication subscription instances.
    */
    SubscriptionInstanceNamesContainer(
        const Array<CIMObjectPath>& subscriptionInstanceNames);

    /**
        Destructs the SubscriptionInstanceNamesContainer.
    */
    virtual ~SubscriptionInstanceNamesContainer();

    /**
        Assigns the value of the specified SubscriptionInstanceNamesContainer
        object to this object.
        @param container The SubscriptionInstanceNamesContainer object to copy.
    */
    SubscriptionInstanceNamesContainer& operator=(
        const SubscriptionInstanceNamesContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this SubscriptionInstanceNamesContainer object.
        The caller is responsible for cleaning up the copy by calling
        destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a SubscriptionInstanceNamesContainer object that was
        created by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the list of subscription instance names from the
        SubscriptionInstanceNamesContainer.
        @return A CIMObjectPath Array of indication subscription instance
        names.
    */
    Array<CIMObjectPath> getInstanceNames() const;

protected:
    SubscriptionInstanceNamesContainerRep* _rep;

private:
    SubscriptionInstanceNamesContainer();    // Unimplemented
};


/**
    A TimeoutContainer object holds an operation timeout value, in
    milliseconds.
*/
class PEGASUS_COMMON_LINKAGE TimeoutContainer
    : virtual public OperationContext::Container
{
public:

    /**
        The unique name for this container type.
    */
    static const String NAME;
      
    /**
        Constructs a TimeoutContainer object from the specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a TimeoutContainer object.
    */
    TimeoutContainer(const OperationContext::Container& container);

    /**
        Constructs a TimeoutContainer with the specified timeout value.
        @param timeout An integer timeout value (in milliseconds).
    */
    TimeoutContainer(Uint32 timeout);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this TimeoutContainer object.  The caller is
        responsible for cleaning up the copy by calling destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a TimeoutContainer object that was created by the clone()
        method.
    */
    virtual void destroy();
      
    /**
        Gets the timeout value from the TimeoutContainer.
        @return An integer timeout value (in milliseconds).
    */
    Uint32 getTimeOut() const;

protected:
    Uint32 _value;

private:
    TimeoutContainer();
};


#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

class AcceptLanguageListContainerRep;

/** <I><B>Experimental Interface</B></I><BR>
    An AcceptLanguageListContainer object holds a list of languages that
    are acceptable in the response for a given operation.
*/
class PEGASUS_COMMON_LINKAGE AcceptLanguageListContainer
    : virtual public OperationContext::Container
{
public:
    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs an AcceptLanguageListContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not an AcceptLanguageListContainer object.
    */
    AcceptLanguageListContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified AcceptLanguageListContainer.
        @param container The AcceptLanguageListContainer object to copy.
    */
    AcceptLanguageListContainer(
        const AcceptLanguageListContainer& container);

    /**
        Constructs an AcceptLanguageListContainer with the specified
        accept language list.
        @param languages An AcceptLanguageList with the response languages
        that are acceptable in this context.
    */
    AcceptLanguageListContainer(const AcceptLanguageList& languages);
    
    /**
        Destructs the AcceptLanguageListContainer.
    */
    virtual ~AcceptLanguageListContainer();

    /**
        Assigns the value of the specified AcceptLanguageListContainer
        object to this object.
        @param container The AcceptLanguageListContainer object to copy.
    */
    AcceptLanguageListContainer& operator=(
        const AcceptLanguageListContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this AcceptLanguageListContainer object.  The caller
        is responsible for cleaning up the copy by calling destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up an AcceptLanguageListContainer object that was created by
        the clone() method.
    */
    virtual void destroy();

    /**
        Gets the list of acceptable response languages from the
        AcceptLanguageListContainer.
        @return An AcceptLanguageList with the response languages that are
        acceptable in this context.
    */
    AcceptLanguageList getLanguages() const;

protected:
    AcceptLanguageListContainerRep* _rep;

private:
    AcceptLanguageListContainer();    // Unimplemented
};


class ContentLanguageListContainerRep;

/** <I><B>Experimental Interface</B></I><BR>
    A ContentLanguageListContainer object holds a list of languages that
    are contained in the associated data.
*/
class PEGASUS_COMMON_LINKAGE ContentLanguageListContainer
    : virtual public OperationContext::Container
{
public:

    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs a ContentLanguageListContainer object from the
        specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not a ContentLanguageListContainer object.
    */
    ContentLanguageListContainer(
        const OperationContext::Container& container);

    /**
        Constructs a copy of the specified ContentLanguageListContainer.
        @param container The ContentLanguageListContainer object to copy.
    */
    ContentLanguageListContainer(
        const ContentLanguageListContainer& container);

    /**
        Constructs a ContentLanguageListContainer with the specified
        content language list.
        @param languages A ContentLanguageList with the languages that are
        contained in the associated data.
    */
    ContentLanguageListContainer(const ContentLanguageList& languages);

    /**
        Destructs the ContentLanguageListContainer.
    */
    virtual ~ContentLanguageListContainer();

    /**
        Assigns the value of the specified ContentLanguageListContainer
        object to this object.
        @param container The ContentLanguageListContainer object to copy.
    */
    ContentLanguageListContainer& operator=(
        const ContentLanguageListContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this ContentLanguageListContainer object.  The caller
        is responsible for cleaning up the copy by calling destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up a ContentLanguageListContainer object that was created
        by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the list of content languages from the
        ContentLanguageListContainer.
        @return A ContentLanguageList with the languages that are contained
        in the associated data.
    */
    ContentLanguageList getLanguages() const;

protected:
    ContentLanguageListContainerRep* _rep;

private:
    ContentLanguageListContainer();    // Unimplemented
};

#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES


class SnmpTrapOidContainerRep;

/**
    An SnmpTrapOidContainer object holds an SNMP trap OID that corresponds
    to the associated data.
*/
class PEGASUS_COMMON_LINKAGE SnmpTrapOidContainer
    : virtual public OperationContext::Container
{
public:

    /**
        The unique name for this container type.
    */
    static const String NAME;

    /**
        Constructs an SnmpTrapOidContainer object from the specified Container.
        @param container The Container object to copy.
        @exception DynamicCastFailedException If the specified Container
        object is not an SnmpTrapOidContainer object.
    */
    SnmpTrapOidContainer(const OperationContext::Container& container);
    
    /**
        Constructs a copy of the specified SnmpTrapOidContainer.
        @param container The SnmpTrapOidContainer object to copy.
    */
    SnmpTrapOidContainer(const SnmpTrapOidContainer& container);

    /**
        Constructs an SnmpTrapOidContainer with the specified SNMP trap OID.
        @param snmpTrapOid A String containing an SNMP trap OID.
    */
    SnmpTrapOidContainer(const String& snmpTrapOid);

    /**
        Destructs the SnmpTrapOidContainer.
    */
    virtual ~SnmpTrapOidContainer();

    /**
        Assigns the value of the specified SnmpTrapOidContainer
        object to this object.
        @param container The SnmpTrapOidContainer object to copy.
    */
    SnmpTrapOidContainer& operator=(const SnmpTrapOidContainer& container);

    /**
        Returns the unique name for this Container type.
        @return The String name of the Container type.
    */
    virtual String getName() const;

    /**
        Makes a copy of this SnmpTrapOidContainer object.  The caller is
        responsible for cleaning up the copy by calling destroy() method.
        @return A pointer to the new Container object.
    */
    virtual OperationContext::Container* clone() const;

    /**
        Cleans up an SnmpTrapOidContainer object that was created
        by the clone() method.
    */
    virtual void destroy();

    /**
        Gets the SNMP trap OID from the SnmpTrapOidContainer.
        @return A String with the SNMP trap OID corresponding to the
        associated data.
    */
    String getSnmpTrapOid() const;

protected:
    SnmpTrapOidContainerRep* _rep;

private:
    SnmpTrapOidContainer();    // Unimplemented
};


PEGASUS_NAMESPACE_END

#endif
