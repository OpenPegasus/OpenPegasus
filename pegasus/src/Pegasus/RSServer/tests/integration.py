# vim: tabstop=4 shiftwidth=4 softtabstop=4
#//%LICENSE////////////////////////////////////////////////////////////////
#//
#// Licensed to The Open Group (TOG) under one or more contributor license
#// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
#// this work for additional information regarding copyright ownership.
#// Each contributor licenses this file to you under the OpenPegasus Open
#// Source License; you may not use this file except in compliance with the
#// License.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a
#// copy of this software and associated documentation files (the "Software"),
#// to deal in the Software without restriction, including without limitation
#// the rights to use, copy, modify, merge, publish, distribute, sublicense,
#// and/or sell copies of the Software, and to permit persons to whom the
#// Software is furnished to do so, subject to the following conditions:
#//
#// The above copyright notice and this permission notice shall be included
#// in all copies or substantial portions of the Software.
#//
#// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//////////////////////////////////////////////////////////////////////////

import unittest
import urllib
import urllib2

class TestPegRSInstanceCollectionRetrieval(unittest.TestCase):
    def setUp(self):
        self.uri = "http://localhost:5988/cimrs/test%2FTestProvider/cmpiPerf_TestClassB/"
        self.response = urllib2.urlopen(self.uri)

    def testShouldHaveCimCollectionHeaders(self):
        self.assertEqual(200, self.response.code, "Should yield a HTTP status code of 200 OK")
        self.assertEqual("application/json", self.response.headers["Content-type"])

    def testEntityShouldBeArray(self):
        data = self.response.read()
        self.assertTrue(len(data) > 0)
        self.assertEqual('[', data[0], "Instance collection should be array")
        self.assertEqual(']', data[-1], "Instance collection should be array")

    def testEntityShouldContainCorrectData(self):
        try:
            import json
            instances = json.load(self.response)
            for instance in instances:
                self.assertEqual(type(int()), type(instance["theKey"]))
                self.assertEqual(type(unicode()), type(instance["$ref"]), "Link should be a string")
                self.assertEqual(int(instance["$ref"]), instance["theKey"], "Link should equal key")
                self.assertEqual(None, instance["Name"])
        except ValueError, e:
            self.fail(e.reason)
        except ImportError:
            # json module requires Python 2.6
            pass

class TestPegRSInstanceRetrieval(unittest.TestCase):
    def setUp(self):
        self.uri = "http://localhost:5988/cimrs/test%2FTestProvider/cmpiPerf_TestClassB/1"
        self.response = urllib2.urlopen(self.uri)

    def testShouldHaveCimInstanceHeader(self):
        self.assertEqual(200, self.response.code, "Should yield a HTTP status code of 200 OK")
        self.assertEqual("application/json", self.response.headers["Content-type"])

    def testEntityShouldBeObject(self):
        data = self.response.read()
        self.assertTrue(len(data) > 0, "Response should not be empty")
        self.assertEqual('{', data[0], "Response should be object")
        self.assertEqual('}', data[-1], "Response should be object")

    def testEntityShouldContainCorrectData(self):
        try:
            import json
            instance = json.load(self.response)
            self.assertEqual(type(int()), type(instance["theKey"]))
            self.assertEqual(1, instance["theKey"])
            self.assertFalse("$ref" in instance.keys(), "Should not have a link")
            self.assertEqual(None, instance["Name"])
        except ValueError, e:
            self.fail(e.reason)
        except ImportError:
            # json module requires Python 2.6
            pass

class TestPegRSClassRetrieval(unittest.TestCase):
    def setUp(self):
        self.uri = "http://localhost:5988/cimrs/test%2FTestProvider/cmpiPerf_TestClassB?IncludeQualifiers=true"
        self.response = urllib2.urlopen(self.uri)

    def testEntityShouldHaveCimClassHeader(self):
        self.assertEqual(200, self.response.code, "Should yield a HTTP status code of 200 OK")
        self.assertEqual("application/json", self.response.headers["Content-type"])

    def testEntityShouldBeObject(self):
        data = self.response.read()
        self.assertTrue(len(data) > 0, "Response should not be empty")
        self.assertEqual('{', data[0], "Response should be object")
        self.assertEqual('}', data[-1], "Response should be object")

    def testEntityShouldContainCorrectData(self):
        try:
            import json
            cimClass = json.load(self.response)

            self.assertEqual('cmpiPerf_TestClassB', cimClass['name'])
            self.assertEqual('CIM_EnabledLogicalElement', cimClass['superclass'])

            properties = cimClass['properties']
            self.assertEqual(26, len(properties.keys()))

            qualifiers = cimClass['qualifiers']
            self.assertEqual(2, len(qualifiers.keys()))

        except ValueError, e:
            self.fail(e.reason)
        except ImportError:
            # json module requires Python 2.6
            pass


if __name__ == '__main__':
    unittest.main()
