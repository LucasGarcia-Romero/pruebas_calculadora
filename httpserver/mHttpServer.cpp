// mHttpServer.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include "lib/HttpServer.h"
#include "lib/System.h"

int main(int argc, char** argv)
{
    System::parseParams(argc, argv);
    HttpServer* s = new HttpServer(System::serverPort);
    s->mainLoop();
    delete s;
    std::cout << "Hello World!\n";
}

