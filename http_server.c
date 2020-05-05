// mrsilvers@163.com
// 2020-05-05 10:20:35
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "request.h"
/*
struct header{
	char *origin_header;
	char method[8];
	char *uri;
};

struct get{
	char *uri_path;
	char *uri_query;
};

struct post{
	int content_length;
	char *from_data;
	char *uri;
};

struct method{
	struct get get;
	struct post post;
};

struct request{
	char method[8];
	struct header header;
	struct get get;
    struct post post;
};
*/

char* req_header_pointer_seq = NULL;
char* request_handle(int client_sockfd,char *request_str);
char* post_handle(int client_sockfd,struct post *r_post);
char* get_handle(int client_sockfd,struct get *r_get);
void response(int client_sockfd,char* content);
void response_200ok(int client_sockfd,char *buf);
void response_404not_found(int client_sockfd,char *buf);
int http_socket(u_short *port);

int main(int argc,char const *argv[]){
    struct sockaddr_in client_addr;
    int server_sockfd = -1;
    u_short server_port = 8888;
    char buffer[4096];
    socklen_t client_addr_len = sizeof(client_addr);
    
    server_sockfd = http_socket(&server_port);
    printf("http server running in port 8888...\n");
    while(1){
        int numbytes;
        int i = 0;
        char *msg = NULL;
        memset(&buffer,0,sizeof(buffer));
        int client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_addr,&client_addr_len);
        if(client_sockfd == -1){
            perror("accept");
            exit(1);
        }
        printf("client connect.\n");
        //while((numbytes=recv(client_sockfd,buffer,sizeof(buffer),0)>0)){
        //    i++;

        
        //}
        numbytes = recv(client_sockfd,buffer,sizeof(buffer),0);
        printf("recv times: %d\n",i);
        msg = request_handle(client_sockfd,buffer);
        printf("request handle result: %s\n",msg);
    }
    return 0;
}

int http_socket(u_short *port){

	int httpd = 0;
	struct sockaddr_in name;

	httpd = socket(AF_INET, SOCK_STREAM, 0);
	if (httpd == -1){
		perror("socket");
	}
    printf("create server socket success.\n");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0){
		perror("bind");
		exit(1);
	}
    printf("bind success.\n");
	if (*port == 0){ /* if dynamically allocating a port */
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1){
			perror("getsockname");
			exit(1);
		}
			
		*port = ntohs(name.sin_port);
	}
	if (listen(httpd, 5) < 0){
		perror("listen");
		exit(1);
	}
	printf("listen port success.\n");
	return httpd;
	
}

char* request_handle(int client_sockfd,char *request_str){
	struct request request;
    struct header header;
    memset(&request,0,sizeof(request));
	memset(&header,'\0',sizeof(header));
	char buf[4096];

	//strcpy把src所指向的字符串复制到dest所指向的空间中，遇到'\0'结束（'\0'也会拷贝过去） ,但header_str没有'\0'
	strcpy(buf,request_str);
	buf[strlen(request_str)] = '\0';
	//strlen的结果不包含'/0'
	header.origin_header = (char *)malloc(strlen(buf)+1);
	strcpy(header.origin_header,buf);
	header.origin_header[strlen(buf)] = '\0';
	char *step = " \r\n";
	char *token;

	//get request method
	token = strtok(buf,step);
	strcpy(header.method,token);

	//get request uri
	token = strtok(NULL,step);
	header.uri = (char *)malloc(strlen(token)+1);
	strcpy(header.uri,token);

	printf("The HTTP Header: %s\n",header.origin_header);
	printf("The Request Method: %s\n",header.method);
	printf("The Request URI: %s\n",header.uri);

    // pack data
    request.header = header;
    strcpy(request.method, header.method);
    
	if(strcasecmp(header.method,"GET")==0){
        struct get r_get;
		memset(&r_get,0,sizeof(r_get));

		strcpy(buf,header.uri);
		if((token = strtok(buf,"?"))!=NULL){
		    r_get.uri_path = (char *)malloc(strlen(token)+1);
			strcpy(r_get.uri_path,token);
			if((token = strtok(NULL,"?"))!=NULL){
			    r_get.uri_query = (char *)malloc(strlen(token)+1);
				strcpy(r_get.uri_query,token);

			}else{
				//pass
				printf("in 2...\n");
			}

		}else{
			//pass
			printf("in 1...\n");
		}
		printf("GET URI PATH: %s\n",r_get.uri_path);
		printf("GET URI Query: %s\n",r_get.uri_query);
		printf("enter get_handle func...\n");
        //pack data
        request.get = r_get;

		return get_handle(client_sockfd,&r_get);

	}else if(strcasecmp(header.method,"POST")==0){
        struct post r_post;
		memset(&r_post,0,sizeof(r_post));
		r_post.uri = (char *)malloc(strlen(header.uri)+1);
		strcpy(r_post.uri,header.uri);
		req_header_pointer_seq = strstr(header.origin_header,"Content-Length:");
		/* DEBUG printf("%s\n",temp);*/
		r_post.content_length = atoi(req_header_pointer_seq+16);
		r_post.from_data = (char *)malloc(r_post.content_length);

        char *temp = strstr(req_header_pointer_seq,"\r\n\r\n");
    	temp = temp + 4;
    	int i;
    	for(i=0;i<r_post.content_length;i++){
	    	*r_post.from_data = *temp++;
	    	r_post.from_data++;
    	}
    	r_post.from_data -= i;
    	printf("from_data: %s",r_post.from_data);
        return "post pass";

    }else{// For Bad Request Method

	    char *bad_method = "other method pass";
        return bad_method;
    }

}

char *get_handle(int client_sockfd,struct get *r_get){
    char *msg = "return response!\n";
    char buf[512];
    struct stat file_st; 
	if(r_get->uri_query==NULL){
		char filename[512];
        //sprintf(filename,"/home/silvers/httpd/www/html%s",r_get->uri_path);
        sprintf(filename,"./http_server_dir%s",r_get->uri_path);
		printf("filename: %s\n",filename);
		if(filename[strlen(filename)-1] == '/'){
			filename[strlen(filename)-1] = '\0';
		}
		if(stat(filename,&file_st)==-1){
			//file not found
		    response_404not_found(client_sockfd,buf);
            perror("get file stat");

		}else{
            printf("file mode: %d\n",file_st.st_mode);
			if(file_st.st_mode & S_IFDIR){

				response_404not_found(client_sockfd,buf);

			}else{
                printf("read html file...\n") ;
				char buf[1024];
                memset(buf,0,sizeof(buf));
                response_200ok(client_sockfd,buf);
                memset(buf,0,sizeof(buf));
				FILE *resource = NULL;
			    /*读取文件中的所有数据写到 socket */
			    resource = fopen(filename, "r");
			    if(resource == NULL){
			    	perror("fopen");
			    	exit(1);
			    }
			    fgets(buf, sizeof(buf), resource);
			    while (!feof(resource)){
                    printf("send: %s\n",buf);
			        send(client_sockfd, buf, strlen(buf), 0);
			        fgets(buf, sizeof(buf), resource);
			    }
                fclose(resource);
			}
		}
		
	}else{
		int cgi = 1;
		//pass
	}
    close(client_sockfd);
    printf("client connect close.\n");
    return "return response!\n";
}

void response(int client_sockfd,char* content){
    printf("send: %s to client.\n",content);
    send(client_sockfd,content,strlen(content),0);
	//close(client_sockfd);
}

void response_200ok(int client_sockfd,char *buf){
	memset(buf,0,sizeof(buf));
	strcpy(buf,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	printf("send: %s\n",buf);
    send(client_sockfd,buf,strlen(buf),0);
}


void response_404not_found(int client_sockfd,char *buf){
    memset(buf,0,sizeof(buf));
	strcpy(buf,"HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n");
	strcat(buf,"<HTML><TITLE>Not Found</TITLE>\r\n<BODY><P>The server could not fulfill\r\nyour request because the resource specified\r\nis unavailable or nonexistent.\r\n</BODY></HTML>\r\n");
    send(client_sockfd,buf,strlen(buf),0);
}

















