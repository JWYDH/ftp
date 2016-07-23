/*
 * FTP.h
 *
 *  Created on: Nov 3, 2014
 *      Author: root
 */

#ifndef FTP_H_
#define FTP_H_

//linux
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>

#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
//C++
#include<string>
#include<string.h>
#include<iostream>
#include<fstream>
#include<sys/epoll.h>
#include<stdlib.h>

using namespace std;

class FTP {
public:
	FTP();
	virtual ~FTP();
	void init();
	void command(struct epoll_event arg);
	int data_recv();
	int data_send();

	int temp_clientfd;
	struct sockaddr_in tmp_client_addr;
private:
	int serverfd;
	int clientfd;


};

#endif /* FTP_H_ */
