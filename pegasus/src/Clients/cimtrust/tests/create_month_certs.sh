#
# Create a self-signed for each month
#
#@$(OPENSSL_COMMAND) genrsa -out $(PEGASUS_HOME)/testmonth1.key 2048
#@$(OPENSSL_COMMAND) req -config $(OPENSSL_CNF) -new -key $(PEGASUS_HOME)/testmonth1.key -out $(PEGASUS_HOME)/testdn1.csr < testmonth.txt
#@$(OPENSSL_COMMAND) x509 -in $(PEGASUS_HOME)/testdn1.csr -out $(PEGASUS_HOME)/testdn1.cert -req -signkey $(PEGASUS_HOME)/testdn1.key $(SETSERIAL_0) -days 356
#@$(RM) $(PEGASUS_HOME)/testdn1.csr

months =
cert_name="$1"
key_file="${cert_name}.key"
echo "INPUT cert_name" $cert_name
key_file="${cert_name}.key"
cert_file="${cert_name}.cert"
csr_file="${cert_name}.csr"
txt_file="${cert_name}.txt"
if [ -f "$cert_file" ]; then
    echo "$cert_file exists."
else
    echo "$cert_file does not exist. Exit"
    exit 1
fi

rm $key_name $cert_name
openssl genrsa --out $key_name 2048
openssl req -config openssl.cnf -new -key $key_file -out $csr_file < $txt_file
openssl x509 -in $csr_file -out $cert_file -set_serial 0 - days 3650
echo created $cert_name
ls -ltr $cert_name.*
openssl x509 -text -in $cert_file
