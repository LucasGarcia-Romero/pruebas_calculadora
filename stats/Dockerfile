FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    coreutils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY stats/temp_monitor.sh /app/temp_monitor.sh

RUN chmod +x /app/temp_monitor.sh

ENTRYPOINT ["/app/temp_monitor.sh"]