/*httpd.c */
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <bits/socket.h>
#include<bits/socket.h>
//global
char *error;
#define LISTENADD "127.0.0.1"
//returns 0 on error , else returns a socket fd
int srv_init(int portno)
{
    int s;
    struct sockaddr_in srv;
    s=socket(AF_INET,SOCK_STREAM,0);
    if(s<0){
        error="socket() error";
        return 0;
    }
    srv.sin_family=AF_INET;
    srv.sin_addr.s_addr=inet_addr(LISTENADD);
    srv.sin_port=htons(portno);

    if (bind(s,(struct sockaddr*)&srv,sizeof(srv)))
    {
        close(s);
        error="bind() error";
        return 0;
    }
    if(listen(s,5))
    {
        close(s);
        error="listen() error";
        return 0;
    }
return s;
}
//returns 0 on error or returns the new client socket fd
int cli_accept(int s)
{
    int c;
    socklen_t addrlen;
    struct sockaddr_in cli;
    memset(&cli,0,sizeof(cli));
    c=accept(s,(struct sockaddr*)&cli,&addrlen);
    if(c<0)
    {
        error="accept() error";
        return 0;
    }
    return c;
}
void  cli_conn(int s,int c){return ;}
int main(int argc,char *argv[])
{
    int s,c;
    char *port;
    if(argc<2)
    {
        fprintf(stderr,"Usage :%s <listening port>\n",argv[0]);
        return -1;
    }
    else
        port=argv[1];
    s=srv_init(atoi(port));
    if(!s)
    {
        fprintf(stderr,"%s\n",error);
        return -1;
    }
    printf("Listening on %s:%s\n",LISTENADD,port);

    while(1)
    {
        c=cli_accept(s);
        if(!c)
        {
            fprintf(stderr,"%s\n",error);
            continue;
        }
        printf("Incoming connection\n");
        /*creates a copy of the process 
        for the main process return the new process id
        for the new process return 0*/
        if(!fork())
            cli_conn(s,c);

        return -1;
        
    }

}
 