#!/bin/bash

set -i

exit_script ()
{
    echo "please provide 'pub' or 'sub' parameter only"
    exit
}

if [[ $# -ne 1 || ( "$1" != "pub"  &&  "$1" != "sub") ]] 
then
    exit_script
fi

source Config

CA_PATH='client.pem'

build_debug/demo/evrythng-cli --$1 -u ${MQTT_URL} -t ${THNG_1} -k ${DEVICE_API_KEY} -n ${PROPERTY_1} -c ${CA_PATH}
