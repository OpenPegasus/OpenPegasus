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

// Author: Humberto Rivero (hurivero@us.ibm.com)

//

// Modified By:

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

#include <Pegasus/Common/AcceptLanguages.h>

#include <Pegasus/Common/ContentLanguages.h>



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


/**

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

	 * AcceptLanguages acceptlanguages: This contains the languages that are acceptable by the caller

	 * of MessageLoader::getMessage(). That is, MessageLoader will do its best to return a message in 

	 * a language that was specified in this container.  This container is naturally ordered using the quality 

	 * values attached to the languages and MessageLoader iterates through this container in its natural 

	 * ordering.  This container is used by MessageLoader to load messages if it is not empty.

	 */

	AcceptLanguages acceptlanguages;

	

	/*

	 * ContentLanguages contentlanguages: This is set by MessageLoader::getMessage() after a message has

	 * been loaded from either a message resource or the default message.  After the call to MessageLoader::getMessage()

	 * the caller can check the MessageLoaderParms.contentlanguages object to see what MessageLoader set it to.

	 * In all cases where a message is returned from MessageLoader::getMessage(), this field will be set to match the

	 * language that the message was found in.

	 */

	ContentLanguages contentlanguages;

	

	/*

	 * Boolean useProcessLocale: Default is false, if true, MessageLoader uses the system default language

	 * to loads messages from.

	 */

	Boolean useProcessLocale;

	

	/*

	 * Boolean useThreadLocale: Default is true, this tells MessageLoader to use the AcceptLanguages container

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

	 * Formatter::Arg0-9: These are assigned the various substitutions necessary to properly format

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

	

	/**

	 * Constructor:

	 */

	MessageLoaderParms(){

		useProcessLocale = false;

		useThreadLocale = true;

		

		#ifdef PEGASUS_HAS_ICU

		useICUfallback = false;

		#endif

		

		acceptlanguages = AcceptLanguages();

		contentlanguages = ContentLanguages();

		msg_src_path = String();

	}

	

	/*

	 * Constructor: 

	 * @param id String - message identifier used to look up a message in a message resource

	 * @param msg String - default message to use if a message cannot be found in the message resources.

	 * @param arg0 Formatter::Arg - optional substitution parameter

	 * @param arg1 Formatter::Arg - optional substitution parameter

	 * @param arg2 Formatter::Arg - optional substitution parameter

	 * @param arg3 Formatter::Arg - optional substitution parameter

	 * @param arg4 Formatter::Arg - optional substitution parameter

	 * @param arg5 Formatter::Arg - optional substitution parameter

	 * @param arg6 Formatter::Arg - optional substitution parameter

	 * @param arg7 Formatter::Arg - optional substitution parameter

	 * @param arg8 Formatter::Arg - optional substitution parameter

	 * @param arg9 Formatter::Arg - optional substitution parameter

	 */

	MessageLoaderParms( String id, String msg, 

						Formatter::Arg anArg0 = Formatter::Arg(), 

						Formatter::Arg anArg1 = Formatter::Arg(),

						Formatter::Arg anArg2 = Formatter::Arg(),

						Formatter::Arg anArg3 = Formatter::Arg(),

						Formatter::Arg anArg4 = Formatter::Arg(),

						Formatter::Arg anArg5 = Formatter::Arg(),

						Formatter::Arg anArg6 = Formatter::Arg(),

						Formatter::Arg anArg7 = Formatter::Arg(),

						Formatter::Arg anArg8 = Formatter::Arg(),

						Formatter::Arg anArg9 = Formatter::Arg() ){

							

		msg_id = id;

		default_msg = msg;

		useProcessLocale = false;

		useThreadLocale = true;

		

		#ifdef PEGASUS_HAS_ICU

		useICUfallback = false;

		#endif

		

		acceptlanguages = AcceptLanguages::EMPTY;

		contentlanguages = ContentLanguages::EMPTY;

		msg_src_path = String::EMPTY;

		this->arg0 = anArg0;

		this->arg1 = anArg1;

		this->arg2 = anArg2;

		this->arg3 = anArg3;

		this->arg4 = anArg4;

		this->arg5 = anArg5;

		this->arg6 = anArg6;

		this->arg7 = anArg7;

		this->arg8 = anArg8;

		this->arg9 = anArg9;

	}	

	

	String toString(){

		

		String s;

		String processLoc,threadLoc,ICUfallback;

		processLoc = (useProcessLocale) ? "true" : "false";

		threadLoc = (useThreadLocale) ? "true" : "false";

		#ifdef PEGASUS_HAS_ICU

		ICUfallback = (useICUfallback) ? "true" : "false";

		#endif

		

		s.append("msg_id = " + msg_id + "\n");

		s.append("default_msg = " + default_msg + "\n");

		s.append("msg_src_path = " + msg_src_path + "\n");

		s.append("acceptlanguages = " + acceptlanguages.toString() + "\n");

		s.append("contentlanguages = " + contentlanguages.toString() + "\n");

		

		s.append("useProcessLocale = " + processLoc + "\n");

		s.append("useThreadLocale = " + threadLoc + "\n");

		#ifdef PEGASUS_HAS_ICU

		s.append("useICUfallback = " + ICUfallback + "\n");

		#endif

		s.append("arg0 = " + arg0.toString() + "\n" + "arg1 = " + arg1.toString() + "\n" + "arg2 = " + arg2.toString() + "\n" + "arg3 = " + arg3.toString() + "\n" + 

		          "arg4 = " + arg4.toString() + "\n" + "arg5 = " + arg5.toString() + "\n" + "arg6 = " + arg6.toString() + "\n" + "arg7 = " + arg7.toString() + "\n" + 

		          "arg8 = " + arg8.toString() + "\n" + "arg9 = " + arg9.toString() + "\n\n");

		          

		return s;

	}	

		

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

	

	static Boolean _useProcessLocale;

	

	static Boolean _useDefaultMsg;

 	

	static AcceptLanguages _acceptlanguages;

	

private:



	static String formatDefaultMessage(MessageLoaderParms &parms);

	

	static String getQualifiedMsgPath(String path);



	static void initPegasusMsgHome();

	

	static void checkDefaultMsgLoading();

	

	static String pegasus_MSG_HOME;

	

	#ifdef PEGASUS_HAS_ICU

		static String loadICUMessage(MessageLoaderParms &parms);

		

		static String extractICUMessage(UResourceBundle * resbundl, MessageLoaderParms &parms);

		

		static String formatICUMessage(UResourceBundle * resbundl, const UChar *msg, int msg_len, MessageLoaderParms &parms);

		

		static String uChar2String(UChar * uchar_str);

		

		static String uChar2String(UChar * uchar_str, int len);

		

		static UChar* string2UChar(String s);

		

		static void xferFormattables(MessageLoaderParms &parms, Formattable *formattables);

	#endif

	

}; // end MessageLoader



PEGASUS_NAMESPACE_END


#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES


#endif

