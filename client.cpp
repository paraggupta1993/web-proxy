#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h> //read and write
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>

#include<arpa/inet.h> //inet_ntop

#include<pthread.h>

#include<iostream>
#define PAGE "/"
#define PORT 80
#define PROXY "127.0.0.1"
#define PROXY_PORT 5000
#define MX 100
using namespace std;

void error(char* msg){
	perror(msg);
	exit(1);
}
void usage(){
	cout << "Usage: client <Hostname> <filename>" << endl;	
	exit(2);
}
int num = 0;
string comm[1000];
void* request_server( void* args){
	int no = (int) args;
	string command = comm[no];
	//cout << command << endl;

	int sockfd;
	struct sockaddr_in  serv_addr;
	struct hostent* server;

	char buffer[1024];

	int n;
	//address of server
	//create a socket
	sockfd = socket ( AF_INET,SOCK_STREAM,0);

	server = gethostbyname(PROXY);
	int iplen = 16;
	char* ip = (char*) malloc(iplen);
	inet_ntop( AF_INET , (void*) server->h_addr_list[0], ip ,iplen);
	printf("IPADDRESS is :%s\n",ip);



	bzero( (sockaddr_in *)&serv_addr , sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;

	bcopy( (char*)server->h_addr, (char*)& serv_addr.sin_addr.s_addr,server->h_length );
	serv_addr.sin_port = htons(PROXY_PORT);

	if(connect(sockfd,(struct sockaddr*)& serv_addr, sizeof(serv_addr)) < 0){
		error("Cannot connect to server");
	}

	//sprintf( buffer ,"GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n",argv[2] , argv[1] , "MOZILLA/4.0");

	if( command == "Print Cache"){
		cout << "Printing cache" << endl;
		strcpy( buffer , command.c_str());
	}
	else if( command == "Print Log"){

		cout << "Printing logs" << endl;
		strcpy( buffer , command.c_str());
	}
	else if( strncmp(command.c_str(),"Search",6)==0){

		cout << "searching key" << endl;
		strcpy( buffer , command.c_str());
	}
	else{
		size_t pos;
		pos = command.find("http://");
		//cout << "pos" << pos << endl;
		if( pos != -1 ) {
			string host =  command.substr( pos+7 );
			pos = host.find_first_of("/");
			if(pos != -1){
				host.erase( pos );
			}
			//cout << "host:" << host << endl;
			sprintf( buffer ,"GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n",command.c_str() , host.c_str() , "MOZILLA/4.0");
		}
		else cout << "Bad URL. HTTP request cannot be formed in client"  << endl;
	}
	//cout << buffer << endl; 
	n = write( sockfd , buffer , strlen(buffer));
	char buff[1024];
	while(1){
		n = read (sockfd , buff , 1023);
		if( n > 0){ 
			buff[n]='\0';
			printf("%s",buff);
		}
		else break;
	}
	close(sockfd);
	pthread_exit(0);
}
int main(int argc, char** argv){
	int i;
	//if(argc <= 1) usage();

	//char* host = argv[1];
	//char* page = argv[2];
	int port = PORT;
	char command[1000];
	pthread_t threads[MX];

	bool flag = 0;
	while(1){
		memset( command , '\0' , sizeof(command));
		while( strlen(command) <= 0){
			if(scanf("%[^\n]",command)==EOF){ flag=1 ;  break;}
			getchar();
		}
		if( flag ) break;

		//cout << num << " "  << command << endl;
		comm[num] = command;
		pthread_create( &threads[num], NULL , &request_server , (void*)num);
		num++;
	}
	for(int i=0;i<num;i++){
		pthread_join(threads[i] , NULL);
	}

	return 0;
}
