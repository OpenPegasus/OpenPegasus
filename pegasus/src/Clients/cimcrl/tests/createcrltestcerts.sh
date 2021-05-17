#!/bin/bash

# Create uppercase and lower case strings for CN and filename of each test cert
CACERTNAME=TestCA
#CACERTFILE=$(echo $CACERTNAME | tr '[:upper:]' '[:lower:]')
CACERTFILE=testcrlca1
crlfile1=testca1.crl

CERT2NAME=TestUnderCA1
CERT2FILE=testcert2

CERT3NAME=TestUnderCA2
CERT3FILE=testcert3

CERT4NAME=TestUnderCA3
CERT4FILE=testcert4

function build_subject {
	# $1 state
	# $2 locality
	# $3 CN/Common name
	echo "/C=US/ST=$1/L=$2/O=OpenGroup/OU=OpenPegasus/CN=$3"
}

CASUBJ=$(build_subject WASHINGTON Seattle $CACERTNAME)
echo CASUBJ $CASUBJ
#CERT2SUBJ="$(build_subject Florida Miami $CERT2NAME)"
CERT3SUBJ="$(build_subject Texas Dallas $CERT3NAME)"
CERT4SUBJ="$(build_subject Califoria Fairfax $CERT4NAME)"

# Build the files required by the ca command to generate serial numbes
echo 01 > serial

# build the databas
rm -f index.txt
rm -f index.txt.attr
touch index.txt

# remove any existing keys, certs, crls, csrs
rm -f *.key
rm -f *.cert
rm -f *.crl
rm -f *.csr

openssl req -new \
       -config openssl.cnf \
       -nodes \
	   -newkey rsa:2048 \
       -keyout ${CACERTFILE}.key \
       -out ${CACERTFILE}.csr \
       -subj "${CASUBJ}" \
       -verbose
       
openssl ca \
	-config openssl.cnf \
	-create_serial \
	-days 3650 \
	-keyfile ${CACERTFILE}.key \
	-selfsign \
	-extensions v3_ca \
	-batch \
	-in ${CACERTFILE}.csr \
	-out ${CACERTFILE}.cert \
	-verbose
	
CA_CERTPATH=${CACERTFILE}.cert
CA_KEYPATH=${CACERTFILE}.key

# create the  CA CRL
openssl ca \
	-config openssl.cnf \
	-gencrl \
	-keyfile ${CACERTFILE}.key \
	-cert ${CACERTFILE}.cert \
	-out ${crlfile1}

# Temp just copy the original crl.	
cp ${Ccrlfile1} testca2.crl
	
# ========================Build the other certificates===================

# Build cert function for other certs
# $1 Subject
# $2 FILE name
function bld_cert {
	CERTFILE=$2

	openssl req -new \
		-config openssl.cnf \
		-nodes  \
		-newkey rsa:2048 \
		-subj=$1 \
		-keyout ${CERTFILE}.key \
		-out ${CERTFILE}.csr
		
	# Sign and generate leaf certificate based on the CA certificate

	openssl ca \
		-config openssl.cnf \
		-create_serial \
		-extensions server_cert \
		-days 3650 \
		-notext \
		-batch  \
		-cert ${CA_CERTPATH} \
		-keyfile ${CA_KEYPATH} \
		-in ${CERTFILE}.csr \
		-out ${CERTFILE}.cert \
		-verbose
		
	# create the  CERT2NAME CRL
	openssl ca \
		-config openssl.cnf \
		-gencrl \
		-keyfile ${CERTFILE}.key \
		-cert ${CERTFILE}.cert \
		-out ${CERTFILE}.crl
		
	echo Show the trust path for ${CERTFILE}.cert
	openssl verify -show_chain -CAfile ${CA_CERTPATH} ${CERTFILE}.cert

	echo Verify the certificate chain for ${CERTFILE}.cert
	openssl verify -CAfile ${CA_CERTPATH} ${CERTFILE}.cert
}
		
# Build the second certificate and crl
CERT2SUBJ=$(build_subject Florida Miami $CERT2NAME)
bld_cert ${CERT2SUBJ} ${CERT2FILE}
# build the third cert
CERT3SUBJ="$(build_subject Texas Dallas $CERT3NAME)"
bld_cert ${CERT3SUBJ} ${CERT3FILE}

# build the forth cert
CERT4SUBJ="$(build_subject Califoria Fairfax $CERT4NAME)"
bld_cert ${CERT4SUBJ} ${CERT4FILE}

# Create crls for the month certs from cimtrust

for i in {1..12}; do
	path=$PEGASUS_ROOT/src/Clients/cimtrust/tests
	thismonth=testmonth$i
	
	echo Create crl for month $thismonth
	
	openssl ca \
		-config openssl.cnf \
		-gencrl \
		-keyfile $path/$thismonth.key \
		-cert $path/$thismonth.cert \
		-out ${thismonth}.crl
done

# Update the various crls to reflect the test requirements

# revoke the CA certificate

#openssl ca \
#    -revoke testcrlca1.cert
#    -crl_reason superseded
    
# Update the crl
#openssl ca -gencrl -out testca1.crl.crl




	
	
