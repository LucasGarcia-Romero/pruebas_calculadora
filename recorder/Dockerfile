FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    sox \
    alsa-utils \
    ca-certificates \
    bash \
    coreutils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY recorder/record.sh /app/record.sh
COPY recorder/entrypoint.sh /app/entrypoint.sh

# Binarios ARM64 precompilados
COPY spectrogram/ /app/spectrogram/
COPY DHT22/ /app/DHT22/

RUN chmod +x /app/record.sh /app/entrypoint.sh \
 && chmod +x /app/spectrogram/spectrogram \
 && chmod +x /app/DHT22/dht22.out

ENTRYPOINT ["/app/entrypoint.sh"]