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
//%////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/List.h>
#include <Pegasus/Common/String.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class Person : public Linkable
{
public:

    Person(const String& name) : _name(name) { }

    ~Person() { }

    const String& name() const { return _name; }

    void print() const { cout << "Person(" << _name << ")" << endl; }

    static bool equal(const Person* person, const void* client_data)
    {
    return *((String*)client_data) == person->name();
    }

private:
    Person(const Person& x);
    Person& operator=(const Person& x);
    String _name;
};

typedef List<Person,NullLock> PersonList;

int main(int, char** argv)
{
    // Create list of persons:

    PersonList list;
    PEGASUS_TEST_ASSERT(list.size() == 0);
    list.insert_back(new Person("John"));
    PEGASUS_TEST_ASSERT(list.size() == 1);
    list.insert_back(new Person("Jane"));
    PEGASUS_TEST_ASSERT(list.size() == 2);
    list.insert_back(new Person("Joe"));
    PEGASUS_TEST_ASSERT(list.size() == 3);
    list.insert_back(new Person("Bob"));
    PEGASUS_TEST_ASSERT(list.size() == 4);

    // Print all elements of the list:

    {
    PersonList::AutoLock autoLock(list);

    for (Person* p = list.front(); p; p = list.next_of(p))
    {
        // p->print();
    }
    }

    // Find "John":

    {
    const String JOHN = "John";
    Person* john = list.find(Person::equal, &JOHN);
    PEGASUS_TEST_ASSERT(john);
    // john->print();
    }

    // Remove "John" and "Jane":
    {
    const String JOHN = "John";
    Person* john = list.remove(Person::equal, &JOHN);
    PEGASUS_TEST_ASSERT(john->name() == "John");
    delete john;
    PEGASUS_TEST_ASSERT(list.size() == 3);

    const String JANE = "Jane";
    Person* jane = list.remove(Person::equal, &JANE);
    PEGASUS_TEST_ASSERT(jane->name() == "Jane");
    delete jane;
    PEGASUS_TEST_ASSERT(list.size() == 2);
    }

    // Clear the list:
    {
    list.clear();
    PEGASUS_TEST_ASSERT(list.size() == 0);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
