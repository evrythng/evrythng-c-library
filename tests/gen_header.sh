#!/bin/bash

CONFIG_FILE=Config
HEADER_FILE=tests/evrythng_config.h


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
