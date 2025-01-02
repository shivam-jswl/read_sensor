#include <iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>


#define SERVER_IP "127.0.0.1"
#define PORT 2000

int main(int argc, char** argv){

    int sock_fd;
    struct sockaddr_in address;

    char buff[1024] = {0};
    const char* msg = argv[1];

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0))==0){
        perror("Unable to create socket.\n");
        return 0;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if(inet_pton(AF_INET, SERVER_IP, &address.sin_addr) <= 0){
        perror("Invalid IP address");
        return 0;
    }
    if(connect(sock_fd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) == -1){
        perror("unable to connect\n");
        return 0;
    }

    if(send(sock_fd, msg, strlen(msg), 0) == -1){
        perror("Unable to send.\n");
        return 0;
    };
    std::cout<<"Message sent to Server \""<<msg<<"\"."<<std::endl;

    if(read(sock_fd, buff, 1024) == -1){
        perror("Unable to read.\n");
        return 0;
    }
    std::cout<<"Message recived from Server \""<<buff<<"\"."<<std::endl;

    close(sock_fd);
    return 0;

}