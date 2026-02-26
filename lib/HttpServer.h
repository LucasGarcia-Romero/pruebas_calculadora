#pragma once
#include "common.h"
#include "HttpConnection.h"




class HttpServer {
private:
    int port;
    __int64 sock_fd;
    bool exit = false;
    unsigned int counterId = 0;
    int lastClientSize = 0;
    std::list<HttpConnection*> waitingClients;
    std::mutex counterId_mutex;
    std::mutex client_mutex;


public: 
    HttpServer(int port);
    __int64  openSocket(int port);
    static void bindSocket(__int64 sock_fd,int port, ULONG addr);

    static void waitForConnectionsAsync(HttpServer* server);
    void waitForConnections();
    void mainLoop();
    static void clientAsync(HttpConnection* conn);
    HttpConnection* getLastClientID();
    bool checkClient();
   
};
