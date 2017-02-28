#!/bin/bash

CONFIG_FILE=$1
HEADER_FILE=$2

if [ "$1" != "" ]; then
    echo "Using test configuration file: ${CONFIG_FILE}"
else
    echo "a path to configuration config was not provided"
    exit
fi

if [ "$2" != "" ]; then
    echo "Generating configuration header: ${HEADER_FILE}"
else
    echo "a path to configuration header was not provided"
    exit
fi

if [ ! -f ${CONFIG_FILE} ]
then
    echo "config file was not found"
    exit
fi

: > ${HEADER_FILE}

echo "#ifndef EVT_CONFIG_H" >> ${HEADER_FILE}
echo "#define EVT_CONFIG_H" >> ${HEADER_FILE}

awk '
BEGIN { 
    FS="=";
    printf("#ifndef EVT_CONFIG_H\n");
    printf("#define EVT_CONFIG_H\n\n");
}
!/^$/ {

    printf("#define %s \"%s\"\n", $1, $2);
}
END {
    printf("\n#endif\n");
}
' ${CONFIG_FILE} > ${HEADER_FILE}
