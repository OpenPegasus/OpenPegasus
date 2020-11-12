Certificate-Based Authentication Testing
========================================

OpenPegasus Test Certificate Definitions
----------------------------------------

The following certificate definitions are currently implemented in
pegasus/mak/certificate\_test.mak: 10, 40, 50, 60, 70, 100, 110, 120,
240, 610, 620, 630, 670, 680, and 690.

**Column Definitions**

-   Type:  1. authority (RA, IA): root/intermediate authority
    certificates. Certificates of this type are added to the trusted
    certificate store. The certuser is optional for authority
    certificates. If no certuser is specified, the certificate may not
    be used to authenticate a user. 2. authority issued end-entity (E):
    Certificates of this type are not added to the trusted certificate
    store. The certuser is required for authority issued end-entity
    certificates. 3. Self-signed identity certificate (S): Certificates
    of this type are not added to the trusted certificate store. The
    certuser is required for self-signed identity certificates.

  --------------------------------------------------------------------------------------------------------------------------------------------
  ID    Name                                            Type   Parent Cert   Status    In Truststore?   User Name
  ----- ----------------------------------------------- ------ ------------- --------- ---------------- --------------------------------------
  10    testSCertNotInTrustStoreNoUser                  S      N/A           Valid     No               None

  20    testSCertNotInTrustStoreValidUser               S      N/A           Valid     No               \$(PEGASUS\_TEST\_USER\_ID)

  30    testSCertNotInTrustStoreInvalidUser             S      N/A           Valid     No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  40    testSCertInTrustStoreNoUser                     S      N/A           Valid     Yes              None

  50    testSCertInTrustStoreValidUser                  S      N/A           Valid     Yes              \$(PEGASUS\_TEST\_USER\_ID)

  60    testSCertInTrustStoreInvalidUser                S      N/A           Valid     Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  70    testExpiredSCertNotInTrustStoreNoUser           S      N/A           Invalid   No               None

  80    testExpiredSCertNotInTrustStoreValidUser        S      N/A           Invalid   No               \$(PEGASUS\_TEST\_USER\_ID)

  90    testExpiredSCertNotInTrustStoreInvalidUser      S      N/A           Invalid   No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  100   testExpiredSCertInTrustStoreNoUser              S      N/A           Invalid   Yes              None

  110   testExpiredSCertInTrustStoreValidUser           S      N/A           Invalid   Yes              \$(PEGASUS\_TEST\_USER\_ID)

  120   testExpiredSCertInTrustStoreInvalidUser         S      N/A           Invalid   Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

                                                                                                         

  210   testRACertNotInTrustStoreNoUser                 RA     N/A           Valid     No               None

  220   testRACertNotInTrustStoreValidUser              RA     N/A           Valid     No               \$(PEGASUS\_TEST\_USER\_ID)

  230   testRACertNotInTrustStoreInvalidUser            RA     N/A           Valid     No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  240   testRACertInTrustStoreNoUser                    RA     N/A           Valid     Yes              None

  250   testRACertInTrustStoreValidUser                 RA     N/A           Valid     Yes              \$(PEGASUS\_TEST\_USER\_ID)

  260   testRACertInTrustStoreInvalidUser               RA     N/A           Valid     Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  270   testExpiredRACertNotInTrustStoreNoUser          RA     N/A           Invalid   No               None

  280   testExpiredRACertNotInTrustStoreValidUser       RA     N/A           Invalid   No               \$(PEGASUS\_TEST\_USER\_ID)

  290   testExpiredRACertNotInTrustStoreInvalidUser     RA     N/A           Invalid   No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  300   testExpiredRACertInTrustStoreNoUser             RA     N/A           Invalid   Yes              None

  310   testExpiredRACertInTrustStoreValidUser          RA     N/A           Invalid   Yes              \$(PEGASUS\_TEST\_USER\_ID)

  320   testExpiredRACertInTrustStoreInvalidUser        RA     N/A           Invalid   Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

                                                                                                         

  410   testIACertNotInTrustStoreNoUser                 IA                   Valid     No               None

  420   testIACertNotInTrustStoreValidUser              IA                   Valid     No               \$(PEGASUS\_TEST\_USER\_ID)

  430   testIACertNotInTrustStoreInvalidUser            IA                   Valid     No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID

  440   testIACertInTrustStoreNoUser                    IA                   Valid     Yes              None

  450   testIACertInTrustStoreValidUser                 IA                   Valid     Yes              \$(PEGASUS\_TEST\_USER\_ID)

  460   testIACertInTrustStoreInvalidUser               IA                   Valid     Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

  500   testExpiredIACertInTrustStoreNoUser             IA                   Invalid   Yes              None

  510   testExpiredIACertInTrustStoreValidUser          IA                   Invalid   Yes              \$(PEGASUS\_TEST\_USER\_ID)

  520   testExpiredIACertInTrustStoreInvalidUser        IA                   Invalid   Yes              \$(PEGASUS\_TEST\_INVALID\_USER\_ID)

                                                                                                         

  610   testECertNotInTrustStoreNoUser\_\               E      240           Valid     No               None
        testRACertInTrustStoreNoUser                                                                    

  620   testECertNotInTrustStoreValidUser\_\            E      240           Valid     No               \$(PEGASUS\_TEST\_USER\_ID)
        testRACertInTrustStoreValidUser                                                                 

  630   testECertNotInTrustStoreInvalidUser\_\          E      240           Valid     No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)
        testRACertInTrustStoreInvalidUser                                                               

  670   testExpiredECertNotInTrustStoreNoUser\_\        E      240           Invalid   No               None
        testRACertInTrustStoreNoUser                                                                    

  680   testExpiredECertNotInTrustStoreValidUser\_\     E      240           Invalid   No               \$(PEGASUS\_TEST\_USER\_ID)
        testRACertInTrustStoreNoUser                                                                    

  690   testExpiredECertNotInTrustStoreInvalidUser\_\   E      240           Invalid   No               \$(PEGASUS\_TEST\_INVALID\_USER\_ID)
        testRACertInTrustStoreNoUser                                                                    

                                                                                                         
  --------------------------------------------------------------------------------------------------------------------------------------------

Configuration 1
---------------

In this configuration, the CIM Server DOES NOT request a Client
Certificate from the CIM Client. The Username/Password is used by the
CIM Server for authentication.

### Test Setup

  Option                                               Value
  ---------------------------------------------------- ----------
  enableHttpsConnection                                true
  sslClientVerificationMode                            disabled
  PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT   NA

### Expected Test Results

Note: A "Yes" in the "Certificate Send?" column indicates that the
client will send the certificate if requested by the CIM Server. With
Configuration 1, we do not expect the CIM Server to request the
certificate.

ID

Certificate

Username/Password (U/P)

Result

CID

Type

Sent?

Valid?

In\
Truststore?

Certificate\
Username\
Valid?

Sent?

U/P Valid?

Authenticated?

Authenticated User\
or\
Generated Exception

1

NA

NA

No

NA

NA

NA

No

NA

No

CIMClientHTTPErrorException(401)

2

NA

NA

No

NA

NA

NA

Yes

No

No

CIMClientHTTPErrorException(401)

3

NA

NA

No

NA

NA

NA

Yes

Yes

Yes

U/P User

 

 

 

 

 

 

 

 

 

 

 

4

10

S

Yes

Yes

No

NA

No

NA

No

CIMClientHTTPErrorException(401)

5

10

S

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

6

10

S

Yes

Yes

No

NA

Yes

Yes

Yes

U/P User

7

60

S

Yes

Yes

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

8

60

S

Yes

Yes

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

9

60

S

Yes

Yes

Yes

No

Yes

Yes

Yes

U/P User

10

50

S

Yes

Yes

Yes

Yes

No

NA

No

CIMClientHTTPErrorException(401)

11

50

S

Yes

Yes

Yes

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

12

50

S

Yes

Yes

Yes

Yes

Yes

Yes

Yes

U/P User

13

70

S

Yes

No

No

NA

No

NA

No

CIMClientHTTPErrorException(401)

14

70

S

Yes

No

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

15

70

S

Yes

No

No

NA

Yes

Yes

Yes

U/P User

16

100

S

Yes

No

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

17

100

S

Yes

No

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

18

100

S

Yes

No

Yes

No

Yes

Yes

Yes

U/P User

19

110

S

Yes

No

Yes

Yes

No

NA

No

CIMClientHTTPErrorException(401)

20

110

S

Yes

No

Yes

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

21

110

S

Yes

No

Yes

Yes

Yes

Yes

Yes

U/P User

 

 

 

 

 

 

 

 

 

 

 

104

610

E

Yes

Yes

No

NA

No

No

No

CIMClientHTTPErrorException(401)

105

610

E

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

106

610

E

Yes

Yes

No

NA

Yes

Yes

Yes

U/P User

107

630

E

Yes

Yes

No

No

No

No

No

CIMClientHTTPErrorException(401)

108

630

E

Yes

Yes

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

109

630

E

Yes

Yes

No

No

Yes

Yes

Yes

U/P User

110

620

E

Yes

Yes

No

Yes

No

No

No

CIMClientHTTPErrorException(401)

111

620

E

Yes

Yes

No

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

112

620

E

Yes

Yes

No

Yes

Yes

Yes

Yes

U/P User

113

670

E

Yes

No

No

NA

No

No

No

CIMClientHTTPErrorException(401)

114

670

E

Yes

No

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

115

670

E

Yes

No

No

NA

Yes

Yes

Yes

U/P User

116

690

E

Yes

No

No

No

No

No

No

CIMClientHTTPErrorException(401)

117

690

E

Yes

No

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

118

690

E

Yes

No

No

No

Yes

Yes

Yes

U/P User

119

680

E

Yes

No

No

Yes

No

No

No

CIMClientHTTPErrorException(401)

120

680

E

Yes

No

No

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

121

680

E

Yes

No

No

Yes

Yes

Yes

Yes

U/P User

Configuration 2A
----------------

In this configuration, the CIM Server will request a Client Certificate
from the CIM Client. If the CIM Client sends a certificate, the
certificate will be used for authentication. Otherwise, the
Username/Password is used by the CIM Server for authentication.

### Test Setup

  Option                                               Value
  ---------------------------------------------------- -------------
  enableHttpsConnection                                true
  sslClientVerificationMode                            optional
  PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT   NOT DEFINED

### Expected Test Results

Note: A "Yes" in the "Certificate Send?" column indicates that the
client will send the certificate if requested by the CIM Server. With
Configuration 2, we expect the CIM Server to request the certificate.

ID

Certificate

Username/Password (U/P)

Result

CID

Type

Sent?

Valid?

In\
Truststore?

Certificate\
Username\
Valid?

Sent?

U/P Valid?

Authenticated?

Authenticated User\
or\
Generated Exception

1

NA

NA

No

NA

NA

NA

No

NA

No

CIMClientHTTPErrorException(401)

2

NA

NA

No

NA

NA

NA

Yes

No

No

CIMClientHTTPErrorException(401)

3

NA

NA

No

NA

NA

NA

Yes

Yes

Yes

U/P User

 

 

 

 

 

 

 

 

 

 

 

4

10

S

Yes

Yes

No

NA

No

NA

No

CannotConnectException

5

10

S

Yes

Yes

No

NA

Yes

No

No

CannotConnectException

6

10

S

Yes

Yes

No

NA

Yes

Yes

Yes

CannotConnectException

7

60

S

Yes

Yes

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

8

60

S

Yes

Yes

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

9

60

S

Yes

Yes

Yes

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

10

50

S

Yes

Yes

Yes

Yes

No

NA

Yes

Cert User

11

50

S

Yes

Yes

Yes

Yes

Yes

No

Yes

Cert User

12

50

S

Yes

Yes

Yes

Yes

Yes

Yes

Yes

Cert User

13

70

S

Yes

No

No

NA

No

NA

No

CannotConnectException

14

70

S

Yes

No

No

NA

Yes

No

No

CannotConnectException

15

70

S

Yes

No

No

NA

Yes

Yes

No

CannotConnectException

16

100

S

Yes

No

Yes

No

No

NA

No

CannotConnectException

17

100

S

Yes

No

Yes

No

Yes

No

Yes

CannotConnectException

18

100

S

Yes

No

Yes

No

Yes

Yes

No

CannotConnectException

19

110

S

Yes

No

Yes

Yes

No

NA

No

CannotConnectException

20

110

S

Yes

No

Yes

Yes

Yes

No

No

CannotConnectException

21

110

S

Yes

No

Yes

Yes

Yes

Yes

No

CannotConnectException

 

 

 

 

 

 

 

 

 

 

 

104

610

E

Yes

Yes

No

NA

No

No

No

CIMClientHTTPErrorException(401)

105

610

E

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

106

610

E

Yes

Yes

No

NA

Yes

Yes

No

CIMClientHTTPErrorException(401)

107

630

E

Yes

Yes

No

No

No

No

No

CIMClientHTTPErrorException(401)

108

630

E

Yes

Yes

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

109

630

E

Yes

Yes

No

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

110

620

E

Yes

Yes

No

Yes

No

No

Yes

Cert User

111

620

E

Yes

Yes

No

Yes

Yes

No

Yes

Cert User

112

620

E

Yes

Yes

No

Yes

Yes

Yes

Yes

Cert User

113

670

E

Yes

No

No

NA

No

No

No

CannotConnectException

114

670

E

Yes

No

No

NA

Yes

No

No

CannotConnectException

115

670

E

Yes

No

No

NA

Yes

Yes

No

CannotConnectException

116

690

E

Yes

No

No

No

No

No

No

CannotConnectException

117

690

E

Yes

No

No

No

Yes

No

No

CannotConnectException

118

690

E

Yes

No

No

No

Yes

Yes

No

CannotConnectException

119

680

E

Yes

No

No

Yes

No

No

No

CannotConnectException

120

680

E

Yes

No

No

Yes

Yes

No

No

CannotConnectException

121

680

E

Yes

No

No

Yes

Yes

Yes

No

CannotConnectException

Configuration 2B
----------------

NOTE: This option requires the CIM Server to override the OpenSSL
certification verification and, as a result, may have unexpected
side-effects on how invalid certificates are handled.   The
PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT variable is
undocumented and subject to change.

### Test Setup

  Option                                               Value
  ---------------------------------------------------- ----------
  enableHttpsConnection                                true
  sslClientVerificationMode                            optional
  PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT   DEFINED

### Expected Test Results

Note: A "Yes" in the "Certificate Send?" column indicates that the
client will send the certificate if requested by the CIM Server. With
Configuration 2, we expect the CIM Server to request the certificate.

ID

Certificate

Username/Password (U/P)

Result

CID

Type

Sent?

Valid?

In\
Truststore?

Certificate\
Username\
Valid?

Sent?

U/P Valid?

Authenticated?

Authenticated User\
or\
Generated Exception

1

NA

NA

No

NA

NA

NA

No

NA

No

CIMClientHTTPErrorException(401)

2

NA

NA

No

NA

NA

NA

Yes

No

No

CIMClientHTTPErrorException(401)

3

NA

NA

No

NA

NA

NA

Yes

Yes

Yes

U/P User

 

 

 

 

 

 

 

 

 

 

 

4

10

S

Yes

Yes

No

NA

No

NA

No

CIMClientHTTPErrorException(401)

5

10

S

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

6

10

S

Yes

Yes

No

NA

Yes

Yes

Yes

U/P User

7

60

S

Yes

Yes

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

8

60

S

Yes

Yes

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

9

60

S

Yes

Yes

Yes

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

10

50

S

Yes

Yes

Yes

Yes

No

NA

Yes

Cert User

11

50

S

Yes

Yes

Yes

Yes

Yes

No

Yes

Cert User

12

50

S

Yes

Yes

Yes

Yes

Yes

Yes

Yes

Cert User

13

70

S

Yes

No

No

NA

No

NA

No

CIMClientHTTPErrorException(401)

14

70

S

Yes

No

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

15

70

S

Yes

No

No

NA

Yes

Yes

Yes

U/P User

16

100

S

Yes

No

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

17

100

S

Yes

No

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

18

100

S

Yes

No

Yes

No

Yes

Yes

Yes

U/P User

19

110

S

Yes

No

Yes

Yes

No

NA

No

CIMClientHTTPErrorException(401)

20

110

S

Yes

No

Yes

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

21

110

S

Yes

No

Yes

Yes

Yes

Yes

Yes

U/P User

 

 

 

 

 

 

 

 

 

 

 

104

610

E

Yes

Yes

No

NA

No

No

No

CIMClientHTTPErrorException(401)

105

610

E

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

106

610

E

Yes

Yes

No

NA

Yes

Yes

No

CIMClientHTTPErrorException(401)

107

630

E

Yes

Yes

No

No

No

No

No

CIMClientHTTPErrorException(401)

108

630

E

Yes

Yes

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

109

630

E

Yes

Yes

No

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

110

620

E

Yes

Yes

No

Yes

No

No

Yes

Cert User

111

620

E

Yes

Yes

No

Yes

Yes

No

Yes

Cert User

112

620

E

Yes

Yes

No

Yes

Yes

Yes

Yes

Cert User

113

670

E

Yes

No

No

NA

No

No

No

CIMClientHTTPErrorException(401)

114

670

E

Yes

No

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

115

670

E

Yes

No

No

NA

Yes

Yes

Yes

U/P User

116

690

E

Yes

No

No

No

No

No

No

CIMClientHTTPErrorException(401)

117

690

E

Yes

No

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

118

690

E

Yes

No

No

No

Yes

Yes

Yes

U/P User

119

680

E

Yes

No

No

Yes

No

No

No

CIMClientHTTPErrorException(401)

120

680

E

Yes

No

No

Yes

Yes

No

No

CIMClientHTTPErrorException(401)

121

680

E

Yes

No

No

Yes

Yes

Yes

Yes

U/P User

Configuration 3
---------------

### Test Setup

  Option                                               Value
  ---------------------------------------------------- ----------
  enableHttpsConnection                                true
  sslClientVerificationMode                            required
  PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT   NA

### Expected Test Results

Note: A "Yes" in the "Certificate Send?" column indicates that the
client will send the certificate if requested by the CIM Server. With
Configuration 3, we expect the CIM Server to request the certificate.

ID

Certificate

Username/Password (U/P)

Result

CID

Type

Sent?

Valid?

In\
Truststore?

Certificate\
Username\
Valid?

Sent?

U/P Valid?

Authenticated?

Authenticated User\
or\
Generated Exception

1

NA

NA

No

NA

NA

NA

No

NA

No

CannotConnectException

2

NA

NA

No

NA

NA

NA

Yes

No

No

CannotConnectException

3

NA

NA

No

NA

NA

NA

Yes

Yes

No

CannotConnectException

 

 

 

 

 

 

 

 

 

 

 

4

10

S

Yes

Yes

No

NA

No

NA

No

CannotConnectException

5

10

S

Yes

Yes

No

NA

Yes

No

No

CannotConnectException

6

10

S

Yes

Yes

No

NA

Yes

Yes

Yes

CannotConnectException

7

60

S

Yes

Yes

Yes

No

No

NA

No

CIMClientHTTPErrorException(401)

8

60

S

Yes

Yes

Yes

No

Yes

No

No

CIMClientHTTPErrorException(401)

9

60

S

Yes

Yes

Yes

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

10

50

S

Yes

Yes

Yes

Yes

No

NA

Yes

Cert User

11

50

S

Yes

Yes

Yes

Yes

Yes

No

Yes

Cert User

12

50

S

Yes

Yes

Yes

Yes

Yes

Yes

Yes

Cert User

13

70

S

Yes

No

No

NA

No

NA

No

CannotConnectException

14

70

S

Yes

No

No

NA

Yes

No

No

CannotConnectException

15

70

S

Yes

No

No

NA

Yes

Yes

No

CannotConnectException

16

100

S

Yes

No

Yes

No

No

NA

No

CannotConnectException

17

100

S

Yes

No

Yes

No

Yes

No

Yes

CannotConnectException

18

100

S

Yes

No

Yes

No

Yes

Yes

No

CannotConnectException

19

110

S

Yes

No

Yes

Yes

No

NA

No

CannotConnectException

20

110

S

Yes

No

Yes

Yes

Yes

No

No

CannotConnectException

21

110

S

Yes

No

Yes

Yes

Yes

Yes

No

CannotConnectException

 

 

 

 

 

 

 

 

 

 

 

104

610

E

Yes

Yes

No

NA

No

No

No

CIMClientHTTPErrorException(401)

105

610

E

Yes

Yes

No

NA

Yes

No

No

CIMClientHTTPErrorException(401)

106

610

E

Yes

Yes

No

NA

Yes

Yes

No

CIMClientHTTPErrorException(401)

107

630

E

Yes

Yes

No

No

No

No

No

CIMClientHTTPErrorException(401)

108

630

E

Yes

Yes

No

No

Yes

No

No

CIMClientHTTPErrorException(401)

109

630

E

Yes

Yes

No

No

Yes

Yes

No

CIMClientHTTPErrorException(401)

110

620

E

Yes

Yes

No

Yes

No

No

Yes

Cert User

111

620

E

Yes

Yes

No

Yes

Yes

No

Yes

Cert User

112

620

E

Yes

Yes

No

Yes

Yes

Yes

Yes

Cert User

113

670

E

Yes

No

No

NA

No

No

No

CannotConnectException

114

670

E

Yes

No

No

NA

Yes

No

No

CannotConnectException

115

670

E

Yes

No

No

NA

Yes

Yes

No

CannotConnectException

116

690

E

Yes

No

No

No

No

No

No

CannotConnectException

117

690

E

Yes

No

No

No

Yes

No

No

CannotConnectException

118

690

E

Yes

No

No

No

Yes

Yes

No

CannotConnectException

119

680

E

Yes

No

No

Yes

No

No

No

CannotConnectException

120

680

E

Yes

No

No

Yes

Yes

No

No

CannotConnectException

121

680

E

Yes

No

No

Yes

Yes

Yes

No

CannotConnectException
