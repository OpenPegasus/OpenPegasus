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

How to run this test:
=====================

1.  This test requires that both the root/cimv2 and
    root/SampleProvider namespaces have been populated.

    In particular, this test requires the existence
    of CIM_ManagedElement and CIM_ComputerSystem in root/cimv2.
    It also requires that the class and instance definitions
    defined in pegasus/src/Providers/sample/Load/ClientTest.mof
    are loaded into the root/SampleProvider namespace.

    The Makefile in the pegasus directory can be
    used to populate both namespaces.

    To populate the namespaces using cimmofl, use ...

    $(MAKE) repository
    $(MAKE) testrepository

    To populate the namespaces using cimmof, use ...

    $(MAKE) repositoryServer
    $(MAKE) testrepositoryServer

2.  Run this test.

    % Client

Here is the expected output:
============================

[//homefront/root/cimv2:Person.name="Sofi"]
[//homefront/root/cimv2:Person.name="Gabi"]

[//homefront/root/cimv2:Person.name="Sofi"]
[//homefront/root/cimv2:Person.name="Gabi"]

[//homefront/root/cimv2:Lineage.child="person.name=\"Sofi\"",parent="person.name=\"Mike\""]
[//homefront/root/cimv2:Lineage.child="person.name=\"Gabi\"",parent="person.name=\"Mike\""]

[//homefront/root/cimv2:Lineage.child="person.name=\"Sofi\"",parent="person.name
=\"Mike\""]
[//homefront/root/cimv2:Lineage.child="person.name=\"Gabi\"",parent="person.name
=\"Mike\""]

[//homefront/root/cimv2:Person]

[//homefront/root/cimv2:Lineage]
[//homefront/root/cimv2:LabeledLineage]

+++++ passed all tests
