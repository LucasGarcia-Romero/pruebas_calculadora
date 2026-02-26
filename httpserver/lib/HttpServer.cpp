#include "HttpServer.h"



HttpServer::HttpServer(int port):port(port)
{
#ifdef _MSC_VER
     WSADATA wsa;
     WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    sock_fd = HttpServer::openSocket(port);
    HttpServer::bindSocket(sock_fd,port, INADDR_ANY);
    std::thread waitForConnectionsThread(waitForConnectionsAsync,this);
    waitForConnectionsThread.detach();
}

__int64 HttpServer::openSocket(int port) {
     __int64 sock_fd = socket(AF_INET, SOCK_STREAM, 0);
     if (sock_fd < 0)
     {
         std::cout << "Error creating socket\n";
     }
     int option = 1;
     setsockopt(sock_fd, SOL_SOCKET,
         (SO_REUSEADDR),
         (char*)&option, sizeof(option));
     return sock_fd;
 }


 void HttpServer::bindSocket(__int64 sock_fd,int port, ULONG addr)
 {
     struct sockaddr_in serv_addr;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = addr;
     serv_addr.sin_port = htons(port);
     if (bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
         std::cout << "ERROR on binding";

     listen(sock_fd, SOMAXCONN);
 }

void HttpServer::waitForConnectionsAsync(HttpServer* server)
{
    while(!server->exit)
    {
        server->waitForConnections();
    }
}

void HttpServer::clientAsync(HttpConnection* conn) {
    conn->clientLoop();
    delete conn;
}


void HttpServer::waitForConnections(){
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    
    __int64 newsock_fd = accept(this->sock_fd,
                            (struct sockaddr * ) &cli_addr,
                            &clilen);
    connection_t client;
    counterId_mutex.lock();
        client.id=counterId++;
    counterId_mutex.unlock();

    client.alive=true;
    client.socket=newsock_fd;
    
    client_mutex.lock();
        waitingClients.push_back(new HttpConnection(client.id,client));
    client_mutex.unlock();

}

HttpConnection* HttpServer::getLastClientID()
{
    client_mutex.lock();
        auto id = waitingClients.back();
        waitingClients.pop_back();
#ifdef DEBUG
        std::cout << "new client\n";
#endif
    client_mutex.unlock();
    return id;
}

bool HttpServer::checkClient()
{
    return waitingClients.size() > 0;

}

void HttpServer::mainLoop()
{
    while (!this->exit)
    {
        if (waitingClients.size() > 0)
        {
            auto newClient = getLastClientID();
            std::thread clientThread(clientAsync, newClient);
            clientThread.detach();
        }
        std::this_thread::sleep_for(100ms);
    }
}

