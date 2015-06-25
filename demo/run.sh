#!/bin/bash

set -i

TCP_URL='tcp://mqtt.evrythng.com:1883'
SSL_URL='ssl://mqtt.evrythng.com:443'
THNG_ID='UfFcGftssBpwrSQ8bmT7Ammr'
API_KEY='HiX0xYZwULxR0GBWb9ZuQi8vTcPSndRxfnx9iIvw4u12Bdt6iMxkjwXujCkadQfBfTiV7kGLx80JPdGj'
PROP_NAME='property_1'
CA_PATH='demo/client.pem'

build_debug/demo/evrythng-cli --pub -u ${SSL_URL} -t ${THNG_ID} -k ${API_KEY} -n ${PROP_NAME} -c ${CA_PATH}
