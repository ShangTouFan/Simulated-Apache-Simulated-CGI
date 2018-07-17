#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

#define GET_SIZE 8192
#define MAX_SIZE 1024
#define PATH_LEN 128
#define HEAD_BUFF 256
#define DATA_BUFF 512

int Get_Sockfd();
void *Pthread_Fun(void *arg);
void Send_Error(int ,char *);
void Get_Event(int,char **);
void Post_Event(int,char **);

int Get_Sockfd()
{
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in ser;
	memset(&ser,0,sizeof(ser));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(80);
	ser.sin_addr.s_addr = inet_addr("127.0.0.1");

	int res = bind(sockfd,(struct sockaddr*)&ser,sizeof(ser));
	if(res==-1)
		sockfd = -1;

	listen(sockfd,5);
	return sockfd;
}

void *Pthread_Fun(void *arg)
{
	int c = (int)arg;
	if( c == -1)
	{
		printf("Error:Bad Request\n");
		return ;
	}
	char buff[GET_SIZE]={0};
	int check = recv(c,buff,GET_SIZE-1,0);
	if(check <= 0)
		Send_Error(c,"400 Error");
	printf("%s\n",buff);
	char *ptr = NULL ;
	char *s = strtok_r(buff," ",&ptr);
	if(s==NULL)
		Send_Error(c,"400 Error");
	if(strcmp(s,"GET") == 0)
		Get_Event(c,&ptr);
	else if(strcmp(s,"POST") == 0)
		Post_Event(c,&ptr);
	else
		Send_Error(c,"400 Error");
	close(c);
}

void Get_Event(int c,char **ptr)//处理浏览器发过来的GET报文
{
	if( c < 0 || ptr == NULL || *ptr == NULL)
	{
		printf("Error:Bad Request\n");
		return ;
	}

	char path[PATH_LEN] = "/home/A386/Desktop/STF/http";
	char *s = strtok_r(NULL," ",ptr);
	if( strcmp(s,"/") == 0 )
		s = "/index.html";
	strcat(path,s);
	printf("%s\n",path);
	int fd = open(path,O_RDONLY);
	int file_size = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	char head_buff[HEAD_BUFF] = {0};
	strcpy(head_buff,"HTTP/1.1 200 ok\r\n");
	strcat(head_buff,"Server: http\r\n");
	sprintf(head_buff+strlen(head_buff),"Content-Length:%d\r\n",file_size);
	strcat(head_buff,"\r\n");
	send(c,head_buff,strlen(head_buff),0);//回复报头
	while( sendfile(c,fd,NULL,file_size) == -1 )//发送GET报文中请求的目标文件
		perror("sendfile error");
}

void Post_Event(int c,char **ptr)//处理POST报文,利用管道，exec动态打印页面
{
	if( c < 0 || ptr == NULL || *ptr == NULL)
	{
		printf("Error:Bad Request\n");
		return ;
	}
	char *s = strtok_r(NULL," ",ptr);
	if(s==NULL)
	{
		Send_Error(c,"500 Error\n");
		return ;
	}
	char path[PATH_LEN] = "/home/A386/Desktop/STF/http";
	strcat(path,s);
	char *p = strrchr(*ptr,'\n')+1;
	if(p==NULL)
	{
		Send_Error(c,"500 Error\n");
		return ;
	}
	int pipefd[2];
	pipe(pipefd);
	pid_t pid = fork();
	if(pid==-1)
	{
		Send_Error(c,"500 Error\n");
		return;
	}
	if(pid==0)
	{
		close(pipefd[0]);
		dup2(pipefd[1],1);
		dup3(pipefd[1],2);
		execl(path,path,p);
	}
	close(pipefd[1]);
	wait(NULL);
	char pipe_buff[MAX_SIZE] ={0};
	int file_size ;
	int i = 0 ;
	int sum_size = 0 ;
	while((file_size = read(pipefd[0],pipe_buff,MAX_SIZE)) > 0 )
	{
		sum_size+=file_size ;
		send(c,pipe_buff,file_size,0);
		memset(pipe_buff,0,MAX_SIZE);
	}
	close(pipefd[0]);
}

void Send_Error(int c,char *Errormsg)
{
	send(c,Errormsg,strlen(Errormsg),0);
}

int main()
{
	int sockfd = Get_Sockfd();
	if(sockfd == -1)
	{
		perror("sockfd");
		exit(0);
	}

	while(1)
	{
		struct sockaddr_in wc;
		int len = sizeof(wc);
		int c = accept(sockfd,(struct sockaddr*)&wc,&len);
		pthread_t pd ;
		pthread_create(&pd,NULL,Pthread_Fun,(void*)c);
	}

	exit(0);
}
