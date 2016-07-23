
#include "MYEPOLL.h"

void* myfunc(void *para);

MYEPOLL::MYEPOLL()
{
}

MYEPOLL::~MYEPOLL()
{
}

void MYEPOLL::Init(int size,unsigned int port)
{

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&server_addr,0,sizeof(struct sockaddr_in));

	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	//char *local_addr="127.0.0.1";
	//inet_aton(local_addr,&(server_addr.sin_addr));

	//htons(portnumber);
	server_addr.sin_port=htons(port);

	if(bind(listenfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr_in))==-1)
	{
		cout <<"fail to bind"<< endl;
		return;
	}
	if(listen(listenfd,LISTENQ)==-1)
	{
        cout <<"fail to linten"<< endl;
        return;
	}

	epfd=epoll_create(size);

	ev.data.fd=listenfd;
	ev.events=EPOLLIN | EPOLLET;

	epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

	cout << "Init successfuly and listening now..." << endl; // prints !!!Hello World!!!

}

void MYEPOLL::WaitToDeal()
{
	uint sockaddr_size=sizeof(struct sockaddr_in);
	/********************************************************/
        for (;;)
          {
            nfds = epoll_wait(epfd, events, 32, 500);
            for (int i = 0; i < nfds; ++i)
              {
                if (events[i].data.fd == listenfd) //有新的连接
                  {
                    int connfd = accept(listenfd, (struct sockaddr *) &client_addr,&sockaddr_size);
                    if (connfd < 0)
                      {
                        cout << "connfd<0" << endl;
                        return ;
                      }
                    //setnonblocking(connfd);
                    char *str = inet_ntoa(client_addr.sin_addr);
                    cout << "accapt a connection from " << str << endl;
                    //设置用于读操作的文件描述符

                    char *result = "220 Welcome CentOS FTP!\r\n";
        			if(::send(connfd,(void *)result,strlen(result),0)==-1)
        			{
        				cout<<"send fail"<<endl;
        			}

                    ev.data.fd = connfd;
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
                  }
               else if (events[i].events & EPOLLIN) //接收到数据，读socket
                  {
            	   	  mRecv(events[i]);
                      //no use EPOLLOUT
//                      int sockfd;
//                      sockfd = events[i].data.fd;
//                      ::send(sockfd,(void *)"i am server",sizeof("i am server"),0);
/*
 *   epoll使用的资料网上一大把，EPOLLIN(读)监听事件的类型，大家一般使用起来一般没有什么疑问，
 *   无非是监听某个端口，一旦客户端连接有数据发送，它马上通知服务端有数据，一般用一个回调的读函数，
 *   从这个相关的socket接口读取数据就行了。但是有关EPOLLOUT(写)监听的使用，网上的资料却讲得不够明白，
 *   理解起来有点麻烦。因为监听一般都是被动操作，客户端有数据上来需要读写(被动的读操作，
 *   EPOLIN监听事件很好理解，但是服务器给客户发送数据是个主动的操作，写操作如何监听呢？

  如果将客户端的socket接口都设置成 EPOLLIN | EPOLLOUT(读，写)两个操作都设置，
  那么这个写操作会一直监听，有点影响效率。经过查阅大量资料，我终于明白了EPOLLOUT(写)监听的使用场，
  一般说明主要有以下三种使用场景:

  1： 对客户端socket只使用EPOLLIN(读)监听，不监听EPOLLOUT(写)，写操作一般使用socket的send操作

  2：客户端的socket初始化为EPOLLIN(读)监听，有数据需要发送时，对客户端的socket修改为EPOLLOUT(写)操作，
  这时EPOLL机制会回调发送数据的函数，发送完数据之后，再将客户端的socket修改为EPOLL(读)监听

 3：对客户端socket使用EPOLLIN 和 EPOLLOUT两种操作，这样每一轮epoll_wait循环都会回调读，写函数，
 这种方式效率不是很好
 */

/*
                          ev.data.fd=events[i].data.fd;
                          //设置用于注测的写操作事件

                          ev.events=EPOLLOUT | EPOLLET;
                          //修改sockfd上要处理的事件为EPOLLOUT

                          epoll_ctl(epfd,EPOLL_CTL_MOD,events[i].data.fd,&ev);

*/
                  }
                  else if (events[i].events & EPOLLOUT) // 如果有数据发送
                    {
                	  mSend(events[i]);
                	  /*
                      int sockfd;
                      sockfd = events[i].data.fd;
                      send(sockfd,(void *)"i am server",sizeof("i am server"),0);
                      //设置用于读操作的文件描述符

                      ev.data.fd = sockfd;
                      //设置用于注测的读操作事件

                      ev.events = EPOLLIN | EPOLLET;
                      //修改sockfd上要处理的事件为EPOLIN

                      epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                	   */
                    }
              }
          }
        close(listenfd);
}

void MYEPOLL::mSend(struct epoll_event arg)
{

}

void MYEPOLL::mRecv(struct epoll_event arg)
{
  if(n<sizeof(Header))
    {
      if ((n = recv(sockfd, (char *)(&thread_Package.head+n), sizeof(Header)-n, 0)) < 0)
        {

            if (errno == ECONNRESET)
            {
                close(sockfd);
                pthread_exit(NULL);
                return NULL;
            }
            else
            {
                std::cout << "readbuffer error" << std::endl;
                pthread_exit(NULL);
                return NULL;
            }
        }
        else if (n == 0)
        {
            cout << "NO DTAT TO READ,may be Client close normaly 2" << endl;
            close(sockfd);
            pthread_exit(NULL);
            return NULL;
        }
    }

  std::cout << "read head : "<<n<< std::endl;
  std::cout << "read packetType : "<<thread_Package.head.packetType<< std::endl;
  std::cout << "read length : "<<thread_Package.head.length<< std::endl;
	return;
}