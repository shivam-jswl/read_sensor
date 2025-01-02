#include<iostream>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include<string.h>


#define PORT 2000

int main(){
    int srv_fd=0, connected_socket=0;

    char buff[1024] ={0};
    const char* msg = "$3131012C015E014A00C803140D0A"; 
    struct sockaddr_in address;
    int address_len = sizeof(address);

    if((srv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Unable to create socket.\n");
        return 0;
    }

    std::cout<<"Socket id is "<<srv_fd<<std::endl;

    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    if(bind(srv_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Binding failed.\n");
        return 0;
    }

    if(listen(srv_fd, 3) == -1){
        perror("Unable to listen.\n");
        return 0;
    }

    if((connected_socket = accept(srv_fd, (struct sockaddr *)&address, (socklen_t *)&address_len)) == -1){
        perror("Unable to accept.\n");
        return 0;
    }
    std::cout<<"Reading..\n";    
    if(read(connected_socket, buff, 1024)== -1){
        perror("Unable to read.\n");
        return 0;
    }
    std::cout<<"Message recived from client \""<<buff<<"\"."<<std::endl;

    while(true){

        if(send(connected_socket, msg, strlen(msg), 0) == -1){
            perror("Unable to send.\n");
            return 0;
        }
        // std::cout<<"Message \""<<msg<<"\" send."<<std::endl;
    }
    close(srv_fd);
    close(connected_socket);
    return 0;
}