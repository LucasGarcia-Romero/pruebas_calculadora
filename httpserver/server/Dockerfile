FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ make \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copiamos el código del server
COPY httpserver/ /app/

# Compilamos (ARM64 en OrangePi)
RUN g++ -O2 -std=c++20 \
    mHttpServer.cpp \
    lib/HttpServer.cpp \
    lib/HttpConnection.cpp \
    lib/PostMethod.cpp \
    -o /usr/local/bin/mHttpServer

# HTML opcional si lo usas
# (se queda dentro de /app/html)

EXPOSE 8000

CMD ["/usr/local/bin/mHttpServer", "--serverPort", "8000", "--htmlFilesFolder", "/app/html", "--dataFilesFolder", "/data"]