#
# GAWK Program to extract all of the @name entities from
# the document and create a table of contents page.
# This file analyzes all of the files pointed to by the top
# level file in the document structure.
#
BEGIN {
level = 0
count = 0
}
# Big limit here in that it is all not tied to the
# inside comments concept.
# Presumes one layer of includes only
{     if (NF == 2 && $1 == "\/\/@Include:") {
	  incfile = $2
          while ((getline < incfile) > 0)
               {
               if ($2 == "@name")
               		{               			
                        fil[count]= incfile
                        lvl[count] = level
                        nme[count] = compressstring(3)
                        flnme[count++] = Svstr(3)
                        }
               if (match ($1, "@name") > 0)
               		{
                        fil[count]= incfile
                        lvl[count] = level
                        nme[count] = compressstring(2)
                        flnme[count++] = Svstr(2)
               		}
               
               if ($1 == "\/\/@{")
               		level++
               if ($1 == "\/\/@}")
               		level--
               }
          close($2)
     } else
          {
               if ($2 == "@name")
               		{
               		fil[count]= FILENAME
			lvl[count] = level
                        nme[count] = compressstring(3)
                        flnme[count++] = Svstr(3)
               		}
               if (match ($1, "@name") > 0)
               		{
               		fil[count]= FILENAME
			lvl[count] = level
                        nme[count] = compressstring(2)
                        flnme[count++] = Svstr(2)
               		}               
               if ($1 == "\/\/@{")
               		level++
               if ($1 == "\/\/@}")
               		level--
          }
}

END{
indent = 0
is = ""
lv = 0
# Print the TOC Header
# print "<html>"
#print "<head>"
#print "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\">"
#print "<title>PegasusDocumentTOC</title>"
#print "<base target=\"main\">"
#print "</head>"
#print "<body bgcolor=\"#FF9966\">"
print "<p align=\"center\"><img border=\"0\" src=\"/pegasus/images/pegicon2.gif\" width=\"100\" height=\"100\"></p>"
print "<p>&nbsp;</p>"
print "<p><b><font size=\"5\">Contents</font></b></p>"
print "<TABLE>"

# The array contains Filename, lvl, section name
#<TR><TD VALIGN=TOP>
# <IMG ALT="o" BORDER=0 SRC=icon1.gif><A NAME="Introduction"></A>
# <A HREF=Introduction.html><B>Introduction</B></A></TD><TD><BR>
# </TD></TR>

# Print an output for each line that represents the TOC HTML for that line
  for (x = 0; x <= count; x++)
  	{
  	print
  	print "<!--Diagnostic " x " " lvl[x] " " nme[x] "-->"
  	if (lvl[x] != 0){
  		ft = "<FONT SIZE=\"-"lvl[x] "\"> "
  		nft = "</FONT>"
  		}
  	else {
  		ft = ""
  		nft = ""
  		}
  	print "<TR><TD VALIGN=TOP>"
  	print ist(lvl[x])"<IMG ALT=\"o\" BORDER=0 SRC=icon1.gif>" " <A NAME=" nme[x] "></A>"
  	print "<A HREF=" nme[x] ".html><B>"  ft flnme[x] nft "</B></A></TD><TD><BR>"
  	print "</TD></TR>"
  	print	
  	}
   

# Print the closing section

print "</table>"
print "<UL>"
print "<LI><I><A HREF=\"index.html\">Alphabetic index</A></I>"
#print "<LI><I><A HREF=\"toc.html\">Alphabetic index</A></I>"
print "<LI><I><A HREF=\"HIER.html\">Hierarchy of classes</A></I>"
print "</UL>"
# print "</body>"
# print "</html>"
}
#
# functions
#
# Compress the $0 string, taking out all spaces from the pos defined
# returns the result With quotes around it.
function compressstring(pos){ 
	out = ""
	for (i = pos; i <= NF; i++)
		out = out $i
	return out
}
# Simply duplicate the remainder of the $0 string and return it
function Svstr(pos){
	out = ""
	for (i = pos; i <= NF; i++)
		out = out " " $i
	return out
}
# create an indent string with length corresponding to the
#indent count.  0 = zero length, 1, 3 char, etc.
# uses the &nbsp; to indent for the moment
function ist (indent){
	isstring = ""
	if (indent == 0)
		return ""
	for (i=1; i <= indent; i++)
		isstring = isstring "&nbsp; &nbsp;"
	return isstring
}