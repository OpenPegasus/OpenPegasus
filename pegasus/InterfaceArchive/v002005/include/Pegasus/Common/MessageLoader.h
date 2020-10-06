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

#ifndef Pegasus_MessageLoader_h
#define Pegasus_MessageLoader_h

#include <cstdlib>
#include <cctype>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Formatter.h>
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>

//ICU specific 
#ifdef PEGASUS_HAS_ICU
#include <unicode/uloc.h> 
#include <unicode/ures.h>
#include <unicode/umsg.h>
#include <unicode/ucnv.h>
#include <unicode/fmtable.h>
#include <unicode/msgfmt.h>
#endif

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

PEGASUS_NAMESPACE_BEGIN

/** <I><B>Experimental Interface</B></I><BR>
 * MessageLoaderParms class is basically a stuct class containing public variables that control
 * the way MessageLoader behaves. MessageLoader uses the fields in this class to decide where and
 * how to load messages from the message resources.
 */

class PEGASUS_COMMON_LINKAGE MessageLoaderParms{
	
public:
	
	/*
	 * String msg_id: unique message identifier for a particular message in a message resource
	 */
	String msg_id;	
	
	/*
	 * String default_msg: the default message to use if a message cannot be loaded from a message resource
	 */		
	String default_msg;     
	
	/*
	 * String msg_src_path: this path tells MessageLoader where to find message resources
	 * it can be empty, fully qualified or relative to $PEGASUS_HOME
	 */
	String msg_src_path;
	
	/*
	 * AcceptLanguageList acceptlanguages: This contains the languages that are acceptable by the caller
	 * of MessageLoader::getMessage(). That is, MessageLoader will do its best to return a message in 
	 * a language that was specified in this container.  This container is naturally ordered using the quality 
	 * values attached to the languages and MessageLoader iterates through this container in its natural 
	 * ordering.  This container is used by MessageLoader to load messages if it is not empty.
	 */
	AcceptLanguageList acceptlanguages;
	
	/*
	 * ContentLanguageList contentlanguages: This is set by MessageLoader::getMessage() after a message has
	 * been loaded from either a message resource or the default message.  After the call to MessageLoader::getMessage()
	 * the caller can check the MessageLoaderParms.contentlanguages object to see what MessageLoader set it to.
	 * In all cases where a message is returned from MessageLoader::getMessage(), this field will be set to match the
	 * language that the message was found in.
	 */
	ContentLanguageList contentlanguages;
	
	/*
	 * Boolean useProcessLocale: Default is false, if true, MessageLoader uses the system default language
	 * to loads messages from.
	 */
	Boolean useProcessLocale;
	
	/*
	 * Boolean useThreadLocale: Default is true, this tells MessageLoader to use the AcceptLanguageList container
	 * from the current Pegasus thread.
	 */
	Boolean useThreadLocale;
	
	/*
	 * Boolean useICUfallback: Default is false.  Only relevant if PEGASUS_HAS_ICU is defined.
	 * MessageLoader::getMessage() default behaviour is to extract messages for the langauge exactly
	 * matching an available message resource.  If this is set to true, the MessageLoader is free to extract
	 * a message from a less specific message resource according to its search algorithm.
	 */
	#ifdef PEGASUS_HAS_ICU
	Boolean useICUfallback;
	#endif
	
	/*
	 * const Formatter::Arg&0-9: These are assigned the various substitutions necessary to properly format
	 * the message being extracted.  MessageLoader substitutes these in the correct places in the message
	 * being returned from MessageLoader::getMessage()
	 */
	Formatter::Arg arg0; 
	Formatter::Arg arg1;
	Formatter::Arg arg2;
	Formatter::Arg arg3;
	Formatter::Arg arg4;
	Formatter::Arg arg5;
	Formatter::Arg arg6;
	Formatter::Arg arg7;
	Formatter::Arg arg8;
	Formatter::Arg arg9;
	
    /** Constructor */
    MessageLoaderParms();

    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg, 
	const Formatter::Arg& arg0,
	const Formatter::Arg& arg1,
	const Formatter::Arg& arg2,
	const Formatter::Arg& arg3,
	const Formatter::Arg& arg4,
	const Formatter::Arg& arg5 = Formatter::DEFAULT_ARG,
	const Formatter::Arg& arg6 = Formatter::DEFAULT_ARG,
	const Formatter::Arg& arg7 = Formatter::DEFAULT_ARG,
	const Formatter::Arg& arg8 = Formatter::DEFAULT_ARG,
	const Formatter::Arg& arg9 = Formatter::DEFAULT_ARG);

    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg);
    
    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg, 
	const Formatter::Arg& arg0);

    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg, 
	const Formatter::Arg& arg0,
	const Formatter::Arg& arg1);

    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg, 
	const Formatter::Arg& arg0,
	const Formatter::Arg& arg1,
	const Formatter::Arg& arg2);

    /** Constructor */
    MessageLoaderParms(
	const String& id, 
	const String& msg, 
	const Formatter::Arg& arg0,
	const Formatter::Arg& arg1,
	const Formatter::Arg& arg2,
	const Formatter::Arg& arg3);

    MessageLoaderParms(
	const char* id, 
	const char* msg);

    MessageLoaderParms(
	const char* id, 
	const char* msg, 
	const String& arg0);

    MessageLoaderParms(
	const char* id, 
	const char* msg, 
	const String& arg0,
	const String& arg1);
    
    /** Converts to string. */
    String toString();

    ~MessageLoaderParms();

private:

    void _init();
}; // end MessageLoaderParms


/*
 * MessageLoader is a static class resposible for looking up messages in message resources. 
 * For specific behaviour details of this class see the Globalization HOWTO.
 */

class PEGASUS_COMMON_LINKAGE MessageLoader{
	
public:
	
	/*
	 * Retrieves a message from a message resource
	 * @param parms MessageLoaderParms - controls the behaviour of how a message is retrieved
	 * @return String - the formatted message
	 */
	static String getMessage(MessageLoaderParms &parms);
	
	static void setPegasusMsgHome(String home);

    static void setPegasusMsgHomeRelative(const String& argv0);
	
	static Boolean _useProcessLocale;
	
	static Boolean _useDefaultMsg;
 	
	static AcceptLanguageList _acceptlanguages;
	
private:

	static String formatDefaultMessage(MessageLoaderParms &parms);
	
	static String getQualifiedMsgPath(String path);

	static void initPegasusMsgHome(const String & startDir);
	
	static void checkDefaultMsgLoading();
	
	static String pegasus_MSG_HOME;
	
	#ifdef PEGASUS_HAS_ICU
		static String loadICUMessage(MessageLoaderParms &parms);
		
		static String extractICUMessage(UResourceBundle * resbundl, MessageLoaderParms &parms);
		
		static String formatICUMessage(UResourceBundle * resbundl, const UChar *msg, int msg_len, MessageLoaderParms &parms);
		
		static String uChar2String(UChar * uchar_str);
		
		static String uChar2String(UChar * uchar_str, int len);
		
        static void xferFormattable(Formatter::Arg& arg, Formattable &formattable);
	#endif
	
}; // end MessageLoader

PEGASUS_NAMESPACE_END

#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES

#endif
