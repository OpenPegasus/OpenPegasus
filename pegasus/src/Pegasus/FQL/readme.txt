FQL Query Language

10 November 2014

NOTE: The FQL query language implementation is considered experimental
in OpenPegasus and subject to non-compatible change.  Further, it 
is included in the source code with the option to not build it into
the server.  Please note that there are some limitations with respect
to the DMTF specification in this version of the implementation.

FQL is a query parser language consistent with the DMTF specification
DSP0101, version 1.01

This parser implements the features of the language as defined in the
specification except:

1. RegularExpression only implements a simple regex similar to CQL
   basic regex. It only implements the "." and "*" special characters from
   the regex specification.
    * matches zero or more instances of the proceeding single character.
    . matches any single character
    \ Escape the next character (next must be must be *, . or \)
    \\ The backslash character
2. Does not implement the comparison of embedded instance properties
3. Not internationalized, in particular, the exceptions are not 
   internationalized.
4. Does not handle the utf8[] CIM property type in accord with the spec.
   It is supposed to be compared to a String per the spec.
5. Does not handle the String escape character definition for comparisons

REGEX Specification
The regex that we will implement is based on a DMTF defined subset of the
Unit regex. The definition is in the DMTF Specification DSP1001 and it also
documented in the file FQLRegularExpression.cpp

The following elements are defined:
Special characters
    SpecialChar = "." / "\" / "[" / "]" / "^" / "$" / "*" / "+" / "?" /
        "/" / "|"
where
    "." matches any single character
    "\" escapes the next character so that it isn't a SpecialChar
    "[" starts a CharacterChoice
    "]" ends a CharacterChoice
    "^" indicates a LeftAnchor
    "$" indicates a RightAnchor
    "*" indicates that the preceding item is matched zero or more times.
    "+" indicates that the preceding item will be matched one or more times.
    "?" indicates that the preceding item is optional,
        and will be matched at most once.
    "|" separates choices

OrdinaryChar = UnicodeChar, except SpecialChar

EscapedChar = "\" SpecialChar

Simple Character

    SimpleChar = OrdinaryChar / EscapedChar

Character Sequence
    CharacterSequence = SimpleChar [ CharacterSequence ]

    A CharacterSequence is a sequence of SimpleChars, for example:
        "ABC" matching "ABC", or
        "D.F" matching "DAF", "DBF", "DCF", and so forth.

Character Choice
    CharacterChoice = "[" CharacterSequence "]" [ "^" ]
    A CharacterChoice defines a set of possible characters. It is indicated by
    square brackets ("[" and "]") enclosing the set of characters.

    -If a caret ("^") is not suffixed after the closing bracket, any character
     from the set matches. For example, "r[au]t" matches "rat" or "rut".

    -If a caret ("^") is suffixed after the closing bracket, any character not
      in the set matches.

    For example, "r[au]^t" matches any three-character sequence with the middle
    character not being "a" or "u", for example, "ret" or "r.t".

Single Character
    SingleChar = "." / SimpleChar / CharacterChoice
    For example,
        "D.F" matching "DAF", "DBF", "DCF", and so forth, or
        "GH[IJ]" matching "GHI" or "GHJ".

Multipliers
    Multiplier = "*" / "+" / "?" / "{" UnsignedInt ["," [UnsignedInt]] "}"
    where:
    "*" indicates that the preceding item is matched zero or more times
    "?" indicates that the preceding item is matched zero or one time
        (optional item)
    "+" indicates that the preceding item is matched one or more times
UnsignedInt is an unsigned integer number

Multiplied character
    MultipliedChar = SingleChar [ Multiplier ]
    A MultipliedChar is a SingleChar with a Multiplier applying, for example:
        "C*" matching "", "C", "CC", "CCC", and so forth, or
        "[EF]{1,2}" matching "E", "F", "EE", "EF", "FE" or "FF"

Character expression
    CharacterExpression = MultipliedChar [ CharacterExpression ]
    A CharacterExpression is a descriptor for a sequence of one or more
    characters. For example:
        "X" matching "X" only,
        "ABC" matching "ABC" only,
        "ABC*" matching "AB", "ABC", "ABCC", "ABCCC", and so forth,
        "A[BC]D" matching "ABD" or "ACD", or
        "1[.]{2,3}n" matching "1..n" or "1...n".

Grouping
    Grouping = "(" CharacterExpression ")" [ Multiplier ]
    A Grouping is a CharacterExpression that optionally can be multiplied, for
    example:
        "(ABC)" matching "ABC",
        "(XYZ)+" matching "XYZ", "XYZXYZ", "XYZXYZXYZ", and so forth.

ChoiceElement
    ChoiceElement = Grouping / CharacterExpression

Choice
    Choice = ChoiceElement [ "|" Choice ]
     A Choice is a choice from one or more ChoiceElements, for example:
        "(DEF)?" matching "" or "DEF",
        "GHI" matching "GHI", or
        "(DEF)?|GHI" matching "", "DEF", or "GHI".

Left anchor
    LeftAnchor = "^"
    A LeftAnchor forces a match at the beginning of a string.

Right anchor
    RightAnchor = "$"
    A RightAnchor forces a match at the end of a string.

AnchoredExpression
    AnchoredExpression = [ LeftAnchor ] Choice [ RightAnchor ]
    An AnchoredExpression is a Choice that is optionally anchored to the left
    end, to the right end, or to both ends of a string.

AnchoredChoice
    AnchoredChoice = AnchoredExpression [ AnchoredChoice ]
    An AnchoredChoice is a choice from one or more AnchoredExpressions.

RegularExpressionInProfile
    RegularExpression = AnchoredChoice
    A regular expression  is an AnchoredChoice.

TESTING
There are several testprograms for FQL

  - Parser - This is the unit test with a fairly complete set of tests
    of different queries includeing both success and failures as well
    as queries that generate exceptions
  - clitest - end-end test of a limited set of queries to confirm that
    the end-end functionality works. This includes the pull operations
       - openEnumerateInstances
       - openAssociatorInstances
       - openReferenceInstances
   - RegularExpresssion - Test the regular expression functionality

NOTE: The diagnostics throught the FQL code (primarily DCOUT statements)
can be conditionally compiled by setting the FQL_DOTRACE in the
Makefile.


Note that there are no end-end tests for the names operations
(ex. openEnumerateInstanceNames) because today OpenPegasus does not allow
the query argument on those commands.  They are refused by the server

FUTURE (post Pegasus 2.14):

1. Bring the regex for CQL and this one into line possibly. However, since
   this one must grow to handle the full defined subset, we will probably
   override the CQL one with this regex
2. Optimize the evaluation loop.
3. Add functionality to move the evaluator to the provider agents for
   OOP providers so that the filtering is done before the data is passed
   from the agent to the server.
4. Add an API to the c++ providers to process filters.
5. Add the CMPI 2.1 functionality including the new functions to process
   filters.
6. A number of cout type diagnostics remain.  We should be able to remove
   in favor of trace in the future


