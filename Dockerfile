FROM ubuntu:20.04

# Előkészületek
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ca-certificates libhwloc15 && rm -rf /var/lib/apt/lists/*

# Fájlok átmásolása
WORKDIR /app
COPY build_linux_x86_64/aidrig ./aidrig
COPY entrypoint.sh ./entrypoint.sh

# Jogosultság
RUN chmod +x aidrig entrypoint.sh

# Entrypoint
ENTRYPOINT ["./entrypoint.sh"]
