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

#ifndef Pegasus_Resolver_h
#define Pegasus_Resolver_h

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMMethod.h>
#include <Pegasus/Common/CIMParameter.h>
#include <Pegasus/Common/CIMQualifier.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/DeclContext.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The Resolver class provides methods to resolve CIM objects, including
    CIMClass, CIMInstance, CIMProperty, CIMMethod, CIMParameter, and
    CIMQualifier flavor.  These methods were moved here from the
    CIM[objectType] classes.  These methods in turn call the resolve() methods
    of the CIM[objectType]Rep classes, which actually do the work.

    Note that this class contains only internal functions and should not be
    made available to external users.
 */

class PEGASUS_COMMON_LINKAGE Resolver
{
public:

    /**
        Resolves the class.  Inherits any properties, methods and qualifiers.
        Makes sure the superClass exists and is consistent with this class.
        Sets the propagated and class origin flags for each class feature.

        @param theClass            CIMClass object to be resolved
        @param declContext         Defines the context in which the class is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the class.
        @param nameSpace           Namespace in which the class is to be placed
    */
    static void resolveClass(
        CIMClass& theClass,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace);

    /**
        Resolves the instance.  Makes sure the class exists and is not
        abstract.  Validates and propagates qualifiers, if requested.
        Validates and resolves properties.  Sets the propagated and class
        origin flags.

        @param theInstance         CIMInstance object to be resolved
        @param declContext         Defines the context in which the instance is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the instance.
        @param nameSpace           Namespace in which the instance is to be
                                   placed
        @param propagateQualifiers Boolean indicating whether qualifiers are to
                                   be propagated
    */
    static void resolveInstance(
        CIMInstance& theInstance,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        Boolean propagateQualifiers);

    /**
        Resolves the instance.  Makes sure the class exists and is not
        abstract.  Validates and propagates qualifiers, if requested.
        Validates and resolves properties.  Sets the propagated and class
        origin flags.

        @param theInstance         CIMInstance object to be resolved
        @param declContext         Defines the context in which the instance is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the instance.
        @param nameSpace           Namespace in which the instance is to be
                                   placed
        @param cimClassOut         CIMClass output parameter containing the
                                   class to which the instance belongs
        @param propagateQualifiers Boolean indicating whether qualifiers are to
                                   be propagated
    */
    static void resolveInstance(
        CIMInstance& theInstance,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        CIMConstClass& cimClassOut,
        Boolean propagateQualifiers);

    /**
        Resolves the property.  Resolution is the process of integrating the
        property into the the context of a repository or other store.
        Validates the qualifiers of the property.

        @param theProperty         CIMProperty object to be resolved
        @param declContext         Defines the context in which the property is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the property.
        @param nameSpace           Namespace in which the property is to be
                                   placed
        @param isInstancePart      Indicates instance or class resolution
        @param inheritedProperty   CIMConstProperty containing the property from
                                   the class or superclass
        @param propagateQualifiers Boolean indicating whether qualifiers are to
    */
    static void resolveProperty(
        CIMProperty& theProperty,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        Boolean isInstancePart,
        const CIMConstProperty& inheritedProperty,
        Boolean propagateQualifiers);

    //  ATTN: P3 03/02/02 KS Needs more documentation.
    /**
        Resolves the property.  Resolution is the process of integrating the
        property into the the context of a repository or other store.
        Validates the qualifiers of the property.

        @param theProperty         CIMProperty object to be resolved
        @param declContext         Defines the context in which the property is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the property.
        @param nameSpace           Namespace in which the property is to be
                                   placed
        @param isInstancePart      Indicates instance or class resolution
        @param propagateQualifiers Boolean indicating whether qualifiers are to
                                   be propagated
    */
    static void resolveProperty(
        CIMProperty& theProperty,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        Boolean isInstancePart,
        Boolean propagateQualifiers);

    /**
        Resolves the CIMMethod.  Validates the qualifiers and parameters of the
        method.

        @param theMethod           CIMMethod object to be resolved
        @param declContext         Defines the context in which the method is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the method.
        @param nameSpace           Namespace in which the method is to be
                                   placed
        @param inheritedMethod     CIMConstMethod containing the method from
                                   the class or superclass
    */
    static void resolveMethod(
        CIMMethod& theMethod,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace,
        const CIMConstMethod& inheritedMethod);

    /**
        Resolves the CIMMethod.  Validates the qualifiers and parameters of the
        method.

        @param theMethod           CIMMethod object to be resolved
        @param declContext         Defines the context in which the method is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the method.
        @param nameSpace           Namespace in which the method is to be
                                   placed
    */
    static void resolveMethod(
        CIMMethod& theMethod,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace);

    /**
        Resolves the parameter.  Validates the qualifiers of the parameter.

        @param theParameter        CIMParameter object to be resolved
        @param declContext         Defines the context in which the parameter is
                                   to be resolved.  This provides the basis for
                                   other functions to get information from the
                                   context to use to resolve the parameter.
        @param nameSpace           Namespace in which the parameter is to be
                                   placed
    */
    static void resolveParameter(
        CIMParameter& theParameter,
        DeclContext* declContext,
        const CIMNamespaceName& nameSpace);

    /**
        Resolves the qualifier flavor.  This function is used only in object
        creation to resolve the combination of a qualifer flavor input and
        the corresponding inherited flavor from declaration or superclass and
        set the current qualifier to that definition.  The function changes
        the current flavor based on the characteristics of the inheritance.

        @param theQualifier      The CIMQualifier object for which to resolve
                                 the flavor.
        @param inheritedFlavor   The flavor inherited from higher level
    */
    static void resolveQualifierFlavor(
        CIMQualifier& theQualifier,
        const CIMFlavor& inheritedFlavor);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Resolver_h */
