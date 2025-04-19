OpenPegasus SSL Guidelines

OpenPegasus 2.5.1 SSL Guidelines
--------------------------------

**Version:** 1.1  
**Created:** July 20, 2005

**Updated:** March 20, 2006

*   [Overview](#OVERVIEW)
*   [Related Information](#RELATED)
*   [Building Pegasus with SSL](#BUILDING)
*   [Creating SSL Certificates](#CERTS)
*   [Configuring Pegasus for SSL](#CONFIGURE)
*   [SSL Design Question List](#DESIGN)
*   [Truststore Management](#TRUSTSTORE)
*   [cimtrust & cimcrl CLI](#CLI)
*   [Configuring the Pegasus CIM Client for SSL](#CLIENT)
*   [SSL Authorization](#AUTH)
*   [Critical Extension Handling](#EXT)
*   [Resources](#RESOURCES)

### Overview

The following document serves as a guide on how to build and configure Pegasus for SSL support. It also discusses how to utilize a certificate-based infrastructure and configure the Pegasus CIM client.

This guide requires a basic understanding of SSL, OpenSSL, and basic authentication. This guide is intended to help developers and administrators make the right decisions about how to use SSL for their particular application. It is not intended to be a primary source of education on SSL. If you are not familiar with these technologies, consult the sources in the [Resources](#RESOURCES) section at the bottom.

Note: In this document, the term "trust" refers only to authentication. It does not imply full trust in the traditional sense, because it does not take into account authorization checks. It remains the responsibility of providers and clients to perform authorization, and therefore establish real trust. Likewise, the term "Trust Store" can be misleading since the "store" is only a source of authentication credentials. Please bear this in mind when documenting recommended deployments or building clients or providers.

### Related Information

A significant portion of the information in this document is taken from various PEP's. This document attempts to bring all of this information together in a cohesive and simplified format.

*   PEP#035 - Add support for /dev/random in SSLContext
*   PEP#060 - SSL support in CIM/XML indication delivery
*   PEP#074 - SSLContext and Certificate verification interface enhancement
*   PEP#165 - SSL Client Verification
*   PEP#187 - SSL Certificate Management Enhancements
*   PEP#200 - Recommended OpenPegasus 2.5 Build and Configuration Options for Selected Platforms

### Building Pegasus with SSL

To build Pegasus with HTTPS support, you will need to build against the [OpenSSL package](http://www.openssl.org). The SSL support outlined here has been tested against recent releases of the major versions 0.9.7X and 0.9.8X (most notably, 0.9.7d). Because some versions of 0.9.6X do not contain full support for the security functions that Pegasus utilizes (for example, certificate-based authentication is not fully supported by some versions of 0.9.6X), Pegasus does not officially support major version 0.9.6. See Bugzilla 4048 for more information. Because this is an open source project, the SSL support has been tested with many versions of OpenSSL, but we cannot guarantee it has been tested with every version on every platform. A list of recent OpenSSL releases, and important-to-review security advisories and fixes, can be found on the [OpenSSL News page](http://www.openssl.org/news).

After grabbing the OpenSSL source tarball, you need to set the following environment variables before building Pegasus:

*   PEGASUS\_HAS\_SSL=1
*   OPENSSL\_HOME=<location of the SDK package> This directory must contain the OpenSSL include directory, $(OPENSSL\_HOME)/include, and the OpenSSL library directory, $(OPENSSL\_HOME)/lib.
*   OPENSSL\_BIN=<location of the binary package> This only needs to be set if the OpenSSL binaries are not in $(OPENSSL\_HOME)/bin.

Note that Pegasus supports SSLv3 and TLSv1 by default. It does NOT support SSLv2. To turn on SSLv2 support, enable the additional environment variable:

*   PEGASUS\_ENABLE\_SSLV2=1

It is not recommended to enable this protocol, as there have been many security weaknesses associated with it. Unless you are dealing with very outdated clients, you probably do not need to enable it.

After setting these variables, proceed as normal with the build instructions in the readme file.

### Creating SSL Certificates

There are two options for creating the CIMOM's certificate:

*   Self-signed certificate
*   Certificate issued by a third-party certificate authority

To generate a self-signed certificate, you must create a private key, a certificate signing request (CSR), and finally the public x509 certificate. You also need an SSL configuration file that defines the parameters of the Distinguished Name (DN). You can use the one that comes with Pegasus, ssl.cnf in the root directory, or generate your own. For a self-signed certificate, the subject is the same as the issuer. Execute the following commands to create a self-signed certificate. The PEGASUS\_ROOT and PEGASUS\_HOME have to be set to your respective installation and source directory. You will also need an OpenSSL configuration file. There is a sample configuration file that comes with the OpenSSL package.

*   To generate a private key, execute the following:  
    openssl genrsa -out myserver.key 1024  
    Set the "sslKeyFilePath" configuration property to point to this key file.
*   To generate a certificate signing request, execute the following:  
    openssl req -config openssl.cnf -new -key myserver.key -out myserver.csr
*   At this point, the certificate signing request can be sent out to a third-party certificate authority for signing, or a self-signed certificate can be generated. To generate a self-signed certificate, execute the following:  
    openssl x509 -in myserver.csr -out myserver.cert -req -signkey myserver.key -days 365  
    Set the "sslCertificateFilePath" configuration property to point to this certificate file. The above CSR file can be discarded after the certificate is created.

After creating the keypair, make sure you protect the information sufficiently by changing permissions on the files and/or directories. The following table shows the recommended privileges:

**SSL file**

**Pegasus Config property**

**Permissions**

Private key

sslKeyFilePath

rwx------

Public certificate

sslCertificateFilePath

rwxr-xr-x

Truststore

sslTrustStore

rwxr-xr-x

CRL store

crlStore

rwxr-xr-x

The administrator is responsible for ensuring that the above file permissions are set correctly. The administrator should also ensure that all containing directories all the way up to the base directory are not world-writable. Pegasus only checks the following conditions when starting up:

*   The sslKeyFilePath and the sslCertificateFilePath are readable by the CIMOM.
*   The sslTrustStore and crlStore are readable by the CIMOM if they are a single file.
*   The sslTrustStore and crlStore are readable and writable by the CIMOM if they are a directory.

These same file permissions should be used for protecting a client's private key, public key, truststore, and crl store as well.

For more information on generating keys and certificates, consult the [OpenSSL HOW-TO documentation](http://www.openssl.org/docs/HOWTO/).

### Configuring Pegasus for SSL

There are many environment variable settings associated with SSL. Here is a brief discussion of the subtleties of these options and how they work together to create a more secure environment. More information on the default and recommended settings can be found in PEP#200 Recommended OpenPegasus 2.5 Build and Configuration Options for Selected Platforms. Additionally, the section on [Design Question List](#DESIGN) should help determine what these settings should be for a given application.

**enableHttpsConnection**  
This is disabled by default on most platforms. It is recommended that all remote communication be done over the HTTPS port. However, if you are sending cleartext passwords over the wire, it is imperative that you only use the secure port. For added security, the HTTP port can be disabled to prevent clients from connecting to it. The HTTPS connection is enabled by default only on the following platforms:

*   LINUX
*   OS-400
*   HP\_UX (if PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS is true)
*   VMS (if PEGASUS\_USE\_RELEASE\_CONFIG\_OPTIONS is true)

**httpsPort**  
The default setting is 5989, the official WBEM secure port.

**sslCertificateFilePath**  
This is the path to the x509 server certificate. The server certificate may be a chain in which case the file should contain PEM encoded certificates beginning with the server certificate and followed by each signing certificate authority (CA) including the root CA. If the server certificate is a self signed certificate, the file only contains the self-signed certificate in PEM format. The certificate cannot be encrypted because there is currently no mechanism for decrypting the certificate using a user-supplied password. This property must be defined if enableHttpsConnection is true. Any failure in finding this file will result in the cimserver failing to start. See [Creating SSL Certificates](#CERTS) for more information.

**sslKeyFilePath**  
This is the path to the server's private key. All keys should be at least 1024 bytes long. This property must be defined if enableHttpsConnection is true. Any failure in finding this file will result in the cimserver failing to start. See [Creating SSL Certificate](#CERTS) for more information.

**sslClientVerificationMode**  
This setting controls how the cimserver (i.e. the HTTPS port) is configured. There are three possible settings: disabled, required, optional. There is no "right" setting for this property. The default is disabled and it is fine to leave the setting as disabled if you are going to use basic authentication to authenticate all client requests. In many applications where a physical person is there to supply a username and password, basic authentication is sufficient. Other environments may be heterogeneous, in which case it makes sense to allow both basic authentication and SSL certificate verification. The setting of this variable also impacts what happens during the OpenSSL handshake:

*   **"required"** -- The server requires that the client certificate be trusted in order for the handshake to continue. If the client fails to send a certificate or sends an untrusted certificate, the handshake is immediately terminated.
*   **"optional"** -- The server will request that a client certificate be sent, but will continue the handshake even if no certificate is received. If authentication is enabled, the server will seek to authenticate the client via an alternative method of authentication. As of 2.5.1, if a certificate is sent but it is not validated, the handshake will fail. _Before 2.5.1,the handshake would have continued and basic authentication would have proceeded._
*   **"disabled"** -- The server will not prompt the client for a certificate. _This is the default._

Pegasus currently ties a certificate to a valid OS user. Multiple certificates may be registered to the same user. When a certificate is authenticated, Pegasus views it in the same way as if a user was authenticated via basic authentication. The providers receive the username that the certificate was mapped to. See the SSL Authorization section for more information.

**sslTrustStore**  
This setting controls the truststore for the cimserver's HTTPS connection. It can be either a directory or a single root CA file. When set to a directory, it is recommended that you use the cimtrust CLI to populate the truststore as there are strict naming requirements for trusted certificate files. See the [cimtrust & cimcrl CLI](#CLI) section for further information.

**sslTrustStoreUserName**  
This setting is only utilized if the sslTrustStore is a single CA file. It is not used if the sslTrustStore setting is a directory, but it still must be set to a valid system user. This is because the validation of the property is done independently of the sslTrustStore setting. This property represents the valid OS user that corresponds to the root certificate. All requests authenticated with a certificate under the root CA will be associated with this user and the username will be propagated to providers. If applications desire for there to be a one-to-one correspondence between users and certificates, it is recommended that each certificate be registered individually using the [cimtrust CLI](#CLI).

**crlStore**  
This is where the CRL (Certificate Revocation List) store resides. It is important to note that certificates are checked first against the CRL (if specified) and then against the server truststore. The [cimcrl CLI](#CLI) should be used for CRL management.

**sslCipherSuite**  
This setting specifies the cipher list used by the server during the SSL handshake phase. If not specified, the "DEFAULT" OpenSSL cipher list is used. The cipher list should be mentioned between single quotes since it can contain special characters like .+, !, -. The cipher lists can be found at [http://www.openssl.org/docs/apps/ciphers.html#CIPHER\_LIST\_FORMAT](http://www.openssl.org/docs/apps/ciphers.html#CIPHER_LIST_FORMAT)

**sslBackwardCompatibility**  
This setting specifies whether the ssl supports SSLv3 and versions of TLS lesser than 1.2. Ideally for security Compilance purposes it is by default set to false.

#### Configuration Limitations

The following are configuration limitations:

*   The x509 server certificate file cannot be encrypted. The reason for this is that there is currently no mechanism in Pegasus to grab the password needed to unencrypt it. Therefore, the best way to secure the file is to follow the file permissions settings specified in [Creating SSL Certificates.](#CERTS)
*   The verification depth cannot be specified. Pegasus uses the default OpenSSL depth of 9. This means the OpenSSL will only accept client certificate chains up to 9 levels deep.
*   No hostname checking is performed to ensure that the subject field of the distinguished name (DN) matches the hostname.

### SSL Design Question List

The following questions may be helpful in determining how to configure Pegasus CIM Server.

**Should I enable the HTTPS port?**  
Yes, especially if you are sending passwords with requests. The HTTP port can be disabled for additional security if desired.  
**Should I configure the CIMOM to use a truststore?**  
This depends on the infrastructure of the application. If all clients are using basic authentication over the secure port (and the passwords are secured), then a truststore may not be needed. If an application does not want to store user/pw information, then it is a good idea to use a certificate-based infrastructure. If a CIMOM certificate is compromised, the cimserver and the providers of the system are compromised. The severity of this scenario is dependent on the resources the providers have access to. If an OS password is compromised, the entire system may be compromised. If using peer verification, it is important to ensure that 1) the cimserver is properly configured to use a truststore, 2) the truststore is loaded properly and protected, and 3) authorization checks are performed after a certificate is verified. These same conditions also apply to a client that is verifying a server.  
**Should I use a self-signed certificate or one issued by a third-party certificate authority?**  
Generally, scalability will determine whether it's appropriate to use a self-signed certificate or one issued by Verisign or another third-party certificate authority. If an administrator administrates their self-signed certificates correctly, they are no less secure than one issued by a CA. What a CA buys you is scalability. An up front cost of setting up a CA relationship will be offset by the convenience of having that CA "vouch" for certs it has signed, in large deployments. In small deployments the incremental cost might never outweigh the initial CA-setup cost.  
One important thing to remember is that you should not use the same certificate for multiple CIMOMs. If using a self-signed certificate, a different one should be generated for each CIMOM, using some unique piece of data to make them different. That way, if one of the certificates is compromised, the other ones remain secure.  
**Should the truststore be a single root CA file or a directory?**  
If you only anticipate connections from a narrowly defined set of clients, then a single root CA certificate file should be sufficient. Alternatively, multiple trusted certificates may be stored in PEM format inside of a single CA file. If you anticipate getting requests from a heterogeneous set of clients, then it probably makes sense to use the directory option to allow flexibility in the future. In the latter scenario, the same single root CA file can still be used with the additional step of using cimtrust to register it. It's important to note that when registering a root CA, only one user can be associated with ALL certificates under that CA. Following the principle of least privilege, it is not a good idea to register a root CA to a privileged user if lesser privileged users will be connecting with it.  
**How do I protect the keystore and the truststore?**  
The server's private key should always be protected; it is private for a reason. Only the system administrator should be able to see it. The public certificate can be viewed by anyone, however, it should be protected from alteration by system users. Similarly, any truststore or CRL file or directory should also be protected from alteration. See [Creating SSL Certificates](#CERTS) for the recommended file privileges.  
**When do I need to use a CRL?**  
Certificate Revocation Lists are regularly issued by CA's. They contain a list of certificates that have been revoked. Any application using a CA certificate in its truststore should also implement CRLs (if the CA supports them). Pegasus itself does not check CRL validity dates during startup. Therefore, it is the responsibility of the administrator to regularly download or acquire the CRL and import it into the CRL store using the [cimcrl CLI](#CLI). CRLs are not checked for expiration during the SSL callback. This means that if a CRL for a particular issuer has expired, Pegasus still accepts certificates from the issuer and uses the expired CRL as the latest. Again, it is the responsibility of the administrator to ensure the CRL is up to date. CRLs are not checked for critical extensions during CRL verification. If a CRL contains a critical extension it will be ignored.  
If using self-signed certificates, however, a CRL is most likely not needed (You can create a self-signed CRL but it is not really necessary). Because of this, the certificate deletion option available via cimtrust is primarily intended for self-signed certificates. Technically, CRL's are the correct way to revoke compromised or invalid certificates.  
**What is the order of operations for certificate verification?**  
The certificate is checked against any CRLs first before going through the rest of the verification process. Verification starts with the root certificate and continues down to the peer certificate. If verification fails at any of these points, the certificate is considered untrusted and the verification process reports an error.

### Truststore Management

There are two directions of trust in an SSL client-server handshake: The client trusts the server. The server trusts the client. Pegasus provides a way to implement one or both of these relationships. Ideally, an application should support both levels of trust for maximum security and this is the implementation Pegasus recommends. However, in some scenarios it may make sense to only implement one of these; in that case, it is possible to override the client or the server to "trust all certificates." For example, if all clients will be using basic authentication over HTTPS, then the server can be setup to "trust all client certificates."

To tell the cimserver to require that all clients be trusted, simply set the sslClientVerificationMode property to "required."  
To tell the cimserver to trust all clients, set the sslClientVerificationMode property to "disabled" or "optional".

The SSL verification in Pegasus is independent of any other authentication mechanism. It can still be utilized when authentication is disabled. When authentication is enabled, the first line of defense is SSL client verification. In situations where a client is not authenticated by SSL because the client sent no certificate and the setting is "optional", the server will attempt to authenticate the client via another method of authentication . In this case, the authentication mechanism specified by the configuration property "httpAuthType" will be used for remote connections and local authentication will be used for local connections. In situations where a client is not authenticated by SSL because the client certificate was invalid, the handshake will be terminated.  
_Note: Before 2.5.1, in the latter case, authentication would have proceeded in the same way as if the client had sent no certificate. To enable the legacy behavior, the compile-time flag PEGASUS\_OVERRIDE\_SSL\_CERT\_VERIFICATION\_RESULT should be defined._

See the [Configuring the Pegasus CIM Client for SSL](#CLIENT) section below on how to setup the client's truststore.

### cimtrust & cimcrl CLI

cimtrust CLI may be used to add, remove or list X509 certificates in a PEM format truststore. cimcrl CLI may be used to add, remove or list X509 Certificate Revocation Lists in a PEM format CRL store. The CLIs interface with a Certificate control provider that runs as part of Pegasus's core. It operates on the PG\_SSLCertificate and PG\_SSLCertificateRevocationList classes in root/PG\_Internal. It is recommended that the CLIs be used in place of manual configuration for several reasons:

*   OpenSSL places strict naming restrictions on certificates and CRLs in a directory (the files are looked up via a subject hash code)
*   Certificate instances are stored in the repository along with the corresponding username. If the certificate is not properly registered, the username mapping will fail. cimtrust CLI supports the ability to register a certificate without a username for root certificates and intermediate certificates, since these certificates represent a collection of users. In this scenario, each leaf certificate must be registered to an individual user. See the Authorization section for more information on username validation.
*   The CLIs, or more correctly the provider they operate on, supports dynamic deletion of certificates by resetting the cimserver's SSL context. Normally, you would need to stop and start the cimserver to accomplish this.
*   The CLIs, or more correctly the provider they operate on, performs a ton of error checking you would not get by manually configuring the stores. This alerts the administrator to various error conditions (e.g. the certificate expired) associated with a certificate or CRL.

The CIMOM must be up and running while executing cimtrust/cimcrl CLI. The cimtrust and cimcrl manpages provide more information on commands and syntax.

### Configuring the Pegasus CIM Client for SSL

A Pegasus CIM client can be configured to use SSL by using a constructor that takes an SSLContext. The construction of the SSLContext is really what controls the behavior of the client during the SSL handshake. Without going into minute details about what happens under the covers, here is a description of the various SSLContext constructor parameters.

Here's a code snippet that shows how to call a client constructor that connects to a server over SSL and can present its own trusted certificate if the server requests it. In this scenario, the client also checks the server certificate against its truststore and specifies an additional callback in addition to the default one (the user-specified callback is optional and can be set to null).

client.connect( hostname, port, **SSLContext(trustStore, certPath, keyPath, verifyCert, randomFile, cipherSuite),** username, password);

Here's a code snippet that shows how to call a client constructor that connects to a server over SSL and does not possess its own trusted certificate. In this scenario, the client also checks the server certificate against its truststore.

client.connect( hostname, port, **SSLContext(trustStore, NULL, randomFile),** username password);

*   **trustStore** -- This specifies the truststore that the client uses to verify server certificates. It can be String::EMPTY if no truststore exists.
*   **certPath** -- This specifies the x509 certificate of the client that will be sent during an SSL handshake. Note that this certificate will only be sent if the server requests it. If this option is specified, the keyPath parameter must also be specified.
*   **keyPath** -- This specifies the private key of the client. If this option is specified, the certPath parameter must also be specified.
*   **crlPath** -- This specifies an optional CRL store path. The client checks the CRL list first, before attempting any further authentication, including the user-specified callback.
*   **verifyCert** -- This is a user-specified verification callback. If this is set to null, the default OpenSSL verification callback will be executed. You can implement this method to "trust all servers" or to perform additional authentication checks that OpenSSL does not perform by default.
*   **randomFile** -- A file to seed the pseudo random number generator (PRNG).
*   **cipherSuite** -- This specifies the cipher list used by the client during the SSL handshake phase. This is an experimental interface.

Here are some general guidelines on implementing peer verification for the client:

*   The client should enable peer verification by specifying a truststore and (optionally) a user-specified callback function.
*   The client should employ a truststore in order to properly verify the server. The truststore should contain a file or directory of trusted CA certificates. The cimtrust CLI cannot be used to configure client truststores. The trusted certificate(s) should be placed in a protected file or directory specified by the trustStore parameter. Keep in mind that the SSL context generally has to be reloaded to pick up any truststore changes.
*   The client could also use a user-specified callback in addition to the default verification callback, if additional verifications are desired over the normal checks that OpenSSL performs. In most cases, the default verification callback is sufficient for checking server certificates.
*   The client should ensure that adequate entropy is attained.
*   The client should use a CRL store if the truststore contains CA certificates that support one.
*   The client should only use the SSLv3 and TLSv1 protocols. By default, Pegasus is not built with SSLv2 support.
*   The client should perform post-connection checks.

*   Ensure a certificate was received.

*   WARNING:  In some implementations of SSL a NULL server certificate is perfectly valid and authenticates against all trust stores.  If the client does not ensure a certificate exists then the client is not providing server authentication and could have a security bulletin class defect.

*   Validate that the certificate received was issued to the host for which the client was attempting to connect.

*   Ensure that the common name (CN) in the server’s certificate subject matches the host name of the server.  For X509v3 certificates, the “SubjectAltName” fields in the certificate's extended attributes are also valid host names for the certificate.
*   WARNING:  If the client does not ensure the host name of the server is the same as one of the host names explicitly described in the server’s certificate, you have not authenticated the server’s identity.  Any other server which was issued a certificate from the same trusted CA can masquerade as the server unless the client performs the host name check.

*   Ensure that certificate verification methods/routines return no errors.

Because only the above arguments can be passed into the Pegasus SSLContext, there are some limitations in the client configuration:

*   The verification depth cannot be specified. Pegasus uses the default OpenSSL depth of 9.
*   No hostname checking is performed to ensure that the subject field of the distinguished name (DN) matches the hostname. If desired, a user-specified callback should be configured to perform this check or any additional checks relevant to the application.

### SSL Authorization

The following paragraphs concern authorization of users authenticated by certificate on the cimserver's HTTPS port.

It is important to note that SSL certificates are verified during the initial handshake, BEFORE any further authentication takes place. If a certificate fails, the connection can be terminated immediately, resulting in a connection exception. This scenario will occur if the sslClientVerification property is set to "required" and no certificate or an untrusted certificate is sent.

Further _**authorization**_ checks must be performed when validating the user that is mapped to the certificate. First, the user that is registered to the certificate is validated as a valid system user and a valid cimuser (if the cimuser function has been configured). In the case of a certificate chain, the username authorization starts with the leaf certificate. If it successfully finds a mapping for the leaf certificate, it continues; if there is no username for the leaf certificate, the validation proceeds up to the root certificate. If the root certificate is reached and there is still no mapped username, the authorization fails. Additionally, if Pegasus was configured to use PAM, the pam\_acct\_mgmt function will be called with the user that is mapped to the certificate. This ensures that any login conditions that would have been placed on a user authenticated via basic authentication are still applied to a user authenticated via certificate. The pam\_authenticate method will NOT be called. Lastly, the providers must authorize the user. They receive the username that was mapped to the certificate in the OperationContext.

### Critical Extension Handling

The extensions defined for X.509 v3 certificates provide methods for associating additional attributes with users or public keys and for managing the certification hierarchy. Each extension in a certificate may be designated as critical or non-critical. Pegasus relies on the underlying OpenSSL implementation to handle critical extensions specified in a certificate. Please refer to the OpenSSL documentation for more information on currently supported extensions in OpenSSL and on the behavior of OpenSSL in the case of unhandled critical extensions.

### Resources

For OpenSSL information pick up a copy of O'Reilly's Network Security with OpenSSL or go to the OpenSSL Site:  
[http://www.openssl.org](http://www.openssl.org)

A really fabulous guide on certificate management and installation with OpenSSL:  
[http://www.gagravarr.org/writing/openssl-certs/index.shtml](http://www.gagravarr.org/writing/openssl-certs/index.shtml)

x509 Certificate and CRL RFC:  
[http://www.ietf.org/rfc/rfc2459.txt?number=2459](http://www.ietf.org/rfc/rfc2459.txt?number=2459)

SSLv3 RFC:  
[http://wp.netscape.com/eng/ssl3](http://wp.netscape.com/eng/ssl3/)

TLSv1 RFC:  
[http://www.ietf.org/rfc/rfc2246.txt](http://www.ietf.org/rfc/rfc2246.txt)

Basic Authentication RFC:  
[http://www.faqs.org/rfcs/rfc2617.html](http://www.faqs.org/rfcs/rfc2617.html)

* * *

Licensed to The Open Group (TOG) under one or more contributor license agreements. Refer to the OpenPegasusNOTICE.txt file distributed with this work for additional information regarding copyright ownership. Each contributor licenses this file to you under the OpenPegasus Open Source License; you may not use this file except in compliance with the License.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* * *