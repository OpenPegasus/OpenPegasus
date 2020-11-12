#!/bin/sh
: '
Script used to renew self-signed certificate saved as PEM
1st arg - current PEM file
rest args - options for openssl x509 -req

new PEM is saved in same directory as old one with .new appendix

example usage: ./pemrenew.sh /tmp/keycert.pem -days 365
'

# First arg is pointed to current PEM file
cert_name="$1"

# shift off first parameter. Remainder are -req parameters
shift
echo "INPUT CERT name" $cert_name
# Create temporary files
#tmp_csr=$(mktemp /tmp/csr.XXXXXXXXX)
#tmp_key=$(mktemp /tmp/key.XXXXXXXXX)
#tmp_crt=$(mktemp /tmp/crt.XXXXXXXXX)
tmp_crt=TMP_CRT
tmp_key=TMP_KEY
tmp_csr=TMP_CSR

cert_file = "${cert_name}.cert"
tmp_pem=${cert_file} ".pem"

key_file="${cert_name}.key"
echo "FILES KEY=" $key_file " CERT=" $cert_file

# Create a corresponding pem file
openssl x509 -inform DER -outform PEM -in $cert_file -out $tmp_pem


openssl x509 -noout -enddate -in $cert_file -out $pem_file

# Extract a certificate sign request form certification file (PEM)
openssl x509 -x509toreq -in $cert_file -out $tmp_csr -signkey $key_file
echo TMP_CSR
echo $TMP_CSR
echo ""
# Extract private key from certification file (PEM)
# openssl rsa -in $pem_file -out $tmp_key
tmp_key=key_file
# Create new certificate with provided options as arguments
# TODO what is purpose of $@
# openssl x509 -req $@ -in $tmp_csr -out $tmp_crt -signkey $tmp_key
openssl x509 -req -days 3650 -in $tmp_csr -out $tmp_crt -signkey $tmp_key

openssl x509 -noout -enddate -in $$tmp_csr

# Merge certificate and key to one file
# no with crt
# cat $tmp_crt $tmp_key > $cert_file.new
cp $tmp_crt $cert_file.nae
echo "OUTPUT " $cert_file.new
openssl x509 -noout -enddate -in $cert_file.new


# Clean temporary files
#rm $tmp_csr $tmp_key $tmp_crt
