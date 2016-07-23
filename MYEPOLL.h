//============================================================================
// Name        : epoll.cpp
// Author      : Saturday
// Version     : v1.0
// Copyright   : Your copyright notice
// Description : test of epoll in C++
//============================================================================

#ifndef _MYEPOLL_H_
#define _MYEPOLL_H_

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

#include<string.h>

#include<pthread.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#include<sys/epoll.h>

#include<iostream>
#include<stdlib.h>
#include<stdio.h>
//#include<string.h>
#include<string>


using namespace std;

#define MAXSOCKETFD 256
#define LISTENQ 16 //number of message
#define MAXSIZE 512 //size of buffer

#define COMMAND_PORT 21
#define DATA_PORT 20

typedef struct{
        unsigned char  packetType;  //协议包类型
        unsigned int length;         //协议包长度
}Header;
typedef  struct  {
        Header head;
        char content[512];          //包内容，最大512byte
}ECOMPROTOCOL;

class MYEPOLL {

public:
	MYEPOLL();
	~MYEPOLL();


	void Init(int size = MAXSOCKETFD,unsigned int port = COMMAND_PORT);

	void WaitToDeal();

	virtual void mSend(struct epoll_event arg);

	virtual void mRecv(struct epoll_event arg);
	
//	void* myfunc(void *para);

	struct epoll_event ev;
	int epfd;
	int listenfd;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	int nfds;//number of message
	struct epoll_event events[32];

	int temp_clientfd;
	struct sockaddr_in tmp_client_addr;

protected:

private:
};

#endif
