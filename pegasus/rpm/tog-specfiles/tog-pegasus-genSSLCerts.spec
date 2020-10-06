#
#  Set up OpenSSL certificates for the tog-pegasus cimserver
#
#  Creates a default ssl.cnf file.
#  Generates a self-signed certificate for use by the cimserver.
#

function create_ssl_cnf #(config_file, CN)
{
    SSL_CFG=$1
    CA=$2 # Add a second argument to differentiate issuer from subject

    # Create OpenSSL configuration files for generating certificates
    echo "[ req ]" > $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "distinguished_name     = req_distinguished_name"  >> \
            $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "prompt                 = no"  >> $PEGASUS_CONFIG_DIR/$SSL_CFG

    # Include support for x509v3 so we can differentiate CA certificates
    # from service certificates
    echo "req_extensions         = v3_req" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "x509_extensions        = v3_ca" >> $PEGASUS_CONFIG_DIR/$SSL_CFG

    echo "[ req_distinguished_name ]" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "C                      = UK" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "ST                     = Berkshire" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "L                      = Reading" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "O                      = The Open Group" >> \
            $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "OU                     = The OpenPegasus Project" >> \
            $PEGASUS_CONFIG_DIR/$SSL_CFG
    DN=`hostname`;
    if [ -z "$DN" ] || [ "$DN" = "(none)" ]; then
            DN='localhost.localdomain';
    fi;
    FQDN=`{ host -W1 $DN 2>/dev/null || echo "$DN has address "; } |\
            grep 'has address' | head -1 | sed 's/\ .*$//'`;
    if [ -z "$FQDN" ] ; then
        FQDN="$DN";
    fi;
    # cannot use 'hostname --fqdn' because this can hang indefinitely
    # Hack the $CA onto the end of the CN so we differentiate the issuer
    # of the signature from the subject
    echo "CN                     = $FQDN$CA"  >> $PEGASUS_CONFIG_DIR/$SSL_CFG

    # Add x509v3 extensions
    echo "[ v3_req ]" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "basicConstraints       = CA:FALSE" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "[ v3_ca ]" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "subjectKeyIdentifier=hash" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "authorityKeyIdentifier=keyid:always,issuer" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
    echo "basicConstraints = CA:TRUE" >> $PEGASUS_CONFIG_DIR/$SSL_CFG
}

cnfChanged=0;
if [ ! -e $PEGASUS_CONFIG_DIR/ssl-ca.cnf ] ||
   [ ! -e $PEGASUS_CONFIG_DIR/ssl-service.cnf ] ||
   [ ! -e $PEGASUS_CONFIG_DIR/server.pem ] ||
   [ ! -e $PEGASUS_CONFIG_DIR/file.pem ]  ||
   [ ! -e $PEGASUS_CONFIG_DIR/client.pem ]; then

    mkdir -p ${PEGASUS_INSTALL_LOG%/*}
    mkdir -p $PEGASUS_CONFIG_DIR

    create_ssl_cnf ssl-ca.cnf CA
    create_ssl_cnf ssl-service.cnf

    chmod 400 $PEGASUS_CONFIG_DIR/ssl-*.cnf
    chown root $PEGASUS_CONFIG_DIR/ssl-*.cnf
    chgrp root $PEGASUS_CONFIG_DIR/ssl-*.cnf
    cnfChanged=1;
fi
if [ $cnfChanged -eq 1 ] || \
         [ ! -e $PEGASUS_PEM_DIR/$PEGASUS_SSL_CERT_FILE ] || \
         [ ! -e $PEGASUS_PEM_DIR/$PEGASUS_SSL_KEY_FILE ]; then

    # Restrict access of the key to root
    OLDUMASK=`umask`
    umask 0077

    # Create private key for the CA certificate
    TMPKEY=`mktemp --tmpdir=$PEGASUS_PEM_DIR XXXXXXXXXXXX`

    /usr/bin/openssl genrsa -out $TMPKEY 2048

    # Restore the umask for the other files
    umask $OLDUMASK

    # Create CA certificate:
    /usr/bin/openssl req -new -x509 -days 3650 \
                         -config $PEGASUS_CONFIG_DIR/ssl-ca.cnf \
                         -key $TMPKEY \
                         -out $PEGASUS_PEM_DIR/ca.crt \

    # Create private key for the service certificate
    /usr/bin/openssl genrsa -out $PEGASUS_PEM_DIR/$PEGASUS_SSL_KEY_FILE 2048

    # Create a signing request for the service certificate
    /usr/bin/openssl req -new \
                         -config $PEGASUS_CONFIG_DIR/ssl-service.cnf \
                         -key $PEGASUS_PEM_DIR/$PEGASUS_SSL_KEY_FILE \
                         -out $PEGASUS_PEM_DIR/server.csr

    # Sign the request with the CA certificate
    /usr/bin/openssl x509 -req -days 3650 \
                          -in $PEGASUS_PEM_DIR/server.csr \
                          -CA $PEGASUS_PEM_DIR/ca.crt \
                          -CAkey $TMPKEY \
                          -CAcreateserial \
                          -out $PEGASUS_PEM_DIR/$PEGASUS_SSL_CERT_FILE \
                          -extfile $PEGASUS_CONFIG_DIR/ssl-ca.cnf

    # Set file permissions appropriately
    chmod 400 $PEGASUS_PEM_DIR/$PEGASUS_SSL_KEY_FILE
    chmod 444 $PEGASUS_PEM_DIR/$PEGASUS_SSL_CERT_FILE

    # Remove the certificate signing request
    # It is not needed after the signature is complete
    rm -f $PEGASUS_PEM_DIR/server.csr

    # Remove the private key for the CA certificate
    # This will ensure that it cannot be used to sign any other
    # (possibly suspicious) certificates
    # This does mean that generating a new certificate for this
    # service will need a new CA cert, but most real deployments
    # will use real infrastructure.
    # This does not impart perfect security; there is a fairly
    # long race here between the key generation and its deletion.
    # The random filename should significantly mitigate this.
    rm -f $TMPKEY

fi;
if [ ! -e $PEGASUS_PEM_DIR/$PEGASUS_SSL_TRUSTSTORE ]; then
    cp -fp $PEGASUS_PEM_DIR/ca.crt \
        $PEGASUS_PEM_DIR/$PEGASUS_SSL_TRUSTSTORE
    chmod 444 $PEGASUS_PEM_DIR/$PEGASUS_SSL_TRUSTSTORE;
fi;
