
set(SRC
    ${SRC}
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTClient-C/src/MQTTClient.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTConnectClient.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTConnectServer.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTDeserializePublish.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTFormat.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTPacket.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTSerializePublish.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTSubscribeClient.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTSubscribeServer.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTUnsubscribeClient.c
    ${PROJECT_SOURCE_DIR}/embedded-mqtt/MQTTPacket/src/MQTTUnsubscribeServer.c
    ${PROJECT_SOURCE_DIR}/platforms/POSIX/posix.c
)
