/*==============================================================
*File Name   : sensor.c 
*Description : 传感器程序定义
*            : 
*            : 
*Author      : guoweikang
*Version     : 1.0
*Copyrirht   : 中移物联网集成电路创新中心
==============================================================*/



#include "common.h"
#include "mqtt/mqtt.h"
#include "test-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>



/*********************************全局变量区***************************************/
struct MqttTestContext sensorctx[1];
int g_pkt_id;

/**********************************************************************************/



/**************************分割线    辅助函数函数*****************************************/


/**************************分割线       相关的钩子函数*****************************************/
//------------------------------- packet handlers -------------------------------------------



# if  0
 //信号会打断 epool 事件 所以我们修改方案 使用 timerfd 


//定时器 信号处理函数
void signal_Ping(int signo) {
	//断开连接
	int ret;
	printf("\n*************** ping **************\n");
    ret = MqttTest_PingReq(sensorctx);

	if ( ret < 0 ) {
		printf("failed ping error code : %d",ret);
	}
	printf("\n*************** ping sucess**************\n");
}
#endif

/*--------------------------------------------------------------
*Function Name : init_time
*Description   : 内部使用 初始化定时器 设定每隔多长时间发送定时信号
*              : 
*              : 
*Input         :sec 秒
*              :
*              : 
*Output        : 
*              : 
*              : 
*Return        : 成功 返回0              失败 返回-1 
*              : 
*----------------------------------------------------------------*/

#if 0 
static int  init_time(int     sec)
 {
 	int ret;
    struct itimerval val;
          
    val.it_value.tv_sec = sec; //1秒后启用定时器
    val.it_value.tv_usec = 0; 

	val.it_interval = val.it_value; //定时器间隔

	//设定定时器
    ret= setitimer(ITIMER_REAL, &val, NULL);
	if (ret <0 ) {
		return -1;
	}
	return 0;
 }
#endif
static int MqttSensor_SetPingTimer(struct MqttTestContext *ctx){

	   //首先先检查连接描述符是否被占用 从epoll事件内删除
	int tfd;
	if(ctx->pingTfd >= 0) {
		close(ctx->pingTfd);
		epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, ctx->pingTfd, NULL);
	}
	
    //创建ping 的timer fd 20 秒PIng 一次 
    tfd = MqttTest_CreateTimerFd(ctx,PING_TIME);
	if (tfd == -1 ){
		return MQTTERR_IO ;
	}
	ctx->pingTfd = tfd;
	//创建一个事件 将timer fd 加入 epoll 中
	struct epoll_event event;
	event.data.fd = ctx->pingTfd;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
	if(epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->pingTfd, &event) < 0) {
		printf("Failed to add the socket to the epoll, errcode is %d.\n", errno);
		return MQTTERR_IO;
	}
	return 0;
}


static int MqttSensor_SetPublishTimer(struct MqttTestContext *ctx){

	   int tfd;

	   //首先先检查连接描述符是否被占用 从epoll事件内删除
	if(ctx->pubTfd >= 0) {
		close(ctx->pubTfd);
		epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, ctx->pubTfd, NULL);
	}
	    //创建publish 的timer fd 60 秒publish 一次 
	tfd = MqttTest_CreateTimerFd(ctx,PUBLISH_TIME);
	if (tfd == -1 ){
		return MQTTERR_IO ;
	}
	ctx->pubTfd = tfd;
	//创建一个事件 将timer fd 加入 epoll 中
	struct epoll_event event;
	event.data.fd = ctx->pubTfd;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
	if(epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->pubTfd, &event) < 0) {
		printf("Failed to add the socket to the epoll, errcode is %d.\n", errno);
		return MQTTERR_IO;
	}
	return 0;
}


/*
处理连接响应的回调函数
flags取值@see MqttConnAckFlag，
ret_code的值为 @see MqttRetCode， 成功则返回非负数
*/

static int MqttSensor_HandleConnAck(void *arg, char flags, char ret_code)
{
	int ret;

    printf("Sensor Success to connect to the server, flags(%0x), code(%d).\n",
           flags, ret_code);
	//如果连接没有成功建立
	if (MQTT_CONNACK_ACCEPTED != ret_code) {
			return MQTTERR_INVALID_PARAMETER;
	}
 	//创建 ping timer 
	struct MqttTestContext *ctx = (struct MqttTestContext *)(arg);
	ret = MqttSensor_SetPingTimer(ctx);
	if (ret < 0 ) {
		return ret;
	}
	//创建 数据上报publish        timerfd 
	ret = MqttSensor_SetPublishTimer(ctx);
	if (ret < 0 ) {
		return ret;
	}
	return 0;
# if  0
 //信号会打断 epool 事件 所以我们修改方案 使用 timerfd 
	//连接成功后使用定时器 每隔20s 发送一次Ping 报文 保证连接正常
	 signal(SIGALRM, signal_Ping);
	 init_time(10);
#endif 
}

static int MqttSensor_HandlePingResp(void *arg)
{
    printf("Recv the ping response.\n");
	printf("\n*************** ping sucess**************\n");
    return 0;
}

static int MqttSensor_HandlePubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish ack, packet id is %d.\n", pkt_id);
	//应该释放相应的报文
	printf("\n*************** publish  sucess**************\n");
    return 0;
}


/*--------------------------------------------------------------
*Function Name :MqttTest_Sensor_Init 
*Description   :内部使用 初始化回掉函数
*              : 
*              : 
*Input         : 
*              : 
*              : 
*Output        : 
*              : 
*              : 
*Return        : 
*              : 
*-----------------------------------------------------------------*/


static int MqttTest_Sensor_Init(struct MqttTestContext *ctx){
		int ret;

		//通用初始化
		ret = MqttTest_Init(ctx,RECIVE_BUFF_SIZE);
		if (-1 == ret ) {
			printf("Failed to Init MqttTestContext.\n");
			return -1;
		}
		ctx->mqttctx->read_func = MqttTest_RecvPkt;
		ctx->mqttctx->read_func_arg =  (void*)(size_t)ctx->mqttfd;
		ctx->mqttctx->writev_func_arg =  (void*)(size_t)ctx->mqttfd;
		ctx->mqttctx->writev_func = MqttTest_SendPkt;
	
		ctx->mqttctx->handle_conn_ack = MqttSensor_HandleConnAck;
	
		ctx->mqttctx->handle_conn_ack_arg = ctx;
 
		ctx->mqttctx->handle_ping_resp = MqttSensor_HandlePingResp;
		ctx->mqttctx->handle_ping_resp_arg = ctx;
		
		ctx->mqttctx->handle_pub_ack = MqttSensor_HandlePubAck;
		ctx->mqttctx->handle_pub_ack_arg = ctx;
#if 0
		ctx->mqttctx->handle_publish = MqttSample_HandlePublish;
		ctx->mqttctx->handle_publish_arg = ctx;

		ctx->mqttctx->handle_pub_rec = MqttSample_HandlePubRec;
		ctx->mqttctx->handle_pub_rec_arg = ctx;
		ctx->mqttctx->handle_pub_rel = MqttSample_HandlePubRel;
		ctx->mqttctx->handle_pub_rel_arg = ctx;
		ctx->mqttctx->handle_pub_comp = MqttSample_HandlePubComp;
		ctx->mqttctx->handle_pub_comp_arg = ctx;

		ctx->mqttctx->handle_sub_ack = MqttSample_HandleSubAck;
		ctx->mqttctx->handle_sub_ack_arg = ctx;
		ctx->mqttctx->handle_unsub_ack = MqttSample_HandleUnsubAck;
		ctx->mqttctx->handle_unsub_ack_arg = ctx;
		ctx->mqttctx->handle_cmd = MqttSample_HandleCmd;
		ctx->mqttctx->handle_cmd_arg = ctx;
	#endif 
		return 0;
}



//ctrl c 信号处理函数
void signal_Stop(int signo) {
	//断开连接
	int ret;
	printf("\n*************** disconnect**************\n");
    ret = MqttTest_DisConnect(sensorctx);

	if ( ret < 0 ) {
		printf("failed disconnect  error code : %d",ret);
	}

}


//构造位置信息
static int MqttTest_CreateLocation(char *locStr){
	 //测试 使用随机数生成 随机位置
	 struct timeval tpstart;
	 gettimeofday(&tpstart,NULL);
	 srand(tpstart.tv_usec);
	 int x = 1+(int) (100.0*rand()/(RAND_MAX+1.0));
	 int y = 1+(int) (100.0*rand()/(RAND_MAX+1.0));
	 int z =  1+(int) (100.0*rand()/(RAND_MAX+1.0));
	 
	 time_t curtime;
	 time(&curtime);
	 struct tm *p = localtime(&curtime);
	 char timeStr[512];
	 sprintf(timeStr,"%d-%d-%d %d:%d:%d",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	 sprintf(locStr,"{ \
	 	\"datastreams\": [ \
	 	{ \"id\":\"location_X\",\"datapoints\":[{\"at\":\"%s\",\"value\": %d}]}, \
	 	{ \"id\":\"location_Y\",\"datapoints\":[{\"at\":\"%s\",\"value\": %d}]}, \
	 	{ \"id\":\"location_Z\",\"datapoints\":[{\"at\":\"%s\",\"value\": %d}]}  \
	 	] \
	 	}",timeStr,x,timeStr,y,timeStr,z);
	 return 0;
}


 static int MqttTest_PublishDataType_1(struct MqttTestContext *ctx,int Qos){
	//上传数据节点 
	//首先，我们采用第一种方式上传
	char locstr[1024];
	MqttTest_CreateLocation(locstr);
	uint32_t size = strlen(locstr);
	int retain = 0;
	int own = 1;
	int err = MQTTERR_NOERROR;
	MqttBuffer_Init(ctx->mqttbuf);
    err = Mqtt_PackDataPointByString(ctx->mqttbuf, g_pkt_id++, 0, kTypeFullJson, locstr, size, Qos, retain, own); 
	return err;
 }

 //上报数据
 int MqttTest_PublishReq(struct MqttTestContext *ctx, int Qos){
 
	 /* 上报数据点，消息支持的格式类型  我们都会选择使用一遍 加深理解*/
	 /*首先获取我们要上传的数据  
	 *第一种数据  我们选择全流模式   选择上传两个流 
     "{\"datastreams\":[{ \"id\":\"temperature\", \"datapoints\":[{\"at\":\"2016-12-22 22:22:22\",\"value\": 36.5}]}]}";
	 *
	 */
	 //对于第一种 我们假设上传两个数据流  分别是本地日志和错误记录
	 if(0==Qos){
 		MqttTest_PublishDataType_1(ctx,MQTT_QOS_LEVEL0);
	 }else if(1==Qos){
		 MqttTest_PublishDataType_1(ctx,MQTT_QOS_LEVEL1);
	 }else if(2 == Qos){
		MqttTest_PublishDataType_1(ctx, MQTT_QOS_LEVEL2);
	 }
	 //发送 报文
	 int bytes = MqttTest_SendPkt_All(ctx);
	 if(bytes < 0  ) {
		//返回错误码
		return bytes;
    }
	 return 0;
 
 }



 int MqttTest_EpollWait(struct MqttTestContext *ctx,int evt_max_cnt)
 {
 //初始化退出标志位 
	ctx->exit = 0;
    ctx->disconnect = 0;
	int evt_cnt;
	struct epoll_event events[evt_max_cnt];
 	while(!(ctx->exit) && (evt_cnt = epoll_wait(ctx->epfd, events, evt_max_cnt, -1)) >= 0) {
		int i;
		//循环抓取 事件 一一进行处理
		for(i = 0; i < evt_cnt; ++i) {
			if(ctx->pingTfd == events[i].data.fd) {
				//如果是ping 的定时器
				printf("\n*************** start  ping **************\n");
				int ret,bytes;
				uint64_t timeOut;
			    bytes = read(ctx->pingTfd, &(timeOut), sizeof(uint64_t)); 
				ctx->exp += timeOut;
				//printf("\n*************** ping  count =  %ld **************\n",ctx->exp);
			 	ret = MqttTest_PingReq(ctx);
	 			if ( ret < 0 ) {
		 			printf("failed ping error code : %d",ret);
	 			}
				
			    events[i].events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, events[i].data.fd, events + i);
			}else if (ctx->pubTfd == events[i].data.fd){
				//如果是数据上报定时器
				//上报数据
				int ret,bytes;
				uint64_t timeOut;
				printf("\n*************** start  publish **************\n");
				bytes = read(ctx->pubTfd, &(timeOut), sizeof(uint64_t)); 
				
				ret = MqttTest_PublishReq(ctx,PUBLISH_QOS_LEVEL_DEFAULT);
				
	 			if ( ret < 0 ) {
		 			printf("failed publish error code : %d",ret);
	 			}
			
				//重新激活
			    events[i].events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, events[i].data.fd, events + i);				
			}else {
				if (MqttTest_HandleSocket(ctx, events[i].events) < 0) {
					ctx->exit = 1;
					break;
				}
			}
		}
	}
	printf("epoll exit!\n");
	return 0 ;
}




int main(void){

	 int evt_max_cnt =2;
	 g_pkt_id = 1;
	 //初始化上下文结构体
	 if(MqttTest_Sensor_Init(sensorctx) < 0) {
        return -1;
    }
	 
	sensorctx->apikey = ONE_NET_AUTH_INFO;
	 //使用sensor id
	sensorctx->devid  = ONE_NET_SENSOR_DEVICE_ID;
	sensorctx->proid = ONE_NET_PROJECT_ID;
	
	sensorctx->host = ONE_NET_MQTT_IP;
	sensorctx->port = ONE_NET_MQTT_PORT;

	sensorctx->keepalive =  120;

	
	//建立连接 
	 if (MqttTest_Connect(sensorctx) < 0) {
		printf("mqtt connect failed.");
	 }
	  //捕捉信号断开连接
	  signal(SIGINT, signal_Stop);
	 //接收挂起 
	 MqttTest_EpollWait(sensorctx,evt_max_cnt);
	

	 //清除善后
	 MqttTest_Destroy(sensorctx);
	 printf("exit sucess.\n");
	 return 1;
}


