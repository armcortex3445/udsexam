#include <iostream>
#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <stdlib.h>

#define UDS_APP_A  "/tmp/app_a.uds"
#define UDS_APP_B "/tmp/app_b.uds"
#define LEN 30


using std::thread;

struct uds_data{

    unsigned int cmd;
    unsigned int len;
    char str[LEN+1];
};

typedef struct process{

struct sockaddr_un server;
struct sockaddr_un client;

}process;

int initSock(int sock ,struct sockaddr_un *addr  ,const char *path)
{
    
    if(0 == access(path,F_OK))  unlink(path); // 

    memset(addr,0, sizeof(struct sockaddr_un));

    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path,path);
    std::cout << "[initSock] " << addr->sun_path << std::endl; 
    if(0>bind(sock,(struct sockaddr*) addr, sizeof(struct sockaddr_un))){
        std::cout << "bind() Error. " << path << std::endl;
        exit(1); //
    }

    sleep(2);
    return 1;

}

void client(int sock, process *p)
{
    
    int isDone = 0;

    for(int i = 0; i < 10; i++){
        std::cout << "Client boot : " << i << std::endl;
        sleep(1);
    }
    
    while(1){
        struct uds_data data = { 0, };
        struct uds_data send_data = { 0 , };
        socklen_t client_size = sizeof(p->client);

        if(0 > recvfrom(sock,&data, sizeof(data), 0 ,(struct sockaddr*) &p->client, &client_size)){
            std::cout << "Client fail recvfrom" << std::endl; 
        }
        
        switch(data.cmd){
        
        case 1 : 
            std::cout << "connect to server : " << data.str << " path : " << p->client.sun_path <<std::endl;
            send_data.cmd = 1;
            strcpy(send_data.str, "<Client> Hello");
            sendto(sock,&send_data,sizeof(struct uds_data),0,(struct sockaddr*) &p->client, sizeof(p->client));
            break;
        
        case 2 :
            std::cout << "reply from server : " << data.str << std::endl;
            send_data.cmd = 2;
            strcpy(send_data.str, "<Client> Done");
            sendto(sock,&send_data, sizeof(uds_data), 0 , (struct sockaddr *) &p->client, sizeof(p->client));
            isDone = 1;
            break;
        default :
            std::cout<< "client ..." << std::endl;
            break; 
        }
        
        if(isDone){
            break;
        }
        sleep(2);
    }

    std::cout<<"[END] Thread Client" << std::endl; 
}

void server(int sock, process *p)
{

    int isDone = 0;
    
    struct uds_data send_data = {0 , };

    for(int i = 0; i < 3; i++){
        std::cout << "Server boot : " << i << std::endl;
        sleep(1);
    }

    send_data.cmd = 1;
    strcpy(send_data.str,"initialize");
    send_data.len = sizeof(send_data.str);
    if(0> sendto(sock,&send_data,sizeof(send_data), 0 ,(struct sockaddr *)&p->client , sizeof(p->client))){
    
            std::cout << "server init fail." << std::endl;
            return;
    }

    std::cout << "<server> send init" << std::endl;

    while(1){
        struct uds_data data = { 0, };
        send_data = {0,};

        if(0>recvfrom(sock,&data, sizeof(data), 0 ,NULL, 0)){
            std::cout << "Server fail. recvfrom" << std::endl;
        }
        
        switch(data.cmd){ 
        
        case 1 : 
            std::cout << "<Server> connect success : " << data.str <<std::endl;
            send_data.cmd = 2;
            strcpy(send_data.str, "<Server> working");
            send_data.len = sizeof(send_data.str);
            sendto(sock,&send_data,sizeof(struct uds_data),0,(struct sockaddr *) &p->client, sizeof(p->client));
            break;
        case 2 :
            std::cout << "<Server> reply from Client : " << data.str << std::endl;
            isDone = 1;
            break;
        defaut :
            std::cout <<"server ... " << std::endl;
            break;
        }

        if(isDone){
            break;
        }

        sleep(2);
        
    }

    std::cout<<"[END] Thread Server" << std::endl; 
}



int main()
{
    int sock1;
    int sock2;
    sock1 = socket(PF_FILE,SOCK_DGRAM,0);
    sock2 = socket(PF_FILE,SOCK_DGRAM,0);
    if(sock1 < 0 || sock2 < 0) 
    {
        std::cout << "sock ERROR : " << sock1 << sock2 << std::endl;
    }

    process a = {0 , } , b = { 0, };

   
    
    initSock(sock1,&a.server,UDS_APP_A);
    initSock(sock2,&b.server,UDS_APP_B);
    b.client.sun_family = AF_UNIX;
    strcpy(b.client.sun_path,UDS_APP_A);

    thread t1(client,sock1,&a);
    thread t2(server,sock2,&b);


    t1.join();
    t2.join();

    std::cout << "END exam" << std::endl;     
    close(sock1);
    close(sock2);

}



