FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ make ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Código del servidor (está en /httpserver del repo)
COPY httpserver/ /app/

# Compilar el binario
RUN g++ -O2 -std=c++20 \
    mHttpServer.cpp \
    lib/HttpServer.cpp \
    lib/HttpConnection.cpp \
    lib/PostMethod.cpp \
    -o /usr/local/bin/mHttpServer

EXPOSE 8000

CMD ["/usr/local/bin/mHttpServer", "--serverPort", "8000", "--htmlFilesFolder", "/app/html", "--dataFilesFolder", "/data"]