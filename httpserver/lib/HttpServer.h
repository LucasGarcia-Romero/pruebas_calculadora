#pragma once
#include "common.h"
#include "HttpConnection.h"




class HttpServer {
private:
    int port;
    int sock_fd;
    bool exit = false;
    unsigned int counterId = 0;
    int lastClientSize = 0;
    std::list<HttpConnection*> waitingClients;
    std::mutex counterId_mutex;
    std::mutex client_mutex;


public: 
    HttpServer(int port);
    int  openSocket(int port);
    static void bindSocket(int sock_fd,int port, unsigned long addr);

    static void waitForConnectionsAsync(HttpServer* server);
    void waitForConnections();
    void mainLoop();
    static void clientAsync(HttpConnection* conn);
    HttpConnection* getLastClientID();
    bool checkClient();
   
};
