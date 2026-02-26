#pragma once
#include "common.h"
#include "PostMethod.h"
typedef struct
{
    int size;
    unsigned char* data;
}msg_t;

typedef struct {
    unsigned int id;
    unsigned int serverId;
    __int64 socket;
    bool alive;
}connection_t;

typedef struct {
    bool sendBody;
    std::string header;
    std::string type;
    std::string body;
}httpResponse_t;

class HttpConnection {
private:
    int id;
    connection_t conn;
    httpResponse_t response;
    std::string rawHttpPacket;
    std::map<std::string, std::string> parameters;
    std::map<std::string, std::string> mimeTypes;
    std::map<std::string, PostMethod*> postMethods;
public:
    HttpConnection(int id, connection_t conn) :id(id), conn(conn) {
        buildMimeTypes();
        postMethods["/listFiles"] = new ListFiles();
        postMethods["/recordData"] = new RecordData();

    }
    void parseLine(std::string& line);
    void parseMethod(std::string& line);

    void recvHttpPacket();
    void parseHttpPacket();
    void sendHttpPacket();
    void clientLoop();
    std::string readLine();
    std::string readData(size_t length);

    void buildMimeTypes();
    string createHeader(std::string responseCode,
        string contentType,
        size_t contentLength);
    string readFileFromFolder(string fileName);
};

