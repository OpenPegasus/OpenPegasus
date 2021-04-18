#!/bin/bash
#
# Create a self-signed for each month
#
#@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testmonth1.key 2048
#@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testmonth1.key -out $(PEGASUS_HOME)/testdn1.csr < testmonth.txt
#@$(OPENSSL_COMMAND) x509 -in $(PEGASUS_HOME)/testdn1.csr -out $(PEGASUS_HOME)/testdn1.cert -req -signkey $(PEGASUS_HOME)/testdn1.key $(SETSERIAL_0) -days 356
#@$(RM) $(PEGASUS_HOME)/testdn1.csr

# parameter 1 is search string, parameter 2 is the array
# Return 0 if found, 1 if not found
#array=("something to search for" "a string" "test2000")
#ElementIn "a string" "${array[@]}"
#echo $?
# returns 1, not found
#ElementIn "blaha" "${array[@]}"
#echo $?
ElementIn () {
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

CERT_SN=18446744073709551615

# Create a single month cert from the integer in $1 
CreateMonthCert() {
	NUMBER=$1
	if [[ $NUMBER  -gt 12 ]]  || [[ $NUMBER -lt 1 ]]; then
		echo invalid input $NUMBER. must be between 1 and 12
	    exit 1
	fi

	cert_name=testmonth"$NUMBER"
	echo CERTNAME $cert_name

	key_file="${cert_name}.key"
	echo "INPUT cert_name" $cert_name
	key_file="${cert_name}.key"
	cert_file="${cert_name}.cert"
	csr_file="${cert_name}.csr"
	txt_file="${cert_name}.txt"
	if [ -f "$cert_file" ]; then
		echo "old $cert_file exists."
	else
		echo "$cert_file does not exist. Exit"
		#exit 1
	fi
	months_array=("unknown" "Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sept" "Oct" "Nov" "Dec")

	MONTH=${months_array[ $1 ]}

	SUBJ="/C=US/ST=VIRGINIA/L=Fairfax/O=OpenGroup/OU=OpenPegasus/CN=TestSelfSigned1-$MONTH"

	# Alternate form openssl genpkey -algorithm RSA -out $key_fo;e -pkeyopt rsa_keygen_bits:2048
	
	# Generate the key
    openssl genrsa \
		-out $key_file \
		2048
		
	# generate the csr
	openssl req  \
	    -config openssl.cnf  \
	    -new -key $key_file -out \
	    $csr_file  -subj $SUBJ
	    
	 echo created CSR file $csr_file
	    
	# display the csr    
	openssl req -noout -text -in $csr_file

    # generate the new certificate
	#openssl x509 -req -in $csr_file -out $cert_file -set_serial 0 -days 3650
	echo creating certfile
    openssl x509  \
        -req \
        -in $csr_file  \
        -signkey $key_file \
        -out $cert_file  \
        -set_serial $CERT_SN \
        -days 3650	
        
    # was set_serial 18446744073709551615
	
	if [ -f $cert_file ]; then
		echo created $cert_name
		ls -ltr $cert_name.*
		openssl x509 -text -in $cert_file
	else
	   echo CERTFILE $cert_file not created
	   exit 1
	fi	
}

if [ -z "$1" ]; then
	for i in {1..12}; do
		CreateMonthCert $i
	done
	
else
    CreateMonthCert $1
fi

