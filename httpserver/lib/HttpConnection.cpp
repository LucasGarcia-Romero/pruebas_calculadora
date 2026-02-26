#include "HttpConnection.h"
#include "System.h"

std::string HttpConnection::readData(size_t length)
{
    size_t recvSize = 0;
    std::string line;
    line.resize(length);
    recvSize = recv(conn.socket, (char*)line.data(), (int)length, 0);

    if (recvSize != length) conn.alive = false;

    return line;
}

std::string HttpConnection::readLine()
{
    char buf[1];
    int recvSize = 0;
    std::string line;

    do
    {
        recvSize = recv(conn.socket, &buf[0], 1, 0);
        if (recvSize == 1) line.push_back(buf[0]);
        else    conn.alive = false;
    } while (buf[0] != '\n' && recvSize == 1);
    line.push_back('\0');
    return line;
}

void HttpConnection::buildMimeTypes()
{
    this->mimeTypes["aac"] = "audio/aac";
    this->mimeTypes["abw"] = "application/x-abiword";
    this->mimeTypes["arc"] = "application/octet-stream";
    this->mimeTypes["avi"] = "video/x-msvideo";
    this->mimeTypes["azw"] = "application/vnd.amazon.ebook";
    this->mimeTypes["bin"] = "application/octet-stream";
    this->mimeTypes["bz"] = "application/x-bzip";
    this->mimeTypes["bz2"] = "application/x-bzip2";
    this->mimeTypes["csh"] = "application/x-csh";
    this->mimeTypes["css"] = "text/css";
    this->mimeTypes["csv"] = "text/csv";
    this->mimeTypes["doc"] = "application/msword";
    this->mimeTypes["epub"] = "application/epub+zip";
    this->mimeTypes["gif"] = "image/gif";
    this->mimeTypes["htm"] = "text/html";
    this->mimeTypes["html"] = "text/html";
    this->mimeTypes["ico"] = "image/x-icon";
    this->mimeTypes["ics"] = "text/calendar";
    this->mimeTypes["jar"] = "application/java-archive";
    this->mimeTypes["jpeg"] = "image/jpeg";
    this->mimeTypes["jpg"] = "image/jpeg";
    this->mimeTypes["js"] = "application/javascript";
    this->mimeTypes["json"] = "application/json";
    this->mimeTypes["mid"] = "audio/midi";
    this->mimeTypes["midi"] = "audio/midi";
    this->mimeTypes["mpeg"] = "video/mpeg";
    this->mimeTypes["mpkg"] = "application/vnd.apple.installer+xml";
    this->mimeTypes["odp"] = "application/vnd.oasis.opendocument.presentation";
    this->mimeTypes["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    this->mimeTypes["odt"] = "application/vnd.oasis.opendocument.text";
    this->mimeTypes["oga"] = "audio/ogg";
    this->mimeTypes["ogv"] = "video/ogg";
    this->mimeTypes["ogx"] = "application/ogg";
    this->mimeTypes["pdf"] = "application/pdf";
    this->mimeTypes["png"] = "image/png";
    this->mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
    this->mimeTypes["rar"] = "application/x-rar-compressed";
    this->mimeTypes["rtf"] = "application/rtf";
    this->mimeTypes["sh"] = "application/x-sh";
    this->mimeTypes["svg"] = "image/svg+xml";
    this->mimeTypes["swf"] = "application/x-shockwave-flash";
    this->mimeTypes["tar"] = "application/x-tar";
    this->mimeTypes["tif"] = "image/tiff";
    this->mimeTypes["tiff"] = "image/tiff";
    this->mimeTypes["ttf"] = "font/ttf";
    this->mimeTypes["vsd"] = "application/vnd.visio";
    this->mimeTypes["wav"] = "audio/x-wav";
    this->mimeTypes["weba"] = "audio/webm";
    this->mimeTypes["webm"] = "video/webm";
    this->mimeTypes["webp"] = "image/webp";
    this->mimeTypes["woff"] = "font/woff";
    this->mimeTypes["woff2"] = "font/woff2";
    this->mimeTypes["xhtml"] = "application/xhtml+xml";
    this->mimeTypes["xls"] = "application/vnd.ms-excel";
    this->mimeTypes["xml"] = "application/xml";
    this->mimeTypes["xul"] = "application/vnd.mozilla.xul+xml";
    this->mimeTypes["zip"] = "application/zip";
    this->mimeTypes["3gp"] = "video/3gpp";
    this->mimeTypes["3g2"] = "video/3gpp2";
    this->mimeTypes["7z"] = "application/x-7z-compressed";
}


void HttpConnection::parseMethod(std::string& line)
{
    __int64 methodEnd = line.find(' ');
    std::string method = line.substr(0, methodEnd);
    size_t urlEnd = line.find(' ', methodEnd + 1);
    if (urlEnd == std::string::npos)
        urlEnd = line.find('\r');
    std::string url = line.substr(methodEnd + 1, urlEnd - (methodEnd + 1));

    parameters["METHOD"] = method;
    parameters["METHODURL"] = url;

}

void HttpConnection::parseLine(std::string& line)
{
    size_t keyEnd = line.find(':');
    std::string key = line.substr(0, keyEnd);
    size_t valueEnd = line.find('\r');
    std::string value = line.substr(keyEnd + 1, valueEnd - (keyEnd + 1));
    parameters[key] = value;
}

void HttpConnection::recvHttpPacket()
{
    parameters.clear();
    rawHttpPacket = "";
    std::string line;
    line = readLine();
    rawHttpPacket += line;
    parseMethod(line);
    do {
        line = readLine();
        parseLine(line);
        rawHttpPacket += line;
    } while (!line.empty() && line[0] != '\0' > 0 && line[0] != '\r');

    auto method = parameters.find("METHOD");
    if (method != parameters.end() && method->second == "POST")
    {
        int contentLength = std::stoi(parameters.find("Content-Length")->second);
        line = decodeURIComponent(readData(contentLength));
        //line=readData(contentLength);
        parameters["METHODPARAMS"] = line;
        rawHttpPacket += line;
    }

}

string HttpConnection::readFileFromFolder(string fileName)
{
    string f = "";
    f = readFile(System::htmlFilesFolder + fileName);
    if (f.length() == 0)
    {
        f = readFile(System::dataFilesFolder + fileName);
    }
    return f;
}

void HttpConnection::parseHttpPacket()
{

    std::cout << rawHttpPacket << "\n";
    auto methodNode = parameters.end();
    auto methodUrl = parameters.end();
    response.header = response.body = response.type = "";
    if ((methodNode = parameters.find("METHOD")) != parameters.end())
    {
        auto method = methodNode->second;
        if (method == "GET")
        {
            auto url = parameters.find("METHODURL")->second;
            if (url.ends_with('/'))
            {
                url = "/";
            }
            if (url == "/")
            {
                response.type = mimeTypes["html"];
                response.body = readFileFromFolder("/index.html");
            }
            else
            {
                response.body = readFileFromFolder(url);
                size_t extensionPos = url.rfind('.');
                if (extensionPos != std::string::npos) {
                    auto mimeExtension = url.substr(extensionPos, url.length() - 1);
                    response.type = mimeTypes[mimeExtension];
                }
                else
                    response.type = "application/octet-stream";
            }
            if (response.body.empty())
            {
                response.header = createHeader("404 Not Found", response.type, response.body.length());
                response.sendBody = false;

            }
            else {
                response.header = createHeader("200 OK", response.type, response.body.length());
                response.sendBody = true;

            }
        }
        else if (method == "POST")
        {
            auto url = parameters.find("METHODURL")->second;

            PostMethod* method = postMethods[url];
            if (method != nullptr)
            {
                response.sendBody = true;
                response.type = mimeTypes["html"];
                response.body = method->exec(parameters["METHODPARAMS"]);
                response.header = createHeader("200 OK", response.type, response.body.length());
            }
            else
            {
                response.sendBody = false;
                response.type = mimeTypes["html"];
                response.body = "";
                response.header = createHeader("404 Not Found", response.type, response.body.length());
            }
        }

    }

}

std::string HttpConnection::createHeader(
    std::string responseCode,
    std::string contentType,
    size_t contentLength)
{
    std::string httpHeader = "";

    if (responseCode == "404 Not Found")
    {
        httpHeader += "HTTP/1.1 404 Not Found\r\n";
        httpHeader += "Connection: keep-alive\r\n";
        httpHeader += "Content-length: " + std::to_string(0) + "\r\n";
    }
    else
    {
        httpHeader += "HTTP/1.1 " + responseCode + "\r\n";
        httpHeader += "Connection: keep-alive\r\n";
        httpHeader += "Keep-Alive: timeout = 5, max = 1000\r\n";
        httpHeader += "Content-type: " + contentType + "\r\n";
        httpHeader += "Content-length: " + std::to_string(contentLength) + "\r\n";
        httpHeader += "Accept-Ranges: bytes\r\n";
        httpHeader += "Access-Control-Allow-Origin: *\r\n";
    }
    httpHeader += "\r\n";
    return httpHeader;
}

void HttpConnection::sendHttpPacket()
{

    std::string httpPacket = response.header;
    if(response.sendBody) httpPacket+= response.body;

    send(conn.socket, httpPacket.c_str(), (int)httpPacket.length(), MSG_NOSIGNAL);


}

void HttpConnection::clientLoop()
{
    while (conn.alive) {
        recvHttpPacket();
        parseHttpPacket();
        sendHttpPacket();
    }
    std::cout << "closed client\n";
}
