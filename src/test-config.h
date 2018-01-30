/*==============================================================
*File Name   : config.h 
*Description : 定义配置文件 如平台连接需要认证信息
*            : 
*            : 
*Author      : guoweikang
*Version     : 1.0
*Copyrirht   : 中移物联网集成电路创新中心
==============================================================*/



#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H



//ONE net  mqtt 服务 IP 
#define ONE_NET_MQTT_IP            "183.230.40.39"
//ONE net  mqtt 服务 PORT
#define ONE_NET_MQTT_PORT          6002
//产品ID
#define ONE_NET_PROJECT_ID         "118577"
//设备ID
#define ONE_NET_SENSOR_DEVICE_ID   "24911005"
#define ONE_NET_CONTROL_DEVICE_ID  "24911006"


//产品API key
#define ONE_NET_AUTH_INFO          "3TrCjKDwwLqkK678MXtgqkmYEUM="

//接收缓冲区大小   设置1M 
#define RECIVE_BUFF_SIZE           1024*1024
//发送消息的默认qos 目前默认只实现了1 
#define PUBLISH_QOS_LEVEL_DEFAULT          1
// 默认 ping 消息保活时间周期 单位秒
#define PING_TIME                          20 

//默认数据上传时间周期        
#define PUBLISH_TIME                          20 

#endif
