#! /bin/bash

# Create new certs for this test.  This provides a way to create new
# certs that are not specifically created as part of the Makefile. These certs
# should duplicate the existing certs with the same name except for changes
# made in this script This
# includes:

# The testchca cert
# The testinter cert
# The testleaf cert

# If these certs are created with this script the result.master must also
# be recreated since data in the certs is modified, thus modifying
# the results of the test.
OPENSSL_CNF=openssl.cnf

# Create the Testchca CA key and certificate
openssl genrsa -out testchca2.key 2048
openssl req -new -key testchca2.key -x509 -config openssl.cnf -set_serial 0 -days 3650 -out testchca2.csr < testchca.txt

# create the intermediate key, csr and signed cert
openssl genrsa -out testinter.key 2048
openssl req -config ${OPENSSL_CNF} -new -key testinter.key -out testinter.csr < testinter.txt
openssl x509 -req -set_serial 2 -days 3650 -in testinter.csr -CA testchca.cert -CAkey testchca.key -CAcreateserial -out testinter.cert
# rm testinter.csr

# create the leaf key, csr and signed cert
openssl genrsa -out testleaf.key 2048
openssl req -config ${OPENSSL_CNF} -new -key testleaf.key -out testleaf.csr < testleaf.txt
openssl x509 -req -set_serial 2 -days 3650 -in testleaf.csr -CA testinter.cert -CAkey testinter.key -CAcreateserial -out testleaf.cert

