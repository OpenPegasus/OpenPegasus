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

#ifndef Pegasus_InheritanceTree_h
#define Pegasus_InheritanceTree_h

#include <iostream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Repository/Linkage.h>
#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

struct InheritanceTreeRep;
class NameSpace;

/** The InheritanceTree class tracks inheritance relationships of CIM classes.

    This class is a memory resident version of the repository's persistent
    inheritance information (represented using file names). The InheritanceTree
    provides O(1) access (via hashing) to any class in the inheritance tree.

    The inheritance tree provides methods for interrogating certain kinds of
    information about a class, including:

        <ul>
        <li>the superclass</li>
        <li>the subclasses</li>
        <li>the descendent classes</li>
        </ul>

    The insert() method is used to build up an InheritanceTree. The insert()
    method is called for each class-subclass relationship. For example, consider
    the following list of class-subclass pairs:

        <pre>
        { "D", "B" }
        { "E", "B" }
        { "B", "A" }
        { "C", "A" }
        { "F", "C" }
        </pre>

    These pairs specify the following inheritance tree:

        <pre>
              A
            /   \
           B     C
         /   \     \
        D     E     F
        </pre>

    The pairs above may be used to build a class tree as follows:

        <pre>
        InheritanceTree it;
        it.insert("D", "B");
        it.insert("E", "B");
        it.insert("B", "A");
        it.insert("C", "A");
        it.insert("F", "C");
        it.insert("A", "");
        it.check();
        </pre>

    The check() method determines whether insert() was called for every class
    used as a superclass. In the following example, check() would fail (and
    throw and exception) since the "B" class is passed as a superclass (second
    argument) in two insert() calls but was never passed as the class itself
    (first argument) in any insert() call:

        <pre>
        InheritanceTree it;
        it.insert("D", "B");
        it.insert("E", "B");
        it.insert("C", "A");
        it.insert("F", "C");
        it.insert("A", "");
        it.check();
        </pre>

    In this case, check() throws an InvalidInheritanceTree exception.

    The InheritanceTree may be printed by calling the print() method.
*/
class PEGASUS_REPOSITORY_LINKAGE InheritanceTree
{
public:

    /** Default constructor. */
    InheritanceTree();

    /** Destructor. */
    ~InheritanceTree();

    /** Inserts a class-subclass relationship into the inheritance three.
        Note that a class CAN be inserted before its superclass, in which case
        a provisional entry is made for the superclass and flagged as such;
        when the superclass is later inserted, the provisional flag is cleared.
        @param className - name of class being inserted.
        @param superClassName - name of super class of class.
    */
    void insert(const String& className, const String& superClassName);
    void insert(
       const String& className,
       const String& superClassName,
       InheritanceTree& parentTree,
       NameSpace* parent);

    /** Checks that every superClassName passed to insert() was also passed
        as a className argument to insert(). In other words, it checks that
        there are no provisional entries as described in the insert() method.
        @exception InvalidInheritanceTree
    */
    void check() const;

    /** Get subclass names of the given class.
        @param className - class whose subclass names will be gotten. If
            className is empty, all classnames are returned.
        @param deepInheritance - if true all descendent classes of class
            are returned. If className is empty, only root classes are returned.
        @param subClassNames - output argument to hold subclass names.
        @return true on success. False if no such class.
    */
    Boolean getSubClassNames(
        const CIMName& className,
        Boolean deepInheritance,
        Array<CIMName>& subClassNames,
        NameSpace* ns = NULL) const;

#if 0
    /** Returns true if class1 is a subclass of class2.
    */
    Boolean isSubClass(const CIMName& class1, const CIMName& class2) const;
#endif

    /** Get the names of all superclasses of this class (direct and indirect).
    */
    Boolean getSuperClassNames(
        const CIMName& className,
        Array<CIMName>& subClassNames) const;

    /** Get the superclass of the given class.
        @param className name of class.
        @param superClassName name of superclass upon return.
        @return true if class was found; false otherwise.
    */
    Boolean getSuperClass(
        const CIMName& className,
        CIMName& superClassName) const;

    /** Returns true if the given class has sub-classes. */
    Boolean hasSubClasses(
        const CIMName& className,
        Boolean& hasSubClasses) const;

    /** Returns true if this inhertance tree contains the given class. */
    Boolean containsClass(const CIMName& className) const;

    /** Removes the given class from the class graph.
        @exception CIMException(CIM_ERR_CLASS_HAS_CHILDREN)
        @exception CIMException(CIM_ERR_INVALID_CLASS)
    */
    void remove(const CIMName& className,
        InheritanceTree& parentTree,
        NameSpace* ns = NULL);

    /** Prints the class */
    void print(PEGASUS_STD(ostream)& os) const;

private:

    InheritanceTree(const InheritanceTree&) { }

    InheritanceTree& operator=(const InheritanceTree&) { return *this; }

    InheritanceTreeRep* _rep;

    friend struct InheritanceTreeNode;
};

/** The InvalidInheritanceTree exception is thrown when the
    InheritanceTreeRep::check() method determines that an inheritance tree
    was not fully specified (when any class was passed as a superClassName
    argument to insert() but never as a className argument.
*/
class PEGASUS_REPOSITORY_LINKAGE InvalidInheritanceTree : public Exception
{
public:
    InvalidInheritanceTree(const String& className)
    : Exception(MessageLoaderParms(
          "Repository.InheritanceTree.INVALID_INHERITANCE_TREE",
          "Invalid inheritance tree: unknown class: $0", className))
    {
    }
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_InheritanceTree_h */
