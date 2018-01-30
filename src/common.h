/*==============================================================
*File Name   : common.h
*Description : 传感器和控制器公用结构体和函数接口
*            : 
*            : 
*Author      : guoweikang
*Version     : 1.0
*Copyrirht   : 中移物联网集成电路创新中心
==============================================================*/




#ifndef COMMON_H
#define COMMON_H


#include "mqtt/mqtt.h"
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>


//枚举 connect 连接clean session 标志位
#define IS_CLEAN_SESSION  1
#define IS_NOT_CLEAN_SESSION 0



/*--------------------------------------------------------------
*结构体:MqttTestContext
*功   能:定义测试软件的上下文环境
*包   含 
*int epfd                     ：epoll 控制描述符
*int mqttfd                   ：连接平台的socket 描述符
*uint32_t sendedbytes         : 已发送报文字节数
*struct MqttContext mqttctx[1]：MQTT SDK 使用的上下文 *重要*
*struct MqttBuffer mqttbuf[1] ：MQTT SDK 使用的构建报文缓冲区   *重要*
*const char *host             ：One Net 平台MQTT服务 IP
*unsigned short port          ：One Net 平台MQTT服务端口 
*const char *proid            ：One Net 平台用户注册的产品ID 
*const char *devid            ：One Net 平台用户注册的设备ID  
*const char *apikey           ：One Net 平台用户连接认证信息 可以是产品API key 也可以是设备的认证信息
*int   keepalive              ：One Net 平台连接保活时间 
*int dup                      ：报文dup信息 
*enum MqttQosLevel qos        : mqtt 报文服务质量
*int retain;                  : mqtt报文retain 属性
*uint16_t pkt_to_ack;         : 收到报文回复
*char cmdid[1024];            : 收到的平台命令指令
*int exit                     : 任务退出标志            
*int disconnect               : 连接断开标志位
---------------------------------------------------------------*/

struct MqttTestContext
{
    int epfd;
    int mqttfd;
	int pingTfd;
	int pubTfd;
	uint64_t  exp;
    uint32_t sendedbytes;
    struct MqttContext mqttctx[1];
    struct MqttBuffer mqttbuf[1];

    const char *host;
    unsigned short port;

    const char *proid;    //自己初始化
    const char *devid;    //自己初始化
    const char *apikey;   //自己初始化
    int keepalive;     
    int dup;
    enum MqttQosLevel qos;
    int retain;
    uint16_t pkt_to_ack;
    char cmdid[1024];
	int exit;
	int disconnect;

};

/*--------------------------------------------------------------
*Function Name : MqttTest_Connect
*Description   : 完成mqtt连接服务
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针
*              : 
*              : 
*Output        : 
*              : 
*              : 
*Return        : 成功返回 0 失败返回错误码
*              : 
*----------------------------------------------------------------*/

int MqttTest_Connect(struct MqttTestContext *ctx);


 /*--------------------------------------------------------------
 *Function Name : MqttTest_DisConnect
 *Description	: 完成mqtt断开连接服务
 *				: 
 *				: 
 *Input 		: ctx:测试例的上下文结构体指针
 *				: 
 *				: 
 *Output		: 
 *				: 
 *				: 
 *Return		: 成功返回 0 失败返回错误码
 *				: 
 *----------------------------------------------------------------*/
 
 int MqttTest_DisConnect(struct MqttTestContext *ctx);



 /*--------------------------------------------------------------
 *Function Name : MqttTest_PingReq
 *Description	: 发送ping 请求
 *				: 
 *				: 
 *Input 		: ctx:测试例的上下文结构体指针
 *				: 
 *				: 
 *Output		: 
 *				: 
 *				: 
 *Return		: 成功返回 0 失败返回错误码
 *				: 
 *----------------------------------------------------------------*/

 int MqttTest_PingReq(struct MqttTestContext *ctx);


 /*--------------------------------------------------------------
 *Function Name : MqttTest_RecvPkt
 *Description	: 公用接受报文接口         mqtt SDK Mqtt_RecvPkt 会使用 
 *				: 
 *				: 
 *Input 		: arg：钩子函数外部调用传入           参数   使用  mqttctx->read_func_arg
 *				: buf: 接收缓冲区地址         缓冲区会使用mqtt 上下文环境中申请的 pos指针    缓冲区大小为  RECIVE_BUFF_SIZE
 *				: count:剩余缓冲区大小
 *Output		: 
 *				: 
 *				: 
 *Return		: 返回读取的字节数
 读取数据回调函数，arg为回调函数关联的参数，buf为读入数据
             存放缓冲区，count为buf的字节数，返回读取的数据的字节数，
             如果失败返回-1，读取到文件结尾返回0.
 *---------------------------------------------------------------*/

int MqttTest_RecvPkt(void *arg, void *buf, uint32_t count);






 /*--------------------------------------------------------------
 *Function Name : MqttTest_RecvPkt
 *Description	: 公用发送报文接口         mqtt SDK Mqtt_SendPkt 会使用 
 *				: 
 *				: 
 *Input 		:
 *Output		: 
 *				: 
 *				: 
 *Return		: 返回读取的字节数

  发送数据的回调函数，其行为类似于 unix中的writev，
	   arg是回调函数的关联参数，iovcnt为iov对象的个数，iovec定义如下：
	   struct iovec {
		   void *iov_base;
		   size_t iov_len;
	   }
	   返回发送的字节数，如果失败返回-1.
 *---------------------------------------------------------------*/

extern int MqttTest_SendPkt(void *arg, const struct iovec *iov, int iovcnt);


/*--------------------------------------------------------------
*Function Name : MqttTest_Init
*Description   : 公用初始化上下文接口
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*              : recvBufSize： 接受缓冲区大小
*              : 
*Output        : 
*              : 
*              : 
*Return        : 0 成功    -1 失败
*              : 注意:此函数接口 只会初始化公共内容 对于回掉函数 需要不同的程序初始化调用
*---------------------------------------------------------------*/

int MqttTest_Init(struct MqttTestContext *ctx,uint32_t  recvBufSize);





/*--------------------------------------------------------------
*Function Name : MqttTest_Destroy
*Description   : 公用推出 清楚操作
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*              : 
*              : 
*Output        : 
*              : 
*              : 
*Return        :
*              : 
*---------------------------------------------------------------*/


void MqttTest_Destroy(struct MqttTestContext *ctx);



/*--------------------------------------------------------------
*Function Name : MqttTest_HandleSocket
*Description   : 公用 处理接收到的socket 事件
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*              : events：接收到的事件类型 
*              : 
*Output        : 
*              : 
*              : 
*Return        :
*              : 
*---------------------------------------------------------------*/



int MqttTest_HandleSocket(struct MqttTestContext *ctx, uint32_t events);




/*--------------------------------------------------------------
*Function Name : MqttTest_EpollWait
*Description   : 每次在需要等待socket 响应时 调用此函数
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*                evt_max_cnt:支持epoll监听的最大事件数量
*              : 
*Output        : 
*              : 
*              : 
*Return        :
*              : 
*---------------------------------------------------------------*/




/*--------------------------------------------------------------
*Function Name : MqttTest_CreateTimerFd
*Description   : 每次在需要等待socket 响应时 调用此函数
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*                sec:Ping时间间隔
*              : 
*Output        : 
*              : 
*              : 
*Return        : -1  failed   success 返回创建的fd
*              : 
*---------------------------------------------------------------*/
int MqttTest_CreateTimerFd(struct MqttTestContext *ctx,time_t sec);






/*--------------------------------------------------------------
*Function Name : MqttTest_SendPkt_All
*Description   : 发送报文接口
*              : 
*              : 
*Input         : ctx:测试例的上下文结构体指针   
*                qos:服务质量      0 1 2 
*              : 
*Output        : 
*              : 
*              : 
*Return        : 1  failed  errcode   success 0
*              : 
*---------------------------------------------------------------*/


int MqttTest_SendPkt_All(struct MqttTestContext *ctx) ;

#endif




