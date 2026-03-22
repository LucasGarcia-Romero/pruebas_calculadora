// Es el punto de inicio del programa

#include <iostream>
#include "lib/HttpServer.h"
#include "lib/System.h"
#include "lib/PostMethod.h"

int main(int argc, char** argv)
{
    // lee argumentos de arranque, en este caso no hay por el momento
    System::parseParams(argc, argv);

    // levanta un servidor Http + abrir socket
    HttpServer* s = new HttpServer(System::serverPort);

    // Comienza a registrar los endpoint de PostMethod
    s->registerPostMethod(new Login());

    s->registerPostMethod(new ListFiles());

    s->registerPostMethod(new RecordData());

    // recoger archivos de configuración
    s->registerPostMethod(new GetConfig());
    s->registerPostMethod(new SaveConfig());

    // Entra en el bucle infinito
    s->mainLoop();
    delete s;
}