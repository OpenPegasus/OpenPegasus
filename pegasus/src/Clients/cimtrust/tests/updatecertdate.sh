#!/bin/sh
: '
Script used to renew self-signed certificate. certificate and key file must
exist
1st arg - current certificate file (suffix cert)
rest args - options for openssl x509 -req

new cerfificate is saved in same directory as old one with .new appendix

example usage: ./certrenew.sh keycert.cert -days 365
'

# First arg is pointed to current PEM file
cert_name="$1"

# shift off first parameter. Remainder are -req parameters
shift

echo "INPUT CERT name" $cert_name
# Create temporary files
tmp_crt=TMP.cert
tmp_csr=TMP.csr

cert_file="${cert_name}.cert"
echo "CERT FILE NAME " $cert_file
if [ -f "$cert_file" ]; then
    echo "$cert_file exists."
else
    echo "$cert_file does not exist. Exit"
    exit 1
fi

key_file="${cert_name}.key"

# display the current end date
echo "ORIGINAL END DATE"
openssl x509 -noout -enddate -in $cert_file

# Extract a certificate sign request from certificate file
openssl x509 -x509toreq -in $cert_file -out $tmp_csr -signkey $key_file

# Create new certificate with provided options as arguments
# TODO what is purpose of $@
# openssl x509 -req $@ -in $tmp_csr -out $tmp_crt -signkey $tmp_key
openssl x509 -req -days 3650 -in $tmp_csr -signkey $key_file -out $tmp_crt

echo NEW END DATE for $tmp_crt
openssl x509 -noout -enddate -in $tmp_crt

cp $tmp_crt ${cert_file}
echo "NEW CERT enddate "
openssl x509 -noout -enddate -in "${cert_file}"
rm $tmp_crs $tmp_crt
