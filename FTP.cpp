/*
 * FTP.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: root
 */

#include "FTP.h"

#include<dirent.h>
FTP::FTP()
{
	// TODO Auto-generated constructor stub

}

FTP::~FTP()
{
	// TODO Auto-generated destructor stub
}

void FTP::init()
{
	if((serverfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		cout<<"fail to create socket"<<endl;
		return;
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(21);

	if(bind(serverfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr_in))==-1)
	{
		cout<<"fail to bind"<<endl;
		return;
	}
	if(listen(serverfd,8)==-1)
	{
        cout <<"fail to linten"<< endl;
        return;
	}

	struct sockaddr_in client_addr;
	uint sockaddr_size=sizeof(struct sockaddr_in);
	int connfd = accept(serverfd, (struct sockaddr *) &client_addr,&sockaddr_size);
    if (connfd < 0)
      {
        cout << "connfd<0" << endl;
        return ;
      }
    //setnonblocking(connfd);
    char *str = inet_ntoa(client_addr.sin_addr);
    cout << "accapt a connection from " << str << endl;

}

void FTP::command(struct epoll_event arg)
{
	char buffer[32];
	int n=0;
	if ((n = recv(arg.data.fd, (void *)&buffer, sizeof(buffer), 0)) < 0)
	{
//		cout<<buffer<<endl;
		cout<<"Unnormal close"<<endl;
		close(arg.data.fd);
		return;
	}
	else if(n==0)
	{
		cout<<"Normal close"<<endl;
		close(arg.data.fd);
		return;
	}
	else if(n>0)
	{
		buffer[n]='\0';
		cout<<buffer<<endl;

		if(strncmp("USER",buffer,4)==0)
		{

			char *result = "331 Please Input Password!\r\n";
			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}
		}
		else if(strncmp("PASS",buffer,4)==0)
		{
			char *result = "230 Login Success!\r\n";
			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}
		}
		else if(strncmp("pasv",buffer,4)==0)
		{
			char *result = "227 Entering Passive Mode! (192,168,56,101,4,20)\r\n";
			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}
		}
		else if(strncmp("PORT",buffer,4)==0)
		{


			temp_clientfd=socket(AF_INET,SOCK_STREAM,0);

			tmp_client_addr.sin_family=AF_INET;

			string *str=new string(buffer);
			int first=str->find_last_of(',',strlen(buffer));
			int second=str->find_last_of(',',first-1);
			char temp[32];
			str->copy(temp,strlen(buffer)-first,first+1);
			temp[strlen(buffer)-first]='\0';
			int port=atoi(temp);
			cout <<port<< endl;
			str->copy(temp,first-second-1,second+1);
			temp[first-second-1]='\0';
			port=port+atoi(temp)*256;
			cout <<port<< endl;
			tmp_client_addr.sin_port=htons(port);

			int third=string(buffer).find_first_of(' ',0);
			str->copy(temp,second-third-1,third+1);
			temp[second-third-1]='\0';
			for(int i=0;temp[i]!='\0';i++)
			{
				if(temp[i]==',')
				{
					temp[i]='.';
				}

			}
			cout <<temp<< endl;
			//char *local_addr="127.0.0.1";
			inet_aton(temp,&(tmp_client_addr.sin_addr));
//			tmp_client_addr.sin_addr.s_addr=htonl(INADDR_ANY);

			delete str;
			char *result = "200 PORT!\r\n";
//			int clientfd;
//			struct sockaddr_in client_addr;


			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}

		}
		else if(strncmp("RETR",buffer,4)==0)
		{
			if((connect(temp_clientfd,(struct sockaddr *)(&tmp_client_addr),sizeof(struct sockaddr_in)))==-1)
			{
				cout << "connnect fail" << endl;
				return;
			}
			else
			{
				cout << "connnect success" << endl;
			}

			char *result = "150 connnect OK!\r\n";

			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}


			FILE *fp;
			fp=fopen("/home/T.txt","r");
			if (fp == NULL)
			{
				cout<<"open fail"<<endl;
			}
			else
			{
				cout<<"open success"<<endl;
			}
			char ch;
			do
			{
				ch=fgetc(fp);
				cout<<ch<<endl;
				if (::send(temp_clientfd, (void *)&ch, sizeof(ch), 0) == -1)
				{
					cout << "send fail" << endl;
				}
				else
				{
					cout << "data send success" << endl;
				}
			}while(ch!=EOF);

			close(temp_clientfd);

			result = "226 send file OK!\r\n";

			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}


		}
		else if(strncmp("LIST",buffer,4)==0)
		{
			if((connect(temp_clientfd,(struct sockaddr *)(&tmp_client_addr),sizeof(struct sockaddr_in)))==-1)
			{
				cout << "connnect fail" << endl;
				return;
			}
			else
			{
				cout << "connnect success" << endl;
			}

			char *result = "150 connnect OK!\r\n";

			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}

			DIR *dir;
			struct dirent *direntp;
			if((dir=opendir("."))==NULL)
			{
				cout<<"open fail"<<endl;
			}
			else
			{
				cout<<"open success"<<endl;
			}
			direntp=readdir(dir);
			do
			{
				int x=strlen(direntp->d_name);
				direntp->d_name[x]='\n';
				x++;
				direntp->d_name[x]='\0';
				if (::send(temp_clientfd, (void *)&(direntp->d_name), strlen(direntp->d_name), 0) == -1)
				{
					cout << "send fail" << endl;
				}
				else
				{
					cout << "data send success" << endl;
				}
				direntp=readdir(dir);
			}while(direntp!=NULL);


			close(temp_clientfd);
			closedir(dir);
			result = "226 send file OK!\r\n";

			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}
		}
		else if(strncmp("QUIT",buffer,4)==0)
		{
			char *result = "200 Close Connect\r\n";
			if (::send(arg.data.fd, (void *) result, strlen(result), 0) == -1)
			{
				cout << "send fail" << endl;
			}
			close(arg.data.fd);
			cout<<"Normal Close"<<endl;
			return;
		}
		else
		{

		}
	}
	return;
}
int FTP::data_recv()
{

	return 0;
}
int FTP::data_send()
{

	return 0;
}
