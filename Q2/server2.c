#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>


char webpage[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html><head><title>Our Sample Page</title><link rel=\"icon\" href=\"testicon.ico\">\r\n"
        "<style>body {background-color: #FFFF00}</style></head>\r\n"
        "<body><center><h1>Hello World!</h1><br>\r\n"
        "<img src=\"testpic.jpg\"><img src=\"tstpic2.jpg\"></center></body></html>\r\n";

int main(int argc , char *argv[]){   
    int master_fd_server , addrlen , new_socket , client_socket[30] ,  
          max_clients = 30 , activity, i , valread , fd_client;   
    struct sockaddr_in server_addr;   
    int fdimg, max_client, opt = 1;
         
    char buf[1025]; 

    server_addr.sin_family = AF_INET;   
    server_addr.sin_addr.s_addr = INADDR_ANY;   
    server_addr.sin_port = htons( 8080 );   

    //set of socket descriptors  
    fd_set readfds;   

    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   {   
        client_socket[i] = 0;   
    }

    // Create master socket
    master_fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (master_fd_server < 0) {
        perror("socket");
        exit(1);
    }

    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_fd_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  sizeof(opt)) < 0 ){   
        perror("setsockopt");
        exit(1);
    }


    if (bind(master_fd_server, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)   {   
        perror("bind");
        close(master_fd_server);
        exit(1);
    }

    if (listen(master_fd_server, 10) == -1){   
        perror("listen");
        close(master_fd_server);
        exit(1);
    }

    //accept the incoming connection  
    addrlen = sizeof(server_addr);   
    puts("Waiting for connections ...");   

    while(1)
    {
        //clear the socket set  
        FD_ZERO(&readfds);   

        //add master socket to set  
        FD_SET(master_fd_server, &readfds);   
        max_client = master_fd_server;   

        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   {   
            //socket descriptor  
            fd_client = client_socket[i];   

            //if valid socket descriptor then add to read list  
            if(fd_client > 0)   
                FD_SET( fd_client , &readfds);   

            //highest file descriptor number, need it for the select function  
            if(fd_client > max_client)   
                max_client = fd_client;   
        }

        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_client + 1 , &readfds , NULL , NULL , NULL);   

        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");   
        }

        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_fd_server, &readfds))   
        {   
            if ((new_socket = accept(master_fd_server,  
                    (struct sockaddr *)&server_addr, (socklen_t*)&addrlen))<0)   {   
                perror("Connection Failed! Can't Conneect to Client .... \n");
                continue; 
            }   

            printf("Accepted the Client Connection ..... \n");
                 
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   {   
                //if position is empty  
                if( client_socket[i] == 0 )   {   
                    client_socket[i] = new_socket;                            
                    break;   
                }   
            }   
        }   

        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++){   
            fd_client = client_socket[i];   
                 
            if (FD_ISSET( fd_client , &readfds)) {

                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read(fd_client , buf, 1024)) == 0)  {

                    //Somebody disconnected , get his details and print  
                    getpeername(fd_client , (struct sockaddr*)&server_addr , \ 
                        (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d\n" ,  inet_ntoa(server_addr.sin_addr) , ntohs(server_addr.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( fd_client );   
                    client_socket[i] = 0;   
                }
                // Send files 
                else {   
                    if (!strncmp(buf, "GET /testicon.ico", 16)) {
                    fdimg = open("testicon.ico", O_RDONLY);
                    sendfile(fd_client, fdimg, NULL, 200000);
                    close(fdimg);
                    } 
                    else if (!strncmp(buf, "GET /testpic.jpg", 16)) {
                        fdimg = open("testpic.jpg", O_RDONLY);
                        sendfile(fd_client, fdimg, NULL, 60000);
                        close(fdimg);
                    }
                    else if (!strncmp(buf, "GET /tstpic2.jpg", 16)) {
                        fdimg = open("tstpic2.jpg", O_RDONLY);
                        sendfile(fd_client, fdimg, NULL, 120000);
                        close(fdimg);
                    }
                    else {
                        write(fd_client, webpage, sizeof(webpage) - 1);
                    }
                }
            }
        }
    }

    return 0;   
}