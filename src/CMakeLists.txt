



project(MqttTest_c)



set (MQTTSENSOR_SOURCE sensor.c common.c) 
set (MQTTSENSOR_DEPLIBS mqtt rt)

link_directories(${LIBRARY_OUTPUT_PATH})

add_executable(MqttSensor ${MQTTSENSOR_SOURCE}) 

target_link_libraries(MqttSensor ${MQTTSENSOR_DEPLIBS})



