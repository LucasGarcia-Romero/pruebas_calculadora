#include <iostream>
#include "lib/HttpServer.h"
#include "lib/System.h"
#include "lib/PostMethod.h"

int main(int argc, char** argv)
{
    System::parseParams(argc, argv);
    // recoger archivos de configuración

    HttpServer* s = new HttpServer(System::serverPort);

    s->registerPostMethod(new Login());

    s->registerPostMethod(new ListFiles());

    s->registerPostMethod(new RecordData());

    s->registerPostMethod(new GetConfig());
    s->registerPostMethod(new SaveConfig());

    s->mainLoop();
    delete s;
}