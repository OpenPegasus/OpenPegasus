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
ROOT = ../../../..

DIR = Clients/cimconfig/tests

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/test.mak

RESULTFILE =  $(TMP_DIR)/result
MASTERRESULTFILE = $(ROOT)/src/$(DIR)/result.master

tests:

poststarttests:
	@$(RM) $(RESULTFILE)
	@$(MAKE) -i -s poststarttests_ignoreerror 
	$(COMPARERESULTS)
	@$(ECHO) +++ Test passed +++ 

poststarttests_ignoreerror:  
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Valid cases:" >> $(RESULTFILE)
	@$(ECHO) "Testing all set options." >> $(RESULTFILE)
	@$(ECHO) "Set the current values:" >> $(RESULTFILE)
	@cimconfig -s traceLevel=1 -c  >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceLevel=2 -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceLevel=3 -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceLevel=4 -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=ALL -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=Config -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=Config,Xml -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceFilePath=/tmp/test.trace -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceFilePath=/tmp/test1.trace -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceFilePath=/tmp/test.trace -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceFileSizeKBytes=10240 -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s numberOfTraceFiles=3 -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Testing all get options." >> $(RESULTFILE)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g traceLevel >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g traceComponents >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g traceFilePath >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Get the current values:" >> $(RESULTFILE)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g  traceLevel -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g  traceComponents -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g  traceFilePath -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Get the planned values:" >> $(RESULTFILE)
	@cimconfig -g traceLevel -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -g traceComponents -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Set the planned values:" >> $(RESULTFILE)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=ALL -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=Config -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents=Config,Xml -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s traceComponents= -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Test with invalid planned values:" >> $(RESULTFILE)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s httpPort=garbage -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s enableHttpConnection=6000 -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -s enable=6000 -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Testing all unset options." >> $(RESULTFILE)
	@$(ECHO) "Unset the current values: " >> $(RESULTFILE)
	@cimconfig -u traceLevel -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig -u traceComponents -c >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Unset the planned values:" >> $(RESULTFILE)
	@cimconfig -u traceComponents -p >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Testing -H option." >> $(RESULTFILE)
	@$(ECHO) "Testing -H option with logLevel." >> $(RESULTFILE)
	@cimconfig -H logLevel >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@$(ECHO) "Testing all LONG options." >> $(RESULTFILE)
	@cimconfig --help >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)
	@cimconfig --version >> $(RESULTFILE) $(REDIRECTERROR)
	@$(ECHO) " " >> $(RESULTFILE)

clean:

depend:
