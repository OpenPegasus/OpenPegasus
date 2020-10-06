##
##  This is a developer awk script to scrape the internationalization strings
##  from the source code and produce a file that can be substitutue
##  into the Pegasus client message bundle. 

## It is a convience for building message bundles for a particular directory
## in that it allows putting the message bundle text for the message in
## the source file and mechanically generating the message bundle.

## NOTE: The created message bundle MUST BE manually transfered to the correct
## place in the OpenPegasus message bundle.

##  The form it expects in the source file for each internationalized message
##  is:
##  line prefix //# for each line that defines a component of a message.
##  The line types are
##  N - Note There may be multiple note lines
##  T - Name of this message
##  S - String defining the message.  There may be multiple string lines
##
##  Example:
## //#N substitution {0} is a string with the property
## //#N name in error
## //#T BOOLEAN_VALUE_ERR
## //#S Invalid keyword for boolean value = {0}. Must be \"true\" or \"false\"
## //#S upper or lower case\n


## Maps each entry to the form 
##	/**
##	* @note PGC01619: substitution {0} is a string with the keyword
##	* in error
## 	*/
##       
##	Clients.cimcli.CIMCLIClient.BOOLEAN_VALUE_ERR:string {
##  "Invalid keyword for boolean value = {0}. Must be \"true\" or \"false\""
##	" upper or lower case\n"}

##  Notes are included only if the //#N tag exists

##  The //#T tag MUST exist and must be unique. It contains a single
##  Non-blank string that is the name component of this message.
##  This is mapped to the name component of the Full message name

##  FileText.name where name is the only parameter on the line
##  at least one //#S tag must exist. Do not put quotes around the text
##  Multiple lines may be supplied.  They will be output as a single line
##  within the context of the message string {"<text>"}

BEGIN {
    ## These are the cimcli message bundle numbers.  We are not using them
    ## at this point because all output is to the local console in response
    ## to console input.
    MsgNumStr = "PGC016"
    MsgNum = 1

    ## Common part of the name of the source file group
    FileText = "Clients.cimcli.CIMCLIClient."
    
    ## name of the output file that will contain the messages
    outputFile = "cimcliCLI_en.txt"
    ## local variables
    arrayIndex = 0
    txtString = ""
    errorCount = 0
    messageNumber = ""

    print "Output msg file " outputFile " will be generated."
    # recreate the output message file
    print "        /*CIMCLIAUTOMEDMESSAGEBUNDLE_BEGIN" > outputFile
    print "        * MessageBundleInput automatically generated "  \
        strftime("%Y-%m-%d") >> outputFile
    print "        * by cimcli buildMsgStrings.awk." >> outputFile
    print "        */\n" >> outputFile
    exitCode = 0
}

# line parsing from all input files

## Parse the first line of the text info, the NOTE info. This line is
## optional
$1 == "//#N" {
    ##print "N record " $0 " $1=" $1
    messageNumber = ""
    if (state == 0)
    {
        $1 = ""
        noteArray[arrayIndex++] = $0
        state = 1
        next
    }
    else if (state == 1)
    {
        $1 = ""
        noteArray[arrayIndex++] = $0
        next
    }
    else
    {
    PrintError("N record found out of sequence")
    }
}

## Used to set message numbers on specific messages. Set the message number
## text into the messageNumber variable to be output on the @note: line
## For now we DO NOT output this with the messages themselves.
$1 == "//#P" {
   if (state != 1)
   {
       PrintError("Invalid use of P record.")
   }
   else
   {
       PrecordNumber = $2
       if ($2 in numbersArray)
       {
            ermsg = "The msg number " $2 " is already defined at " nameArray[$2]
            PrintError(ermsg)
       }
       else
       {
           numbersArray[$2] = FILENAME":"FNR
       }
       messageNumber = "PGC016"$2
       next
   }
}
## This processes the message name line.  It contains a single string
## the name of this message in the msg file.  Must be all caps. This
## may be the first line in a description. This record must exist so it
## increments the state variable.
$1 == "//#T" {
    ##print "T record " $0 " $2" $2
    if ($2 == "")
    {
        PrintError("Invalid name for this msg txt.  Must be single capped wrd")
        next
    }
    if (state == 1 || state == 0)
    {
        if ($2 in nameArray)
        {
            ermsg = "The msg name " $2 " is already defined at " nameArray[$2]
            PrintError(ermsg)
            next
        }
        else
        {
            nameArray[$2] = FILENAME":"FNR
        }
        state = 2
        MSGName = FileText  $2 ":string {\""
        next
    }
    else
    {
        PrintError("Unexpected T record. Must be first or follow N record")
        next
    }
}

## Process the String text line.  There may be multiple lines of text to
## allow the source file to maintain the 80 col length.  They are
## assembled into a single string
$1 == "//#S" {
    ##print "S record" $0 " state=" state " txtString=" txtString  ##remove
    ## remove the //#S from the line
    $1 = ""
    ## remove leading blanks and the ..#S sequence
    ##print "before " $0
    gsub(/^[ ]+/,"",$0)
    ##print "afterx " $0

    if (state == 2)
    {
        state = 3

        where = match($0, /\$[0-9]/)
        if (where != 0)
        {
            PrintError("Internal form of variable (ex. $0 found")
        }

        txtString = $0

        where = match($0, /\$[0-9]/)
        ##print "match result " where
        if (where != 0)
        {
            PrintError("Internal form of variable (ex. $0 found")
            fixInput($0)
        }

        next
    }
    else if (state == 3)
    {
        if (match($0, /\$[0-9]/))
        {
            PrintError("Internal form of variable (ex. $1 found)" $0)
            fixInput($0)
        }
        ##else
        ##{
        ##    print " Match on following record failed. " $0
        ##}
        txtString = txtString " " $0
        ##print "txtString appended" txtString  ##remove
        next
    }
    else
    {
        PrintError("Unexpected N or T record. Expected S record.")
        next
    }
}

## process lines that do not have the //# prefix.  If state != 0
## this is first record after a msg definition and we output the
## result.

## if state == 1, there was no string definition
((state == 1) || (state == 2)) {
    PrintError("T record must preceed S record.")
    state = 0
    next
}
## state = 3. Valid message definition complete. Output the result
(state == 3) {
    ##print "State 3 found"  ##remove
    state = 0;
    if (arrayIndex != 0)
    {
    	print "        /**"  >> outputFile
        if (messageNumber != "")
        {
            print "        * @note "messageNumber":" >> outputFile
        }
        else
        {

            print "        * @note:" >> outputFile
        }
    	for (i = 0; i < arrayIndex; i++)
    	{
    	    print "        *     " noteArray[i]  >> outputFile
    	}

        print "        */" >> outputFile
        print "" >> outputFile

        arrayIndex = 0
    }
    # any note output, proceed to output the name and text line
    ## Note that we have to append the leading and trailing quotes
    ##printf("\"%s\"\n", txtString)
    if (txtString in strTextArray)
    {
        errmsg = "Duplicated text of messages " txtString " at " strTextArray[txtString]
        PrintError(errmsg)
    }
    else
    {
        strTextArray[txtString] = FILENAME":"FNR
    }
    printf("        %s%s\"}\n", MSGName, txtString) >> outputFile
    print ""  >> outputFile   ## blank line after each message
    count++
}

END {
    print Message File Generation Complete
    if (errorCount != 0)
    {
        print "ERRORS Found. " errorCount " errors found."
    }
    print "Generated file" outputFile " with " count " entries."
    if (errorCount != 0)
    {
        print "        /*" > outputFile
        print "        * cimcli MessageBundle End with " errorCount " errors."  >> outputFile
        print "        */" >> outputFile
    }
    else
    {
        print "        /*" > outputFile
        print "        * cimcli MessageBundle End"  >> outputFile
        print "        * Please insert into msg/CLI/pegasusCLI_en.txt."
        print "        CIMCLIAUTOMEDMESSAGEBUNDLE_END*/" >> outputFile
    }
    if (exitCode != 0)
    {
        exit exitCode
        print "Exiting with error code " exitCode
    }
}

## fix the input string to replace $x with {x} for all x between 0 and 9
function fixInput(textLine)
{
    ##print "Fixing " textLine
    temp = textLine
    if (match($0, /\$[0-9]/))
    {
        for (x = 0; x <= 9; x++)
        {
            replacement = "{"x"}"
            regex = "\\$"x
            ##print "fixInput. regex " regex " replacement " replacement
            where = match(textLine, regex)
            ##print "where " where
            if (where != 0)
            {
                sub(regex, replacement, textLine)
                ##print "fixed for replacement " replacement " textLine" textLine
            }
            else
            {
                ##print "Could not fix " textLine " with replacement " replacement " using regex " regex
                break
            }
        }
    }
    ##print "new  "  temp
    ##print "orig "  textLine
}

function PrintError(errmsgText)
{

   print "ERROR: "FILENAME":"FNR " " errmsgText
   errorCount++
   exitCode = 1
}


