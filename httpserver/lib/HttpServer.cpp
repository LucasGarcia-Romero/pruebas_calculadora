#include "HttpServer.h"

HttpServer::HttpServer(int port):port(port)
{
#ifdef _MSC_VER
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    sock_fd = HttpServer::openSocket(port);
    HttpServer::bindSocket(sock_fd, port, INADDR_ANY);
    std::thread waitForConnectionsThread(waitForConnectionsAsync, this);
    waitForConnectionsThread.detach();
}

int HttpServer::openSocket(int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
        std::cout << "Error creating socket\n";
    int option = 1;
    setsockopt(sock_fd, SOL_SOCKET, (SO_REUSEADDR), (char*)&option, sizeof(option));
    return sock_fd;
}

void HttpServer::bindSocket(int sock_fd, int port, unsigned long addr)
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
    while (!server->exit)
        server->waitForConnections();
}

void HttpServer::clientAsync(HttpConnection* conn) {
    conn->clientLoop();
    delete conn;
}

void HttpServer::waitForConnections()
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int newsock_fd = accept(this->sock_fd, (struct sockaddr*)&cli_addr, &clilen);

    connection_t client;
    counterId_mutex.lock();
        client.id = counterId++;
    counterId_mutex.unlock();

    client.alive = true;
    client.socket = newsock_fd;

    client_mutex.lock();
        waitingClients.push_back(new HttpConnection(client.id, client, postMethods));
    client_mutex.unlock();
}

HttpConnection* HttpServer::getLastClientID()
{
    client_mutex.lock();
        auto id = waitingClients.back();
        waitingClients.pop_back();
    client_mutex.unlock();
    return id;
}

bool HttpServer::checkClient()
{
    return waitingClients.size() > 0;
}

// Permite tener varios clientes en paralelo
void HttpServer::mainLoop()
{
    while (!this->exit)
    {
        // Si hay clientes en la cola
        if (waitingClients.size() > 0)  
        {
            // Toma el ultimo cliente
            auto newClient = getLastClientID();
            // Crea un nuevo hilo para el cliente
            std::thread clientThread(clientAsync, newClient);
            // Deja el cliente corriendo de forma independiente
            clientThread.detach();
        }
        std::this_thread::sleep_for(100ms);
    }
}