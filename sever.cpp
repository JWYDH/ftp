//============================================================================
// Name        : sever.cpp
// Author      : YE
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "MYEPOLL.h"
#include "ThreadPool.h"
#include "MYEPOLL.h"
#include "FTP.h"

using namespace std;
int temp_clientfd;
struct sockaddr_in tmp_client_addr;
class comobo : public  MYEPOLL
{

public:
	comobo()
	{
		thp = new ThreadPool();
	}
	~comobo()
	{
		thp->threadpool_destroy(thp->pool);
	}
	void mSend(struct epoll_event arg)
	{
		return;
	}
	void mRecv(struct epoll_event arg)
	{
//		char buffer[32];
//		int n = recv(arg.data.fd, (void *)&buffer, sizeof(buffer), 0);
//
//		cout<<buffer<<endl;

//		pthread_t id;
//		pthread_attr_t attr;
//		pthread_attr_init(&attr);
//
//		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//		int ret=pthread_create(&id,NULL,thread,(void *)&arg);
//		pthread_join(id,NULL);

		fd=arg.data.fd;
		thp->threadpool_add(thp->pool,process,(void *)&fd);

	}
	static void *process(void *arg)
	{
		FTP *ftp = new FTP();
		ftp->temp_clientfd=::temp_clientfd;
		ftp->tmp_client_addr=::tmp_client_addr;
		struct epoll_event var;
		var.data.fd=*(int *)arg;
//		var = (struct epoll_event)(*(struct epoll_event *)arg);
		ftp->command(var);
		::temp_clientfd=ftp->temp_clientfd;
		::tmp_client_addr=ftp->tmp_client_addr;
		delete ftp;
		return NULL;
	}
	static void *thread(void *arg)
	{
		struct epoll_event var;
//		var.data.fd=((struct epoll_event *)arg)->data.fd;
		var = (struct epoll_event)(*(struct epoll_event *)arg);

		FTP *ftp = new FTP();
		ftp->command(var);
		delete ftp;
//		pthread_exit(NULL);
//				char buffer[32];
//				int n = recv(var.data.fd, (void *)&buffer, sizeof(buffer), 0);
//
//				cout<<buffer<<endl;
//		cout<<"this is pthread"<<pthread_self()<<endl;
//	//	pthread_exit(NULL);
//		cout<<"this is pthread"<<pthread_self()<<endl;
		return NULL;
	}

	ThreadPool *thp;
	int fd;

};

int main()
{
	comobo *epoll = new comobo();
	epoll->Init();
	epoll->WaitToDeal();

	return 0;
}
