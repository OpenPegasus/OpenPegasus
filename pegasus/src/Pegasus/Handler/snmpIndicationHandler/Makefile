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

DIR = Pegasus/Handler/snmpIndicationHandler

include $(ROOT)/mak/config.mak

ifeq ($(OS_TYPE),vms)
 VMS_VECTOR = PegasusCreateHandler
endif

LOCAL_DEFINES = -DPEGASUS_HANDLER_INTERNAL -DPEGASUS_INTERNALONLY

LIBRARIES = \
    peggetoopt \
    pegrepository \
    pegclient \
    pegconfig \
    pegcommon

SOURCES = snmpIndicationHandler.cpp

ifeq ($(PEGASUS_USE_EMANATE), true)
    ifdef PEGASUS_EMANATE_LIB_DIR
        EXTRA_LIBRARIES += -L$(PEGASUS_EMANATE_LIB_DIR)
    endif 
    EXTRA_LIBRARIES += -lsubagent -lpack -lagent -lemanate \
            -lmibtable -lsnmp

    ifdef PEGASUS_EMANATE_INCLUDE_DIR
        EXTRA_INCLUDES += -I$(PEGASUS_EMANATE_INCLUDE_DIR) \
                -I$(PEGASUS_EMANATE_INCLUDE_DIR)/snmpd 
    endif
    EXTRA_INCLUDES += $(SYS_INCLUDES) $(ACE_INCLUDES)

    LOCAL_DEFINES += -DSETS -DSR_SNMPv1 -DSR_SNMPv2c -DSR_SNMPv2 \
            -DEMANATE -DSR_ANSI_PROTOTYPES -DSUBAGENT -DSR_USER_AP_SERVICE \
            -DSR_UDS_IPC -DSR_MIB_TABLE_HASHING -DSR_IP

    SOURCES += snmpDeliverTrap_emanate.cpp
else
    # Controls snmp indication handler to use NET-SNMP to deliver trap
    ifeq ($(PEGASUS_USE_NET_SNMP), true)
        ifdef PEGASUS_NET_SNMP_LIB_DIR
            EXTRA_LIBRARIES += -L$(PEGASUS_NET_SNMP_LIB_DIR)
        endif
        EXTRA_LIBRARIES += -lnetsnmp

        ifdef PEGASUS_NET_SNMP_INCLUDE_DIR
            EXTRA_INCLUDES = -I$(PEGASUS_NET_SNMP_INCLUDE_DIR)
        endif

        SOURCES += snmpDeliverTrap_netsnmp.cpp
    else
        $(error no PEGASUS SNMP option is set)
    endif
endif

LIBRARY = snmpIndicationHandler
 
include $(ROOT)/mak/dynamic-library.mak

tests:
