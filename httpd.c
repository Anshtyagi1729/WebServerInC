#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <bits/socket.h> // for socket-specific constants
#include<bits/socket.h> // for socket definitions and constants

/* Structure to hold HTTP request details */
struct sHttpRequest{
    char methods[8]; // Stores the HTTP method (e.g., "GET")
    char url[128]; // Stores the requested URL
};
typedef struct sHttpRequest httpreq;

char *error; // Global error string to hold error messages

#define LISTENADD "127.0.0.1" // Server address

// Initializes the server socket on the specified port
int srv_init(int portno)
{
    int s;
    struct sockaddr_in srv;
    s=socket(AF_INET,SOCK_STREAM,0); // Create socket
    if(s<0){
        error="socket() error"; // Socket creation failed
        return 0;
    }
    srv.sin_family=AF_INET; // Use IPv4
    srv.sin_addr.s_addr=inet_addr(LISTENADD); // Bind to loopback address
    srv.sin_port=htons(portno); // Set port

    // Bind the socket
    if (bind(s,(struct sockaddr*)&srv,sizeof(srv)))
    {
        close(s);
        error="bind() error"; // Binding failed
        return 0;
    }
    
    // Set up listening on the socket
    if(listen(s,5))
    {
        close(s);
        error="listen() error"; // Listen failed
        return 0;
    }
    return s; // Return socket descriptor if successful
}

// Accepts incoming client connections
int cli_accept(int s)
{
    int c;
    socklen_t addrlen;  
    struct sockaddr_in cli;
    memset(&cli,0,sizeof(cli));
    c=accept(s,(struct sockaddr*)&cli,&addrlen); // Accept connection
    if(c<0)
    {
        error="accept() error"; // Accept failed
        return 0;
    }
    return c; // Return client socket descriptor
}

/* Parses the HTTP request string to extract method and URL */
httpreq* parse_request(const char *request) {
    httpreq* req=malloc(sizeof(httpreq)); // Allocate memory for request struct
    const char *start = strstr(request, "Received: "); // Look for start of the request
    if (start) {
        start += strlen("Received: ");   // Skip "Received: " prefix
    } else {
        start = request; 
    }

    char temp[1024];
    strncpy(temp, start, sizeof(temp)); // Copy the request to a temp buffer

    // Extract the method and URL from the first line of the request
    char *line = strtok(temp, "\n");     // Get the first line
    if (line) {
        char *token = strtok(line, " "); // First token: Method
        if (token) {
            strcpy(req->methods, token); // Copy method
            token = strtok(NULL, " ");   // Second token: URL
            if (token) {
                strcpy(req->url, token); // Copy URL
            }
        }
    }
    return req; // Return parsed request structure
}

// Reads client request data
char *cli_read(int c)
{
    static char buf[512];
    memset(buf,0,512);
    if(read(c,buf,511)<0) // Read request from client socket
    {
        error="read() error"; // Read failed
        return 0;
    }
    else 
        return buf; // Return buffer with client data
}

// Handles client connection and processes requests
void  cli_conn(int s,int c)
{
    httpreq* req;
    char *p,*res;
    p=cli_read(c); // Read client request
    
    if(!p) // Check if read was successful
    {
        fprintf(stderr,"%s\n",error);
        close(c); // Close client socket
        return ;
    }
    req=parse_request(p); // Parse the client request
    if(!req) // Check if parsing was successful
    {
        fprintf(stderr,"%s\n",error);
        close(c); // Close client socket
        return;
    }
    
    // Further code for processing requests
    if(!strcmp(req->methods,"GET")||!strcmp(req->url,"/app/webpage")) // Check for specific method and URL
    {
        res="<html>hello world</html>"; // Respond with hello world page
        http_headers(c,200); // Send HTTP 200 OK
        http_response(c,res); // Send HTML response
        close(c); // Close client socket
        free(req); // Free allocated memory
    }
    else {
        http_headers(c,404); // Send HTTP 404 Not Found
    }
    return ;
}

// Main function to set up server and accept connections
int main(int argc,char *argv[])
{
    int s,c;
    char *port;
    char* p;
    if(argc<2) // Check if port argument is passed
    {
        fprintf(stderr,"Usage :%s <listening port>\n",argv[0]);
        return -1;
    }
    else
        port=argv[1];
    s=srv_init(atoi(port)); // Initialize server with specified port
    if(!s)
    {
        fprintf(stderr,"%s\n",error); // Print error if server init failed
        return -1;
    }
    printf("Listening on %s:%s\n",LISTENADD,port); // Print listening address

    while(1) // Infinite loop to accept connections
    {
        c=cli_accept(s); // Accept new client
        if(!c) // Check if accept was successful
        {
            fprintf(stderr,"%s\n",error); // Print error if accept failed
            continue;
        }
        printf("Incoming connection\n");
        
        // Fork process to handle connection in a child process
        if(!fork())
            cli_conn(s,c); // Child process handles connection

        return -1;
    }
}
