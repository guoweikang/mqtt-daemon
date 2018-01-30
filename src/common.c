/*==============================================================
*File Name   : common.h
*Description : 传感器和控制器公用函数实现
*            : 
*            : 
*Author      : guoweikang
*Version     : 1.0
*Copyrirht   : 中移物联网集成电路创新中心
==============================================================*/

#include "common.h"
#include "mqtt/mqtt.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <sys/timerfd.h>

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>



#include <stdint.h>





/*****************************分割线    内部代码使用 用户无需关心******************************************************/







/*--------------------------------------------------------------
*Function Name : MqttTest_SendPkt
*Description   : 内部使用 发送报文接口
*              : 
*              : 
*Input         : ctx: 测试例环境上下文
*              : 
*              : 
*Output        : 
*              : 
*              : 
*Return        : 成功 返回发送的字节数             失败 返回错误值  
*              : 
*----------------------------------------------------------------*/

int MqttTest_SendPkt_All(struct MqttTestContext *ctx) {

	int bytes;
	//USE_SDK： 调用mqtt SDK 接口  内部会调用
	bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);

	if(bytes < 0) {
        printf("Failed to send the packet to the server.\n");
        return bytes;
    }
	//如果一次没有发送完成 修改事件 增加可写事件
	else if(bytes != ctx->mqttbuf->buffered_bytes) {
		   struct epoll_event evt[1];
		   ctx->sendedbytes = bytes;
		   printf("There are some data not sended(%d bytes).\n",
				  ctx->mqttbuf->buffered_bytes - bytes);
	
		   evt->data.fd = ctx->mqttfd;
		   evt->events = EPOLLIN | EPOLLOUT | EPOLLONESHOT | EPOLLET;
		   epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, ctx->mqttfd, evt);
		   return 0;
	 }
   //USE_SDK： 发送成功的话 重置 发送缓冲区  调用了mqttsdk接口
	MqttBuffer_Reset(ctx->mqttbuf);
	return bytes;
}




/*****************************分割线 外部接口          初始化相关******************************************************/


int MqttTest_Init(struct MqttTestContext *ctx,uint32_t recvBufSize)
{
	int err;
	ctx->sendedbytes = -1;
	ctx->mqttfd = -1;
	ctx->pingTfd = -1;
	ctx->cmdid[0] = '\0';
	//USE_SDK： 掉用SDK接口初始化 mqtt 上下文环境 初始化接收缓冲区
	err = Mqtt_InitContext(ctx->mqttctx, recvBufSize);
	if(MQTTERR_NOERROR != err) {
		printf("Failed to init MQTT context errcode is %d", err);
		return -1;
	}
	//USE_SDK： 掉用SDK接口初始化 mqtt 发送缓冲区
	MqttBuffer_Init(ctx->mqttbuf);
	//初始化epoll 控制文件描述符
	ctx->epfd = epoll_create(10);
	if(ctx->epfd < 0) {
		printf("Failed to create the epoll instance.\n");
		return -1;
	}
	return 0;

}



/*****************************分割线 公用的钩子函数******************************************************/


int MqttTest_RecvPkt(void *arg, void *buf, uint32_t count)
{
    int bytes = read((int)(size_t)arg, buf, count);
    return bytes;
}



int MqttTest_SendPkt(void *arg, const struct iovec *iov, int iovcnt)
{
    int bytes;
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (struct iovec*)iov;
    msg.msg_iovlen = (size_t)iovcnt;

    int i=0,j=0;
    printf("send one pkt\n");
    for(i=0; i<iovcnt; ++i){
        char *pkg = (char*)iov[i].iov_base;
        if(iov[i].iov_len > 1024){
            printf("length:%d\n", (int)iov[i].iov_len);
        }else{
            for(j=0; j<iov[i].iov_len; ++j)
                printf("%02X ", pkg[j]&0xFF);
            printf("\n");
        }
    }
    printf("send over\n");
    bytes = sendmsg((int)(size_t)arg, &msg, 0);
    return bytes;
}

/*****************************分割线    建立连接相关******************************************************/


/*--------------------------------------------------------------
*Function Name : MqttTest_CreateTcpConnect
*Description   : 内部使用     创建TCP连接
*              : 
*              : 
*Input         : host: TCP 连接IP   
*              : port：TCP 连接端口
*              : 
*Output        : 
*              : 
*              : 
*Return        : 
*              : 
*--------------------------------------------------------------*/

static int MqttTest_CreateTcpConnect(const char *host, unsigned short port)
{
    struct sockaddr_in add;
    int fd;
    struct hostent *server;

    bzero(&add, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(port);
    server = gethostbyname(host);
    if(NULL == server) {
        printf("Failed to get the ip of the host(%s).\n", host);
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        printf("Failed to create socket file descriptor.\n");
        return fd;
    }

    bcopy((char*)server->h_addr, (char*)&add.sin_addr.s_addr, server->h_length);
    if(-1 == connect(fd, (struct sockaddr*)&add, sizeof(add))) {
        printf("Failed to connect to the server.\n");
        close(fd);
        return -1;
    }

    return fd;
}



int MqttTest_Connect(struct MqttTestContext *ctx){
	int err, flags, bytes;
	struct epoll_event event;

	//首先先检查连接描述符是否被占用 从epoll事件内删除
	   if(ctx->mqttfd >= 0) {
		   close(ctx->mqttfd);
		   epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, ctx->mqttfd, NULL);
	   }
	//创建TCP 连接
	   ctx->mqttfd = MqttTest_CreateTcpConnect(ctx->host, ctx->port);
	   if(ctx->mqttfd < 0) {
		   return -1;
	   }
	//重置mqtt 上下文中的读写参数为 socket fd
	   ctx->mqttctx->read_func_arg = (void*)(size_t)ctx->mqttfd;
	   ctx->mqttctx->writev_func_arg = (void*)(size_t)ctx->mqttfd;

	//设置socket fd 为非阻塞 并且添加 可读事件 到epoll 事件列表 
	   flags = fcntl(ctx->mqttfd, F_GETFL, 0);
	   if(-1 == flags) {
		   printf("Failed to get the socket file flags, errcode is %d.\n", errno);
	   }
	   
	   if(fcntl(ctx->mqttfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		   printf("Failed to set the socket to nonblock mode, errcode is %d.\n", errno);
		   return -1;
	   }
	
	   event.data.fd = ctx->mqttfd;
	   event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
	   if(epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->mqttfd, &event) < 0) {
		   printf("Failed to add the socket to the epoll, errcode is %d.\n", errno);
		   return -1;
	   }

	  //构建Connect 报文  使用 mqtt SDK 接口
	   int keep_alive = ctx->keepalive;
	   printf("dev id:%s\n", ctx->devid );
	   printf("project id:%s\n",ctx->proid);
	   printf("auth info:%s\n", ctx->apikey);
	   //USE_SDK： 构建连接报文 
	   err = Mqtt_PackConnectPkt(ctx->mqttbuf, keep_alive, ctx->devid,
								 IS_CLEAN_SESSION, NULL,
								 NULL, 0,
								 MQTT_QOS_LEVEL0, 0, ctx->proid,
								 ctx->apikey, strlen(ctx->apikey));
	   if(MQTTERR_NOERROR != err) {
		   printf("Failed to pack the MQTT CONNECT PACKET, errcode is %d.\n", err);
		   return err;
	   }
	  //发送连接报文  需要注意 如果发送未完成 需要捕捉可写信号 并进行处理
	   bytes = MqttTest_SendPkt_All(ctx);
	   if( bytes < 0) {
			return bytes;
	   }
	   return 0;
}


int MqttTest_DisConnect(struct MqttTestContext *ctx){

    int err,bytes;
    //USE_SDK：构建disconnect 报文   具体参考协议
    err = Mqtt_PackDisconnectPkt(ctx->mqttbuf);
    if(MQTTERR_NOERROR != err) {
	    printf("Critical bug: failed to pack the disconnect packet.\n");
	    return -1;
    }
    //发送报文 如果发送未完成 需要捕捉可写信号 并进行处理
    //发送连接报文  需要注意 如果发送未完成 需要捕捉可写信号 并进行处理
    bytes = MqttTest_SendPkt_All(ctx);
    if(bytes < 0  ) {
		//返回错误码
		return bytes;
    }
	
//如果disconnect 报文没有完全写到缓冲区 需要在等下一次  
	if(ctx->sendedbytes != -1){
		ctx->disconnect = 1 ;
	}else{
		//直接退出
		ctx->exit=1;
	}
	return 0;
}





int MqttTest_PingReq(struct MqttTestContext *ctx){

	int err,bytes;
	//构造ping 报文
	err = Mqtt_PackPingReqPkt(ctx->mqttbuf);
	if(MQTTERR_NOERROR != err) {
		printf("Critical bug: failed to pack the ping request packet.\n");
		return err;
	}
	
	//发送Ping 报文
	bytes = MqttTest_SendPkt_All(ctx);
	 if(bytes < 0  ) {
		//返回错误码
		return bytes;
    }
	return 0;

}





void MqttTest_Destroy(struct MqttTestContext *ctx){
	   //USE_SDK  清除发送缓冲区
		MqttBuffer_Destroy(ctx->mqttbuf);
		//USE_SDK  清除mqtt 上下文 	 释放 mqtt 中的接受缓冲区
		Mqtt_DestroyContext(ctx->mqttctx);
	
		 //清除上下文中用到的 epoll 文件描述符
		if(ctx->epfd >= 0) {
		   close(ctx->epfd);
		   ctx->epfd = -1;
	   }
	   //关闭socket fd
	
	   if(ctx->mqttfd >= 0) {
		   close(ctx->mqttfd);
		   ctx->mqttfd = -1;
	   }


}




 int MqttTest_HandleSocket(struct MqttTestContext *ctx, uint32_t events)
{
	//重新构建可读事件
    struct epoll_event evt[1];
    evt->data.fd = ctx->mqttfd;
    evt->events = EPOLLIN;

    //可读事件 说明有报文返回
    if(events & EPOLLIN) {
        while(1) {
            int err;
			//进行报文接收处理  内部会根据接收报文类型 进行不同的分发处理
            err = Mqtt_RecvPkt(ctx->mqttctx);
            if(MQTTERR_ENDOFFILE == err) {
                printf("The connection is disconnected.\n");
                close(ctx->mqttfd);
                epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, ctx->mqttfd, NULL);
                ctx->mqttfd = -1;
                return 0;
            }

            if(MQTTERR_IO == err) {
                if((EAGAIN == errno) || (EWOULDBLOCK == errno)) {
                    break;
                }

                printf("Send TCP data error: %s.\n", strerror(errno));
                return -1;
            }

            if(MQTTERR_NOERROR != err) {
                printf("Mqtt_RecvPkt error is %d.\n", err);
                return -1;
            }
        }
    }
    //可写事件 表示还有报文没有发送完成
    if(events & EPOLLOUT) {
        if(-1 != ctx->sendedbytes) {
            int bytes = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, ctx->sendedbytes);
            if(bytes < 0) {
                return -1;
            }
            else {
                ctx->sendedbytes += bytes;
                if(ctx->sendedbytes == ctx->mqttbuf->buffered_bytes) {
                    MqttBuffer_Reset(ctx->mqttbuf);
                    ctx->sendedbytes = -1;
					if(ctx->exit == 0 && ctx->disconnect == 1 ) {
						//需要退出 
						ctx->exit =1 ;
					}
                }
				//如果仍然没有发送完成 继续增加可写事件标志
                else {
                    evt->events |= EPOLLOUT;
                }
            }
        }
    }
    //再次激活事件  
    epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, ctx->mqttfd, evt);
    return 0;
}



 
 int MqttTest_CreateTimerFd(struct MqttTestContext *ctx,time_t sec){
	 struct timespec now;
	 struct itimerspec new_value;
	 int tifd = 0;
	 // 得到现在的时间
	  if (clock_gettime(CLOCK_MONOTONIC, &now) == -1){ 
		 printf("get time now error\n");
		 return -1;
	  }
	  tifd= timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	  if (-1 == tifd) {
		 printf("get time now error\n");
		 return -1 ;
	  }
	  //设置时间
	   new_value.it_value.tv_sec = now.tv_sec+sec;	 
	   new_value.it_value.tv_nsec = 0;
	   new_value.it_interval.tv_sec = sec;
	   new_value.it_interval.tv_nsec = 0;
	   if(-1 == timerfd_settime(tifd,TFD_TIMER_ABSTIME,&new_value,NULL)) {
		 printf("set time  error\n");
		 return -1;
	   }
	   
	  return tifd;
 }

