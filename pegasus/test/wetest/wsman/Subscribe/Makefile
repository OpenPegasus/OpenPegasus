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
include $(ROOT)/mak/config.mak
include $(ROOT)/test/config.mak

STRIPLINES = "CIM_ERR" "quot;"

MULINES = "strip" "Content" "content"

DIFFLINE1 = "content"
DIFFLINE2 = "CIM_ERR"

XMLSCRIPTS = \
    Successful_Subscription01 \
    Successful_Subscription02 \
	Unsuccessful_Renew01 \
	InvalidMessageID_Subscription02 \
	InvalidExpires_Subscription01 \
	InvalidExpires_Subscription02 \
	InvalidFilter_Subscription \
	InvalidNamespace_Subscription \
	InvalidDeliveryMode_Subscription \
	InvalidFilterDialect_Subscription \
	InvalidReplyTo_subscription \
    MutipleFilter_Subscription \
    CannotProcessFilter_Subscription \
    UnsupportedBookmarks_Subscription \
    UnsupportedHeartbeats_Subscription \
    Invalid_subscription_allcalsses \
    Subscription_WithOutFilterQuery \
    Subscription_WithoutExpiry \
    Subscription_withConnectionRetry \
    Subscription_withExistingFilter \
    Subscription_with_diefferentclasses_RU_FilterQuery \
    Successful_subscription_allcalsses \
    Successful_unsubscribe_withexistingfilter \
    Successful_Unsubscription_allcalsses \
    Successful_unsubscribe_withoutExpiry \
    Unsubscribe_WithOutFilterQuery \
    invalidFilterNameInBody_Subscription \
    Successful_subscription_pushWithAck \
    Successful_unsubscribe_pushWithAck

XMLSCRIPTS_STRIPPED_MU = \
	InvalidMessageID_Subscription01 \
	Successful_UnSubscribe01 \
	Successful_UnSubscribe02 \
	Unsuccessful_Unsubscribe \
    invalidFilterNameinHeader_Subscription 


include $(ROOT)/test/configend.mak
