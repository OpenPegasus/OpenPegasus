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

#ifndef Pegasus_ObjectPath_h
#define Pegasus_ObjectPath_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

class CIMObjectPath;
class CIMKeyBindingRep;
class CIMObjectPathRep;
class CIMValue;

/** The CIMKeyBinding class associates a key name, value, and type.
    It is used by the CIMObjectPath class to represent key bindings.
    See the CIMObjectPath class to see how they are used.
*/
class PEGASUS_COMMON_LINKAGE CIMKeyBinding
{
public:

    enum Type { BOOLEAN, STRING, NUMERIC, REFERENCE };

    /** Default constructor. */
    CIMKeyBinding();

    /** Copy constructor. */
    CIMKeyBinding(const CIMKeyBinding& x);

    /** Constructs a CIMKeyBinding with a name, value, and type.
        @param name CIMName for the key for this binding object.
        @param value String value for this key.
        @param type CIMKeyBinding::Type representing the type of this key.
    */
    CIMKeyBinding(const CIMName& name, const String& value, Type type);

    /** Constructs a CIMKeyBinding with a name and CIMValue, mapping from
        CIMValue types to CIMKeyBinding types.
        @param name CIMName for the key for this binding object.
        @param value CIMValue from which to extract the value for this key.
        @exception TypeMismatchException if the type of the value is not valid
        for a key property.
    */
    CIMKeyBinding(const CIMName& name, const CIMValue& value);

    /** Destructor */
    ~CIMKeyBinding();

    /** Assignment operator. */
    CIMKeyBinding& operator=(const CIMKeyBinding& x);

    ///
    const CIMName& getName() const;

    ///
    void setName(const CIMName& name);

    ///
    const String& getValue() const;

    ///
    void setValue(const String& value);

    ///
    Type getType() const;

    /// 
    void setType(Type type);

    ///
    Boolean equal(CIMValue value);

private:

    CIMKeyBindingRep* _rep;

    friend class CIMObjectPath;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMKeyBinding& x,
    const CIMKeyBinding& y);

#define PEGASUS_ARRAY_T CIMKeyBinding
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

class XmlWriter;

/** The CIMObjectPath class represents the value of a reference. A reference
    is one of the property types that an association may contain. Consider the
    following MOF for example:

    <pre>
    [Association]
    class MyAssociations
    {
        MyClass ref from;
        MyClass ref to;
    };
    </pre>

    The value of the from and to properties are internally represented using
    the CIMObjectPath class.

    A CIM reference is used to uniquely identify a CIM class or CIM instance
    object. CIMObjectPath objects contain the following parts:

    <ul>
    <li>Host - name of host that contains the object</li>
    <li>NameSpace - the namespace which contains the object</li>
    <li>ClassName - name of objects class</li>
    <li>KeyBindings key/value pairs which uniquely identify an instance</li>
    </ul>

    CIM references may also be expressed as simple strings (as opposed to
    being represented by the CIMObjectPath class). This string is known as
    the "Object Name". An object name has the following form:

    <pre>
    &lt;namespace-path&gt;:&lt;model-path&gt;
    </pre>

    As for the model-path mentioned above, its form is defined by the CIM
    Standard (more is defined by the "XML Mapping Specification v2.0.0"
    specification) as follows:

    <pre>

    &lt;Qualifyingclass&gt;.&lt;key-1&gt;=&lt;value-1&gt;[,&lt;key-n&gt;=
    &lt;value-n&gt;]*
    </pre>

    For example:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    </pre>

    This of course presupposes the existence of a class called "TennisPlayer"
    that has key properties named "first" and "last". For example, here is what
    the MOF might look like:

    <pre>
    class TennisPlayer : Person
    {
        [key] string first;
        [key] string last;
    };
    </pre>

    All keys must be present in the model path.

    Now the namespace-type and model-path are combined in the following
    string object name.

    //atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter"

    Now suppose we wish to create a CIMObjectPath from this above string. There
    are two constructors provided: one which takes the above string and the
    other that takes the constituent elements. Here are the signature of the
    two constructors:

    <pre>
    CIMObjectPath(const String& objectName);

    CIMObjectPath(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Array<CIMKeyBinding>& keyBindings);
    </pre>

    Following our example, the above object name may be used to initialize
    a CIMObjectPath like this:

        <pre>
        CIMObjectPath ref =
            "//atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter";
        </pre>

    A CIMObjectPath may also be initialized using the constituent elements
    of the object name (sometimes the object name is not available as a string:
    this is the case with CIM XML encodings). The arguments shown in that
    constructor above correspond elements of the object name in the following
    way:

    <ul>
    <li>host = "atp:9999"</li>
    <li>nameSpace = "root/cimv25"</li>
    <li>className = "TennisPlayer"</li>
    <li>keyBindings = "first=\"Patrick\",last=\"Rafter\""</li>
    </ul>

    Note that the host and nameSpace argument may be empty since object names
    need not necessarily include a namespace path according to the standard.

    The key bindings must be built up by appending CIMKeyBinding objects
    to an Array of CIMKeyBindings like this:

    <pre>
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("first", "Patrick", CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("last", "Rafter", CIMKeyBinding::STRING));
    </pre>

    The key binding types that are supported are:

    <ul>
    <li>CIMKeyBinding::BOOLEAN</li>
    <li>CIMKeyBinding::STRING</li>
    <li>CIMKeyBinding::NUMERIC</li>
    <li>CIMKeyBinding::REFERENCE</li>
    </ul>

    The CIM types are encoded as one of the key binding types in the
    following way

    <pre>
    boolean - BOOLEAN (the value must be "true" or "false")
    uint8 - NUMERIC
    sint8 - NUMERIC
    uint16 - NUMERIC
    sint16 - NUMERIC
    uint32 - NUMERIC
    sint32 - NUMERIC
    uint64 - NUMERIC
    sint64 - NUMERIC
    char16 - NUMERIC
    string - STRING
    datetime - STRING
    </pre>

    Notice that real32 and real64 are missing. Properties of these types
    cannot be used as keys.

    Notice that the keys in the object name may appear in any order.
    That is the following object names refer to the same object:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer.last="Rafter",first="Patrick"
    </pre>

    And since CIM is not case sensitive, the following refer to the same
    object:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    tennisplayer.FIRST="Patrick",Last="Rafter"
    </pre>

    Therefore, the CIMObjectPaths::operator==() would return true for the last
    two examples.

    The CIM standard leaves it an open question whether model paths may have
    spaces around delimiters (like '.', '=', and ','). We assume they cannot.
    So the following is an invalid model path:

    <pre>
    TennisPlayer . first = "Patrick", last="Rafter"
    </pre>

    We require that the '.', '=', and ',' have no spaces around them.

    For reasons of efficiency, the key bindings are internally sorted
    during initialization. This allows the key bindings to be compared
    more easily. This means that when the string is converted back to
    string (by calling toString()) that the keys may have been rearranged.

    There are two forms an object name can take:

    <pre>
    &lt;namespace-path&gt;:&lt;model-path&gt;
    &lt;model-path&gt;
    </pre>

    In other words, the namespace-path is optional. Here is an example of
    each:

    <pre>
    //atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer.first="Patrick",last="Rafter"
    </pre>

    If it begins with "//" then we assume the namespace-path is present and
    process it that way.

    It should also be noted that an object name may refer to an instance or
    a class. Here is an example of each:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer
    </pre>

    In the second case--when it refers to a class--the key bindings are
    omitted.
*/
class PEGASUS_COMMON_LINKAGE CIMObjectPath
{
public:

    /** Default constructor. */
    CIMObjectPath();

    /** Copy constructor. */
    CIMObjectPath(const CIMObjectPath& x);

    /** Initializes a CIMObjectPath object from a CIM object name.
        @param objectName String representing the object name.
        @return Returns the initialized CIMObjectPath
        @exception MalformedObjectNameException if the name is not parsable.
        <PRE>
            CIMObjectPath r1 = "MyClass.z=true,y=1234,x=\"Hello World\"";
        </PRE>
    */
    CIMObjectPath(const String& objectName);

    /** Constructs a CIMObjectPath from constituent elements.
        @param host Name of host (e.g., "nemesis-5988").
        @param nameSpace Namespace (e.g., "root/cimv2").
        @param className Name of a class (e.g., "MyClass").
        @param keyBindings An array of CIMKeyBinding objects.
        @return Returns the constructed CIMObjectPath
    */
    CIMObjectPath(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        //
        //  NOTE: Due to a bug in MSVC 5, the following will not work on MSVC 5
        //
        const Array<CIMKeyBinding>& keyBindings = Array<CIMKeyBinding>());

    /** Destructor. */
    ~CIMObjectPath();

    /** Assignment operator. */
    CIMObjectPath& operator=(const CIMObjectPath& x);

    /** Clears out the internal fields of this object making it an empty
        (or uninitialized) reference. The effect is the same as if the object
        was initialized with the default constructor.
    */
    void clear();

    /** Sets this reference from constituent elements. The effect is same
        as if the object was initialized using the constructor above that
        has the same arguments.
        @exception MalformedObjectNameException if host name is illformed.
    */
    void set(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        //
        //  NOTE: Due to a bug in MSVC 5, the following will not work on MSVC 5
        //
        const Array<CIMKeyBinding>& keyBindings = Array<CIMKeyBinding>());

    /** Set the reference from an object name . */
    void set(const String& objectName);

    /** Same as set() above except that it is an assignment operator. */
    CIMObjectPath& operator=(const String& objectName);

    /** Gets the hostname component of the CIMObjectPath.
        @return String containing hostname.
    */
    const String& getHost() const;

    /** Sets the hostname component of the CIMObjectPath
        object to the input parameter.
        @param host String parameter with the hostname
        <PRE>
        CIMObjectPath r1;
        r1.setHost("fred:5988");
        </PRE>
        Note that Pegasus does no checking on valid host names.
    */
    void setHost(const String& host);

    /** Gets the namespace component of the
        CIMObjectPath as a CIMNamespaceName.
    */
    const CIMNamespaceName& getNameSpace() const;

    /** Sets the namespace component.
        @param nameSpace CIMNamespaceName representing the namespace.
    */
    void setNameSpace(const CIMNamespaceName& nameSpace);

    /** Gets the className component of the CIMObjectPath.
        @return CIMName containing the classname.
     */
    const CIMName& getClassName() const;

    /** Sets the classname component of the CIMObjectPath object to 
        the input parameter.
        @param className CIMName containing the className.
    */
    void setClassName(const CIMName& className);

    /** Returns an Array of keybindings from the
        CIMObjectPath representing all of the key/value pairs defined 
        in the ObjectPath.
        @return Array of CIMKeyBinding objects from the CIMObjectPath.
    */
    const Array<CIMKeyBinding>& getKeyBindings() const;

    /** Sets the key/value pairs in the CIMObjectPath
        from an array of keybindings defined by the input parameter.
        @param keyBindings Array of keybindings to set into the CIMObjectPath
        object.
    */
    void setKeyBindings(const Array<CIMKeyBinding>& keyBindings);


    /** Returns the object name represented by this reference. The returned
        string is formed from the hostname, namespace, classname
        and keybindings defined for this CIMObjectPath object.
        The form of the name is:

        <PRE>
            "//" + hostname + "/" + namespace + ":" + classname +"." +
            (keyname) + "=" (keyvalue) +"," ...
        </PRE>

        The building includes the escaping of special characters.
    */
    String toString() const;

    /** Returns true if this reference is identical to the one given
        by the x argument. Since CIMObjectPaths are normalized when they
        are created, any differences in the ordering of keybindings is accounted
        for as are the case insensitivity characteristics defined by
        the specification.
        @param x CIMObjectPath for comparison.
        @return true if the objects have identical components, false otherwise.
    */
    Boolean identical(const CIMObjectPath& x) const;

    /** Generates hash code for the given reference. Two identical references
        generate the same hash code (despite any subtle differences such as
        the case of the classname and key names as well as the order of the
        keys).
    */
    Uint32 makeHashCode() const;


private:

    /** Stringizes object into canonical form (in which all keys are sorted
        into ascending order and classnames and keynames are shifted to
        lower case.
    */
    String _toStringCanonical() const;

    CIMObjectPathRep* _rep;
};

PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMObjectPath& x,
    const CIMObjectPath& y);

PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const CIMObjectPath& x,
    const CIMObjectPath& y);

#define PEGASUS_ARRAY_T CIMObjectPath
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ObjectPath_h */


