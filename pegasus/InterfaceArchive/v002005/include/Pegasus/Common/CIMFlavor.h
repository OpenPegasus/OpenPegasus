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

#ifndef Pegasus_Flavor_h
#define Pegasus_Flavor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMFlavor class represents the DMTF standard CIM qualifier flavor
    definition, which encapsulates the propagation and override rules for
    qualifiers.  The propagation rules define whether a qualifier may be
    propagated from classes to derived classes or from classes to instances.
    The override rules define whether a derived class may override a
    qualifier value.

    A CIMFlavor contains one or more of these values: OVERRIDABLE,
    TOSUBCLASS, TOINSTANCE, TRANSLATABLE, DISABLEOVERRIDE, ENABLEOVERRIDE,
    RESTRICTED and DEFAULTS.
*/
class PEGASUS_COMMON_LINKAGE CIMFlavor
{
public:

    /**
        Constructs a CIMFlavor object with the value NONE.
    */
    CIMFlavor();

    /**
        Constructs a CIMFlavor object from the value of a specified
        CIMFlavor object.
        @param flavor The CIMFlavor object from which to construct a new
            CIMFlavor object.
    */
    CIMFlavor(const CIMFlavor & flavor);

    /**
        Assigns the value of the specified CIMFlavor object to this object.
        @param scope The CIMFlavor object from which to assign this
            CIMFlavor object.
        @return A reference to this CIMFlavor object.
    */
    CIMFlavor& operator=(const CIMFlavor& flavor);

    /**
        Adds flavor values to the CIMFlavor object.
        @param flavor A CIMFlavor containing the flavor values to add.
    */
    void addFlavor(const CIMFlavor& flavor);

    /**
        Removes flavor values from the CIMFlavor object.
        @param flavor A CIMFlavor containing the flavor values to remove.
    */
    void removeFlavor(const CIMFlavor& flavor);

    /**
        Checks whether the flavor contains specified flavor values.
        @param flavor A CIMFlavor specifying the flavor values to check.
        @return True if the flavor contains all the values in the specified
            CIMFlavor object, false otherwise.
    */
    Boolean hasFlavor(const CIMFlavor& flavor) const;

    /**
        Compares the CIMFlavor with a specified CIMFlavor.
        @param flavor The CIMFlavor to be compared.
        @return True if this flavor has the same set of values as the
            specified flavor, false otherwise.
    */
    Boolean equal(const CIMFlavor& flavor) const;

    /**
        Adds two flavor values.
        @param flavor A CIMFlavor containing the flavor value to add to this
            flavor.
        @return A new CIMFlavor object containing a union of the values in the
            two flavor objects.
    */
    CIMFlavor operator+(const CIMFlavor& flavor) const;

    /**
        Returns a String representation of the CIMFlavor object.
        This method is for diagnostic purposes. The format of the output
        is subject to change.
        @return A String containing a human-readable representation of the
            flavor value.
    */
    String toString() const;

    /**
        Indicates that the qualifier has no flavors.
    */
    static const CIMFlavor NONE;

    /**
        Indicates that the qualifier may be overridden.
    */
    static const CIMFlavor OVERRIDABLE;

    /**
        Indicates that the override feature is enabled for the qualifier.
        Thus, the qualifier may be overridden.
    */
    static const CIMFlavor ENABLEOVERRIDE;

    /**
        Indicates that the override feature is disabled for the qualifier.
        Thus, the qualifier may not be overridden.
    */
    static const CIMFlavor DISABLEOVERRIDE;

    /**
        Indicates that the qualifier is propagated to subclasses.
    */
    static const CIMFlavor TOSUBCLASS;

    /**
        Indicates that the qualifier is not propagated to subclasses.
    */
    static const CIMFlavor RESTRICTED;

    /**
        Indicates that the qualifier is propagated to instances.
    */
    static const CIMFlavor TOINSTANCE;

    /**
        Indicates that the qualifier is translatable (for
        internationalization).
    */
    static const CIMFlavor TRANSLATABLE;

    /**
        Indicates the default flavor settings (OVERRIDABLE | TOSUBCLASS).
    */
    static const CIMFlavor DEFAULTS;

    /**
        Indicates that the qualifier is propagated to subclasses and to
        instances (TOSUBCLASS | TOINSTANCE).
    */
    static const CIMFlavor TOSUBELEMENTS;

private:

    /*
        Constructs a CIMFlavor object with the specified values.
        @param flavor A Uint32 representing the set of flavor values.
    */
    CIMFlavor(const Uint32 flavor);

    Uint32 cimFlavor;

    friend class BinaryStreamer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Flavor_h */
