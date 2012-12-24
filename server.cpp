#include<iostream>
#include<stdio.h>
#include<sys/types.h>
#include<cstdlib>
#include<string.h>
#include<string>
#include<unistd.h>
#include<netinet/in.h> // for struct sockaddr_in 
#include<sys/socket.h> //for socket() and Defined constants
#include<netdb.h>
#include<arpa/inet.h>
#include<pthread.h> //threading
#include<map>

#include<fcntl.h>//open

#include<ctype.h>
#define MX 100
#define FLUSH  {fflush(stdout);}
#include<vector>
#define repla 1
#define CACHE_MAX 10
using namespace std;

#define PROXY_PORT 5000

FILE *fkey,*fpage;

map <string,vector<string> > mp;
map <string,vector<string> >::iterator it;

map <string,string >::iterator it2;
map <string,string> mp2;

vector<string> validHeaders;
void error( char* temp){
	perror(temp);
	exit(1);
}
void *process_client( void* args );

void defineValidHeaders(){
	validHeaders.push_back("Accept");
	validHeaders.push_back("Accept-Language");
	validHeaders.push_back("Accept-Encoding");
	validHeaders.push_back("Accept-Charset");
	validHeaders.push_back("Accept-Datetime");
	validHeaders.push_back("Authorization");
	validHeaders.push_back("Cache-Control");
	validHeaders.push_back("Connection");
	validHeaders.push_back("Cookie");
	validHeaders.push_back("Content-Length");
	validHeaders.push_back("Content-MD5");
	validHeaders.push_back("Content-Type");
	validHeaders.push_back("Date");
	validHeaders.push_back("Expect");
	validHeaders.push_back("From");
	validHeaders.push_back("Host");
	validHeaders.push_back("If-Match");
	validHeaders.push_back("If-Modified-Since");
	validHeaders.push_back("If-None-Match");
	validHeaders.push_back("If-Range");
	validHeaders.push_back("If-Unmodified-Since");
	validHeaders.push_back("Max-Forwards");
	validHeaders.push_back("Pragma");
	validHeaders.push_back("Proxy-Authorization");
	validHeaders.push_back("Range");
	validHeaders.push_back("Referer");
	validHeaders.push_back("TE");
	validHeaders.push_back("Upgrade");
	validHeaders.push_back("User-Agent");
	validHeaders.push_back("Via");
	validHeaders.push_back("Warning");
	validHeaders.push_back("Keep-Alive");
	validHeaders.push_back("Proxy-Connection");
}
int main(){
	fkey=fopen("key","r");
	char temp2[1000];

	while(fscanf(fkey," %[^\n]",temp2)!=EOF){
		char c;
		fscanf(fkey,"%c",&c);
		char *main_key=strtok(temp2,";");
		string tty1=main_key;
		main_key=strtok(NULL,";");
		string tty2=main_key;
		main_key=strtok(NULL,";");
		string tty3=main_key;
		main_key=strtok(NULL,";");
		string tty4=main_key;
		vector<string> pp;
		pp.push_back(tty2);
		pp.push_back(tty3);
		pp.push_back(tty4);
		//cout<<tty4<<endl;
		mp[tty1]=pp;
		mp2[tty4]=tty1;
	}
	fclose(fkey);
	
	defineValidHeaders();

	//cout << "scanned 'key' file" << endl;
	FLUSH;
	int sockfd;
	int newsockfd;

	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	int server_port;

	socklen_t clilen = sizeof( client_address);
	//create a socket
	sockfd = socket( AF_INET , SOCK_STREAM , 0 );
	if(sockfd < 0){
		error("can't establish socket");
	}//IFNET is the protocol family and arg2 => type of socket , arg3 
	//The protocol specifies a particular protocol to be used with the socket.  Normally only a single protocol exists to support a particular socket type within a  given  protocol family, in which case protocol can be specified as 0.  


	//fill the sockaddr_in to specify an internet address (where the socket should be bind)
	bzero( &(server_address) ,sizeof(server_address ));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PROXY_PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;



	//bind the created socket to the sockaddr_in
	if(bind( sockfd , (struct sockaddr *)&server_address , sizeof(struct sockaddr)) < 0) error("can't bind");

	//listen for the clients
	if(listen(sockfd ,5) < 0) error("cannot listen");
	//create a new socket for each client 

	int n;

	pthread_t threads[MX];
	int num=0;
	while(1){
		newsockfd = accept( sockfd , (sockaddr *)&client_address , &clilen);
		//cout << "newsockfd: " << newsockfd << endl;
		pthread_create( &threads[num++], NULL , &process_client , (void*)newsockfd);
	}
	for(int i=0;i<num;i++){
		pthread_join(threads[i] , NULL);
	}
	close(sockfd);
	return 0;

}
bool valid_request( char *buffer ){
	return true;

}
char* gethostname(char* buffer){

}
bool presentInDic( char* key){
	for(it=mp.begin() ; it!=mp.end();it++){
		if(!strcmp(it->first.c_str(),key)){
			cout<<"Page found in Cache" << endl;
			return true;
		}
	}
	return false;
}
string getLastModified(char* key){
	for(it=mp.begin() ; it!=mp.end();it++){
		if(strcmp(it->first.c_str(),key)==0){
			return it->second[0];
		}
	}
}
void responseToClient( int newsockfd , char* key ){
	int n;
	FILE *uu;
	uu=fopen(mp[key][2].c_str(),"r");
	char buffer[10000];
	char c;
	int len;
	while(fscanf(uu," %[^\n]",buffer)!=EOF){
		len=strlen(buffer);
		buffer[len]='\n';
		buffer[len+1]='\0';
		fscanf(uu,"%c",&c);
		n = write( newsockfd , buffer , strlen(buffer));
		buffer[0]='\0';
	}
}
void connectToMainServer( char* host_name , int &sockfd2){

	struct sockaddr_in serv_addr2;
	struct hostent* server2;

	sockfd2=socket(AF_INET,SOCK_STREAM,0);
	server2= gethostbyname(host_name);

	int iplen=16;
	char *ip=(char*)malloc(sizeof(iplen));

	inet_ntop( AF_INET , (void*) server2->h_addr_list[0], ip ,iplen); 

	bzero( (sockaddr_in *)&serv_addr2 , sizeof(sockaddr_in));

	serv_addr2.sin_family = AF_INET;

	bcopy( (char*)server2->h_addr, (char*)& serv_addr2.sin_addr.s_addr,server2->h_length );

	serv_addr2.sin_port = htons(80);

	if(connect(sockfd2,(struct sockaddr*)& serv_addr2, sizeof(serv_addr2)) < 0){
		error("Cannot connect to server");

	}
}
void appendToFile( char* buffer2,string file_no){
	FILE *ffl;
	ffl=fopen(file_no.c_str(),"a");
	fprintf(ffl,"%s",buffer2);
	fclose(ffl);
}
void writeToFile( char* buff2 , string file_no ){
	FILE *ffl;
	ffl=fopen(file_no.c_str(),"w");
	fprintf(ffl,"%s",buff2);
	fclose(ffl);
}

string findDeleteKey( ){
	string compare="\0";
	int ck=0;
	string delete_key;
	for(it=mp.begin() ; it!=mp.end();it++){
		char com1[10];
		strcpy(com1,it->second[1].c_str());
		int com_i=atoi(com1);
		if(ck<=com_i){
			ck=com_i;
			compare=it->second[1];
			delete_key=it->first;
		}
	}
	return delete_key;
}
string to_lower( string a ){
	string b=a;
	for(int i=0;i< a.size();i++){
		b[i] = tolower(a[i]);
	}
	return b;
}
bool validKey( string key){
	for(int j=0;j<validHeaders.size();j++){
		if( to_lower(key) == to_lower(validHeaders[j]) )return true;
	}
	return false;
}
string trim(string& str)
{
str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
return str;
}

bool getHeaders( char* buffer, map<string ,string> &headers , string &page_name , bool request){
	char* line;
	char* head;
	char* word;

	string key,value ;

	char temp_line[1000];
	vector <string> entry;
	string temp_header;
	bool host = 0;
	string hosti = "host";

	char* buff;
	char* temp;
	buff = strstr( buffer , "\r\n\r\n");
	buff = buff + 4;
	*buff = '\0';

	head = strtok( buffer , "\r\n");
	if( head !=NULL){
		
		line = strtok( NULL , "\r\n");
		while(line!=NULL){
			temp_header = line;
			entry.push_back(temp_header);
			line = strtok( NULL , "\r\n");
		}
		for(int i=0;i<entry.size();i++){
			FLUSH;
			sprintf(temp_line,"%s",entry[i].c_str());
			
			size_t  pos = entry[i].find_first_of(":");
			if( pos == NULL ) return false;
			key = entry[i].substr( 0 , pos );
			key = trim(key);
			if(request &&  !validKey( key )) return false;
			if( to_lower(key) == hosti) host = 1;
			value = entry[i].substr( pos+1 );
			value = trim(value);
			headers[ key ] = value ;
			//cout << key << " " << value << endl;
		}


		head  = strtok(head," ");
		if(head !=NULL){ 
			
			head = strtok(NULL," ");
			if( head==NULL) return false;
			page_name = head;
			
			head = strtok(NULL," ");
			if( head==NULL) return false;
			
			head = strtok(NULL," ");
			if( head!=NULL) return false;

		}else return false;

		if(request && !host) return false;
	}else return false;
	return true;
}
void enterLog( char* key , char* option ){
	FILE* fd = fopen("logfile","a"); 
	char line[1000];
	if(strcmp( option , "cache")==0){
		sprintf( line , "%s::served from cache\n",key);
	}
	else{
		sprintf( line , "%s::contacted origin server\n",key);
	}
		fprintf( fd , "%s",line);
		fclose(fd);

}
bool error304( char* buffer ){
	string temp = buffer;
	if( temp.substr(9,3) == "304"){
		cout << "Page Unmodified" << endl;
		return true;
	}
	cout << "Page Modified" << endl;
	return false;;
}
void getpage(char* buffer,int newsockfd){
	char buff[1024];
	strcpy(buff,buffer);

	int count=1;
	int cond_flag=0;

	char file_name[1000];
	char host_name[100];
	char *header,*tt;
	string value;
	
	
	map <string , string > headers;
	map <string , string >::iterator header_it;
	string page_name;
	char tempbuffer[1024];
	strcpy( tempbuffer , buffer);
	if(!getHeaders(tempbuffer , headers , page_name , 1 )){
		return ; //have to make a function saying "Error in headers to the client"
	}

	for(header_it=headers.begin(); header_it!= headers.end() ; header_it++){
		if( to_lower(header_it->first) == "host")  strcpy(host_name,header_it->second.c_str());
	}

	char key[ page_name.size()+1 ];
	strcpy( key, page_name.c_str());

	//cout<<key<<endl;

	bool found_flag=0;
	found_flag = presentInDic( key );

	string tempreq = buffer;
	if(found_flag){//present in cache
		tempreq = tempreq.substr( 0 , tempreq.size()-2);
		string date = "If-Modified-Since: " + getLastModified( key ) + "\r\n\r\n";
		tempreq = tempreq + date;
		strcpy(buffer,tempreq.c_str());
		//cout << "New Request: " << tempreq << endl;
	}

	int sockfd2;
	connectToMainServer( host_name , sockfd2);

	char buffer2[1024];
	string file_no;

	int n;
	n = write( sockfd2 , buffer , strlen(buffer));
	n = read(sockfd2 , buffer2 , 512);
	//if( n <= 0) break;
	buffer2[n] = '\0';
	//cout << "From original server: " << buffer2 << endl;
	if( found_flag && error304(buffer2) ){
		responseToClient( newsockfd, key);
		enterLog( key , "cache" );
	}
	else{
		char buff2[1025];
		map<string,string> responseHeaders;
		string status;
		strcpy(buff2,buffer2);
		getHeaders ( buff2 , responseHeaders , status,0 );
		/*cout << "printg out headers " << endl;
		  for(header_it=responseHeaders.begin(); header_it!= responseHeaders.end() ; header_it++){
		  cout <<  to_lower(header_it->first) << " " << header_it->second << endl;
		  }*/
		if( !found_flag){
			vector<string> ak;
			int map_c=0;
			int ass_flag=0;
			char pl[3];


			for(it2=mp2.begin();it2!=mp2.end();it2++){
				char kl[3];
				strcpy(kl,it2->first.c_str());
				sprintf(pl,"%d",map_c);
				if(strcmp(kl,pl)!=0){ //do not match
					ass_flag=1;
					file_no=pl;
					break;
				}
				map_c++;
			}
			if(ass_flag==0){ //if no hole found in map.
				if(map_c!= CACHE_MAX ){ //if cache is NOT full
					sprintf(pl,"%d",map_c);
					file_no=pl;
				}
				else{  //if cache is full
					string delete_key;
					delete_key = findDeleteKey();
					//cout<<delete_key<<endl;
					mp.erase(delete_key);
					for(it2=mp2.begin();it2!=mp2.end();it2++){
						if(it2->second==delete_key){
							file_no=it2->first;
							mp2.erase(it2);
							break;
						}
					}
				}
			}
			mp2[file_no]=key;

			header_it = responseHeaders.find("Last-Modified");
			if(header_it == responseHeaders.end()) error("error in Last_modified");
			ak.push_back(header_it->second);

			header_it = responseHeaders.find("Content-Length");
			if(header_it == responseHeaders.end() ) error("error in Content_lenght");
			ak.push_back(header_it->second);

			ak.push_back(file_no);

			mp[key]=ak;
		}
		else{
			
			for(it=mp.begin();it!=mp.end();it++){
				if( strcmp(it->first.c_str(),key)==0)	break;
			}

			header_it = responseHeaders.find("Last-Modified");
			if(header_it == responseHeaders.end()) error("error in Last_modified");
			it->second[0] = header_it->second;

			header_it = responseHeaders.find("Content-Length");
			if(header_it == responseHeaders.end() ) error("error in Content_lenght");
			it->second[1] = header_it->second;
			file_no = it->second[2];

		}

		fkey=fopen("key","w");
		//fprintf(fkey,"%s;%s;%s;%s\n",key,ak[0].c_str(),ak[1].c_str(),file_no.c_str());
		for(it=mp.begin();it!=mp.end();it++){
			fprintf(fkey,"%s;%s;%s;%s\n",(*it).first.c_str() , (*it).second[0].c_str() , (*it).second[1].c_str() , (*it).second[2].c_str());
		}
		fclose(fkey);

		n = write(newsockfd,buffer2, n );
		writeToFile( buffer2 , file_no);
		while(1){
			n = read(sockfd2 , buffer2 , 512);
			if( n <= 0) break;
			buffer2[n] = '\0';
			n = write(newsockfd,buffer2, n );
			appendToFile(buffer2 , file_no );
		}
		enterLog( key , "server" );
	}
}
void send_cache( int newsockfd) {
	char buff[1024];
	int fd = open("key",O_RDONLY);
	int n;
	while(1){
		n = read( fd,buff,1023);
		if(n<=0) break;
		else n = write( newsockfd , buff , strlen(buff));
		if( n<=0 ) error("unable to write\n");
	}
	close(fd);
}
void send_log( int newsockfd) {
	char buff[1024];
	int fd = open("logfile",O_RDONLY);
	int n;
	while(1){
		n = read( fd,buff,1023);
		if(n<=0) break;
		else n = write( newsockfd , buff , strlen(buff));
		if( n<=0 ) error("unable to write\n");
	}
	close(fd);

}
void send_search( int newsockfd , string command ) {
	int n;
	command.erase( 0 , 7 );
	//cout << "Seraching for:" << command << endl;
	command = to_lower( command );
	bool flag= 0;
	for( it = mp.begin(); it != mp.end() ; it++){
		if( strstr( it->first.c_str(), command.c_str() )!=NULL){
			string buffer = "\n";
			flag= 1;
			buffer = it->first.c_str() + buffer;;
			n = write( newsockfd , buffer.c_str() , strlen(buffer.c_str()));
			if( n<=0 ) error("unable to write\n");
		}
	}
	if( !flag) {
			string temp= "Not Present\n";
			n = write( newsockfd , temp.c_str()  , temp.size() );
			if( n<=0 ) error("unable to write\n");
	}

}
bool isCommand(char* buffer, int newsockfd){
	string command = buffer;
	if( command == "Print Cache"){
		//cout << "Server Printing cache" << endl;
		send_cache(newsockfd);
	}
	else if( command == "Print Log"){
		//cout << "Server Printing logs" << endl;
		send_log(newsockfd);
	}
	else if( strncmp(command.c_str(),"Search",6)==0){
		//cout << "Server searching key" << endl;
		send_search(newsockfd , command );
	}
	else return false;
	return true;
}
void *process_client( void* args ){
	cout << "Start Processing client" << endl;
	FLUSH;
	char buffer[256]; //for writing
	int n;
	int newsockfd = (int) args;
	//cout << "newsockfd: " << newsockfd << endl;
	FLUSH;
	bzero( buffer , 256);
	while(1){
		n = read( newsockfd , buffer , 255 );
		if(n<=0) break;
		//n = write (newsockfd,"I got your message",18);
		printf("client says : %s\n",buffer);
		if( valid_request( buffer )){
			//gethostname( buffer );
			if( !isCommand( buffer,newsockfd) ){
			  getpage( buffer,newsockfd );
			}
		}
		else{

		}
		bzero( buffer , 256);
	}
	cout << "FINISH Processing client" << endl;
	FLUSH;
	close(newsockfd);
	pthread_exit(0);
}



