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

#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Repository/NameSpaceManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

static Boolean verbose;

Array<CIMNamespaceName> arr1;
Array<CIMNamespaceName> arro;
Array<CIMName> Ns1SubClasses;
Array<CIMName> Srw1Ns1SubClasses;
Array<CIMName> Sns1NoDiSubClasses;
Array<CIMName> Class2Ns1SubClasses;
Array<CIMName> Sub1Sub4Sub1Class2Ns1SuperClasses;
Array<CIMName> Class1Sns1SubClasses;

void createNameSpaces(NameSpaceManager& nsm)
{
     // Create an array of names
     arr1.append(CIMNamespaceName("root/Ns1"));
     arr1.append(CIMNamespaceName("root/Srw1Ns1"));
     arr1.append(CIMNamespaceName("root/Sro1Srw1Ns1"));
     arr1.append(CIMNamespaceName("root/Sro2Sro1Ns1"));

     for (int i=0,m=arr1.size(); i<m; i++)
        arro.append(arr1[i]);

     // create the namespaces

     nsm.createNameSpace(arr1[0], true, true, String::EMPTY);

     nsm.createNameSpace(arr1[1], true, true, arr1[0].getString());

     nsm.createNameSpace(arr1[2], true, false, arr1[1].getString());

     nsm.createNameSpace(arr1[3], true, false, arr1[2].getString());
}
//

// Namespaces                              class trees                     .
// ----------                              -----------                     .
//                                                                         .
// root/Ns1:                                Class1Ns1                      .
//                                         /         \                     .
// root/Ns1:                       Sub1Class1Ns1  Sub2Class1Ns1            .
//                                                                         .
//                                                                         .
// root/Ns1:                                Class2Ns1                      .
//                                         /         \                     .
// root/Ns1:                       Sub1Class2Ns1  Sub2Class2Ns1            .
//                                /             \                          .
// root/Srw1Ns1:         Sub3Sub1Class2Ns1 Sub4Sub1Class2Ns1               .
//                                                          \              .
// root/Srw1Ns1:                                    Sub1Sub4Sub1Class2Ns1  .
//                                                                         .
//                                                                         .
// root/Srw1Ns1:                            Class1Sns1                     .
//                                         /          \                    .
// root/Srw1Ns1:                   Sub1Class1Sns1 Sub2Class1Sns1           .
//

void test01(NameSpaceManager& nsm)
{
    nsm.createClass (arr1[0], "Class1Ns1", CIMName());
     Ns1SubClasses.append("Class1Ns1");
     Sns1NoDiSubClasses.append("Class1Ns1");

    nsm.createClass (arr1[0], "Class2Ns1", CIMName());
     Ns1SubClasses.append("Class2Ns1");
     Sns1NoDiSubClasses.append("Class2Ns1");

    nsm.createClass (arr1[0], "Sub1Class1Ns1", CIMName("Class1Ns1"));
     Ns1SubClasses.append("Sub1Class1Ns1");

    nsm.createClass (arr1[0], "Sub2Class1Ns1", CIMName("Class1Ns1"));
     Ns1SubClasses.append("Sub2Class1Ns1");

    nsm.createClass (arr1[0], "Sub1Class2Ns1", CIMName("Class2Ns1"));
     Ns1SubClasses.append("Sub1Class2Ns1");
     Class2Ns1SubClasses.append("Sub1Class2Ns1");

    nsm.createClass (arr1[0], "Sub2Class2Ns1", CIMName("Class2Ns1"));
     Ns1SubClasses.append("Sub2Class2Ns1");
     Class2Ns1SubClasses.append("Sub2Class2Ns1");


    Srw1Ns1SubClasses=Ns1SubClasses;


    nsm.createClass (arr1[1], "Class1Sns1", CIMName());
     Srw1Ns1SubClasses.append("Class1Sns1");
     Sns1NoDiSubClasses.append("Class1Sns1");

    nsm.createClass (arr1[1], "Sub1Class1Sns1", CIMName("Class1Sns1"));
     Srw1Ns1SubClasses.append("Sub1Class1Sns1");
     Class1Sns1SubClasses.append("Sub1Class1Sns1");

    nsm.createClass (arr1[1], "Sub2Class1Sns1", CIMName("Class1Sns1"));
     Srw1Ns1SubClasses.append("Sub2Class1Sns1");
     Class1Sns1SubClasses.append("Sub2Class1Sns1");

    nsm.createClass (arr1[1], "Sub3Sub1Class2Ns1",
            CIMName("Sub1Class2Ns1")); //
     Srw1Ns1SubClasses.append("Sub3Sub1Class2Ns1");
     Class2Ns1SubClasses.append("Sub3Sub1Class2Ns1");

    nsm.createClass (arr1[1], "Sub4Sub1Class2Ns1", CIMName("Sub1Class2Ns1")); //
     Srw1Ns1SubClasses.append("Sub4Sub1Class2Ns1");
     Class2Ns1SubClasses.append("Sub4Sub1Class2Ns1");

    nsm.createClass (arr1[1], "Sub1Sub4Sub1Class2Ns1",
        CIMName("Sub4Sub1Class2Ns1"));
     Srw1Ns1SubClasses.append("Sub1Sub4Sub1Class2Ns1");
     Class2Ns1SubClasses.append("Sub1Sub4Sub1Class2Ns1");
     Sub1Sub4Sub1Class2Ns1SuperClasses.append("Sub4Sub1Class2Ns1");
     Sub1Sub4Sub1Class2Ns1SuperClasses.append("Sub1Class2Ns1");
     Sub1Sub4Sub1Class2Ns1SuperClasses.append("Class2Ns1");
}

void test02(NameSpaceManager& nsm) {

    Array<CIMName> classNames;
    nsm.getSubClassNames(arr1[0],CIMName(),true,classNames);

    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance "
        <<arr1[0].getString()<<endl;
    if (verbose) for (int i=0, m=classNames.size(); i<m; i++) {
       cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Ns1SubClasses);
    PEGASUS_TEST_ASSERT(Ns1SubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[1],CIMName(),false,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames No-deepInheritance "
        <<arr1[1].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Sns1NoDiSubClasses);
    PEGASUS_TEST_ASSERT(Sns1NoDiSubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[1],CIMName(),true,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance "
        <<arr1[1].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Srw1Ns1SubClasses);
    PEGASUS_TEST_ASSERT(Srw1Ns1SubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[2],CIMName(),true,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance "
        <<arr1[2].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Srw1Ns1SubClasses);
    PEGASUS_TEST_ASSERT(Srw1Ns1SubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[3],CIMName(),true,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance "
        <<arr1[3].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Srw1Ns1SubClasses);
    PEGASUS_TEST_ASSERT(Srw1Ns1SubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[1],CIMName("Class2Ns1"),true,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance Class2Ns1 "
        <<arr1[1].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Class2Ns1SubClasses);
    PEGASUS_TEST_ASSERT(Class2Ns1SubClasses == classNames);

    classNames.clear();
    nsm.getSubClassNames(arr1[1],CIMName("Class1Sns1"),true,classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SubClassNames deepInheritance Class1Sns11 "
        <<arr1[1].getString()<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Class1Sns1SubClasses);
    PEGASUS_TEST_ASSERT(Class1Sns1SubClasses == classNames);

    classNames.clear();
    nsm.getSuperClassNames(arr1[1],CIMName("Sub1Sub4Sub1Class2Ns1"),classNames);
    BubbleSort(classNames);
    if (verbose) cout<<"------- SuperClassNames of Sub1Sub4Sub1Class2Ns1"<<endl;
    for (int i=0, m=classNames.size(); i<m; i++) {
       if (verbose) cout<<"--- class: "<<classNames[i].getString()<<endl;
    }
    BubbleSort(Sub1Sub4Sub1Class2Ns1SuperClasses);
    PEGASUS_TEST_ASSERT(Sub1Sub4Sub1Class2Ns1SuperClasses == classNames);

}

int main(int, char** argv)
{

    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    NameSpaceManager nsm;

    try {
    createNameSpaces(nsm);
        test01(nsm);
        test02(nsm);
    }
    catch (Exception& e) {
    cout << argv[0] << " " << e.getMessage() << endl;
        exit (1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}
