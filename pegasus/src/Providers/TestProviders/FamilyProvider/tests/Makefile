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
# Test the Family Provider using the script toosls

ROOT = ../../../../..
include $(ROOT)/mak/config.mak
include $(ROOT)/mak/configschema.mak
SHH = @

# sampleprovider\Load puts the sample MOF into a separate namespace.
# The following variable defines this namespace. Note that
# originally this Makefile was to do the install but that
# was moved to sampleprovider/load

PROVIDERNS=test/TestProvider

QUOTE=\"

ifeq ($(OS_TYPE),vms)
QUOTE=""
endif

depend:

sub:

misc:

tests:

messages:

poststarttests:

	@$(ECHO) 1. +++++ Test Classes to be sure all classes are here

	$(SHH)cimcli ec "TST_Person" -n "$(PROVIDERNS)" --sum -count 2
	$(SHH)cimcli ec "TST_Person" -n "$(PROVIDERNS)" --sum -di -count 3
	$(SHH)cimcli ec "TST_PersonS" -n "$(PROVIDERNS)" --sum -count 0
	$(SHH)cimcli ec "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ec "TST_Lineage" -n "$(PROVIDERNS)" --sum -count 0
	$(SHH)cimcli ec "TST_LineageDynamic" -n "$(PROVIDERNS)" --sum -count 0

	@$(ECHO) 2. +++++ Test all enumerate operations on the classes

	$(SHH)cimcli ei "TST_Person" -n "$(PROVIDERNS)" --sum -count 14
	$(SHH)cimcli ni "TST_Person" -n "$(PROVIDERNS)" --sum -count 14
	$(SHH)cimcli ei "TST_PersonS" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli ni "TST_PersonS" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli ei "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli ni "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 6

	$(SHH)cimcli ei "TST_PersonDynamicSubclass" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli ni "TST_PersonDynamicSubClass" -n "$(PROVIDERNS)" --sum -count 1

	$(SHH)cimcli ei "TST_Lineage" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli ni "TST_Lineage" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli ei "TST_LabeledLineage" -n "$(PROVIDERNS)" --sum -count 0
	$(SHH)cimcli ni "TST_LabeledLineage" -n "$(PROVIDERNS)" --sum -count 0

	$(SHH)cimcli ei "TST_LineageDynamic" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli ni "TST_LineageDynamic" -n "$(PROVIDERNS)" --sum -count 6

	$(SHH)cimcli ei "TST_LabeledLineageDynamic" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli ni "TST_LabeledLineageDynamic" -n "$(PROVIDERNS)" --sum -count 6

	@$(ECHO) 3. +++++ TST_Person Testing getInstance Family provider in  $(PROVIDERNS)

	$(SHH)cimcli gi "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli gi "TST_PersonDynamicSubClass.name=$(QUOTE)AnotherKid$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli gi "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 1

	@$(ECHO) 4. +++++ TST_Person Testing Family provider in  $(PROVIDERNS)

	$(SHH)cimcli ei "TST_Person" -n "$(PROVIDERNS)" --sum -count 14
	$(SHH)cimcli ni "TST_Person" -n "$(PROVIDERNS)" --sum -count 14
	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli r  "TST_Person" -n "$(PROVIDERNS)" --sum -count 4
	$(SHH)cimcli an "TST_Person" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli a  "TST_Person" -n "$(PROVIDERNS)" --sum -count 1
	$(SHH)cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 2
	$(SHH)cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 2
	$(SHH)cimcli an "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 2
	$(SHH)cimcli a  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 2

	@$(ECHO) 5. +++++ TST_Person Testing with  resultClass filters

	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -rc TST_Lineage --sum -count 1
	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -rc TST_Lineage --sum -count 1
	$(SHH)cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc TST_Lineage --sum -count 2
	$(SHH)cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc TST_Lineage --sum -count 2

	@$(ECHO) 6. ++++ TST_Person Testing with assocClass filter

	$(SHH)cimcli an "TST_Person" -n "$(PROVIDERNS)" -ac  "TST_Lineage" --sum -count 1
	$(SHH)cimcli a "TST_Person" -n "$(PROVIDERNS)" -ac "TST_Lineage" --sum -count 1
	$(SHH)cimcli an "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -ac "TST_Lineage" --sum -count 2
	$(SHH)cimcli a  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -ac "TST_Lineage" --sum -count 2

	@$(ECHO) 7. ++++ TST_Person Testing with role filter alone

	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -r parent --sum -count 4
	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -r parent --sum -count 4
	$(SHH)cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -r parent --sum -count 2
	$(SHH)cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -r parent --sum -count 2

	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -r child --sum -count 4
	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -r child --sum -count 4
	$(SHH)cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -r child --sum -count 0
	$(SHH)cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -r child --sum -count 0

	@$(ECHO) 8. +++++ TEST_Person Test with role and result class

	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -rc  TST_Lineage -r parent --sum -count 1
	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -rc  TST_Lineage -r parent --sum -count 1
	$(SHH)cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r parent --sum -count 2
	$(SHH)cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r parent --sum -count 2

	@$(ECHO) 9. ++++ TST_Person Test with errors in role and result class

# Bypassing following tests because they generate exception return from provider
# The error is valid because Provider does exception on bad property.
#   But we don't know how to handle that in a script with cimcli
#
#	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -rc  TST_Lineage -r junk --sum -count 0
#	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -rc  TST_Lineage -r junk --sum -count 0
#	$(SHH)cimcli rn TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r junk --sum -count 0
#	$(SHH)cimcli r  TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r junk --sum -count 0

#	$(SHH)cimcli rn "TST_Person" -n "$(PROVIDERNS)" -rc  "TST_Lineage" -r junk --sum -count 0
#	$(SHH)cimcli r "TST_Person" -n "$(PROVIDERNS)" -rc  "TST_Lineage" -r junk --sum -count 0
#	cimcli rn "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  "TST_Lineage" -r junk --sum -count 0
#	cimcli r  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  "TST_Lineage" -r junk --sum -count 0

	@$(ECHO) 10. ++++ tests for associations with  result class and result role.

	$(SHH)cimcli an  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r child --sum -count 0
#	$(SHH)cimcli an  "TST_Person.name=$(QUOTE)Mike$(QUOTE)" -n "$(PROVIDERNS)" -rc  TST_Lineage -r junk --sum -count 0

#	@$(ECHO) 11. +++++ TST_Person with result Class filters but with bad Class
#Note that we can add more tests here.

	@$(ECHO) 12. +++++ TST_PersonDynamic Family class Operations in  $(PROVIDERNS)

	$(SHH)cimcli ei "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli ni "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli rn "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 5
	$(SHH)cimcli r  "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 5
	$(SHH)cimcli an "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 2
	$(SHH)cimcli a  "TST_PersonDynamic" -n "$(PROVIDERNS)" --sum -count 2

	@$(ECHO) 13. +++++ TST_PersonDynamic and Family provider with simple class operations
 
	$(SHH)cimcli rn "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli r "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli an "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 6
	$(SHH)cimcli a "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 6

	@$(ECHO) "TST_PersonDynamic with bad object name (lower case)"

	$(SHH)cimcli rn "TST_PersonDynamic.name=$(QUOTE)father$(QUOTE)" -n "$(PROVIDERNS)" --sum -count 0

	@$(ECHO) 14. ++++ Test TST_PersonDynamic classes with  resultClass/AssocClass options

	$(SHH)cimcli rn "TST_PersonDynamic" -n "$(PROVIDERNS)" -rc TST_LineageDynamic --sum -count 1
	$(SHH)cimcli r "TST_PersonDynamic" -n "$(PROVIDERNS)" -rc TST_LineageDynamic --sum -count 1
	$(SHH)cimcli an "TST_PersonDynamic" -n "$(PROVIDERNS)" -ac TST_LineageDynamic --sum -count 1
	$(SHH)cimcli a "TST_PersonDynamic" -n "$(PROVIDERNS)" -ac TST_LineageDynamic --sum -count 1

	@$(ECHO) 15. ++++ TST_PersonDynamic instances with resultClass/AssociationClass options

	$(SHH)cimcli rn "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" -rc TST_LineageDynamic --sum -count 3
	$(SHH)cimcli r "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" -rc TST_LineageDynamic --sum -count 3
	$(SHH)cimcli an "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" -ac TST_LineageDynamic --sum -count 3
	$(SHH)cimcli a "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" -ac TST_LineageDynamic --sum -count 3
	$(SHH)cimcli a "TST_PersonDynamic.name=$(QUOTE)Father$(QUOTE)" -n "$(PROVIDERNS)" -ac TST_LineageDynamic --sum -count 3

	@$(ECHO) 16. ++++ TST_PersonDynamic association with resultClass role option

	$(SHH)cimcli an "TST_PersonDynamic" -n "$(PROVIDERNS)" -rc TST_Lineage --sum -count 0
	$(SHH)cimcli an "TST_PersonDynamic" -n "$(PROVIDERNS)" -rc TST_PersonDynamic --sum -count 1
	$(SHH)cimcli an "TST_PersonDynamic" -n "$(PROVIDERNS)" -rc TST_Person --sum -count 2

	@$(ECHO) +++++ All FamilyProvider tests Passed.

general:

clean:

# END_OF_FILE
