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
OPENSSL_CNF=minica.cnf

# Create the Testchca CA (root) key and certificate
CERTNAMEROOT=testchca
echo ------------------Create key for $CERTNAMEROOT
# generate private key
#openssl genrsa -out ${CERTNAMEROOT}.key 2048
# generate certificate signing request
echo -------------------create root csr  for ${CERTNAMEROOT}.

#  	#-newkey rsa:2048 -keyout ${CERTNAMEROOT}.key \
	#-set_serial 0 \

# # From Makefile This has bunch of errors right now.
#openssl req -new \
	#-config ${OPENSSL_CNF} \
	#-key ${CERTNAMEROOT}.key \
	#-x509 \
	#-set_serial 0 -days 3650 \
	#-out ${CERTNAMEROOT}.cert < ${CERTNAMEROOT}.txt
#exit

openssl req -new \
 	-config ${OPENSSL_CNF} \
 	-key ${CERTNAMEROOT}.key \
 	-nodes \
 	-set_serial 1 \
 	-days 3650 \
 	-newkey rsa:2048 \
 	-keyout ${CERTNAMEROOT}.key \
 	-verbose \
	-out ${CERTNAMEROOT}.csr < ${CERTNAMEROOT}.txt
	
echo show results after csr create
ls -ltr ${CERTNAMEROOT}.*

# Generate the signed certificate
#	-CAkey ${CERTNAMEROOT}.key \
	# -set_serial 0\

mkdir -p newcerts
echo ------------------create CA root certificate ${CERTNAMEROOT}.cert

echo 0 > ${CERTNAMEROOT}.srl

openssl ca \
	-config ${OPENSSL_CNF} \
	-key ${CERTNAMEROOT}.key \
	-selfsign \
	-keyfile ${CERTNAMEROOT}.key \
	-extensions v3_ca \
	-days 3650 \
	-create_serial \
	-batch \
	-verbose \
	-in ${CERTNAMEROOT}.csr \
	-out ${CERTNAMEROOT}.cert
	
#Verify the certificate

openssl verify -verbose -CAfile {CERTNAMEROOT}.cert {CERTNAMEROOT}.cert
	
exit

# From other test
openssl ca \
	-config ./rootssl.cnf \
	-create_serial \
	-days 3650 \
	-keyfile $KEY \
	-selfsign \
	-extensions v3_ca \
	-batch \
	-in $CSR \
	-out certs/${CA_FILENAME}.cert \
# verify root  certificate

openssl x509 -noout -text -in certs/${CA_FILENAME}.cert

# verify as valid
openssl verify -verbose -CAfile certs/${CA_FILENAME}.cert certs/${CA_FILENAME}.cert




openssl ca \
	-config ${OPENSSL_CNF} \
	-key ${CERTNAMEROOT}.key \
	-selfsign \
	-extensions v3_ca \
	-days 3650 \
	-batch \
	-create_serial \
	-in ${CERTNAMEROOT}.csr \
	-out ${CERTNAMEROOT}.cert

exit
#
# create the intermediate key, csr and signed cert
#
CERTNAMEINTER="testinter"
echo create $CERTNAMEINTER
openssl genrsa -out ${CERTNAMEINTER}.key 2048
echo create inter csr
openssl req \
	-new 
	-config ${OPENSSL_CNF} \
	-key ${CERTNAMEINTER}.key \
	-out ${CERTNAMEINTER}.csr < ${CERTNAMEINTER}.txt
echo create inter cert
openssl x509 -req \
	-in ${CERTNAMEINTER}.csr \
	-set_serial 2 \
	-days 3650 \
	-CA ${CERTNAMEROOT}.cert \
	-CAkey ${CERTNAMEROOT}.key \
	-out testinter.cert
# rm testinter.csr

# create the leaf key, csr and signed cert
CERTNAMELEAF=testleaf
echo create $CERTNAMELEAF

openssl genrsa -out ${CERTNAMELEAF}.key 2048
echo create csr
openssl req \
	-new \
	-config ${OPENSSL_CNF} \
	-key ${CERTNAMELEAF}.key \
	-out ${CERTNAMELEAF}.csr < ${CERTNAMELEAF}.txt
echo create cert
openssl x509 -req \
	-set_serial 2 
	-days 3650 \
	-in ${CERTNAMELEAF}.csr \
	-CA ${CERTNAMEINTER}.cert \
	-CAkey ${CERTNAMEINTER}.key \
	-out ${CERTNAMELEAF}.cert

