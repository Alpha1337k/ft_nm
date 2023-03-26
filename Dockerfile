FROM ubuntu:latest

RUN apt-get update && apt-get install file binutils make gcc gcc-multilib python3.11 -y

WORKDIR /app

CMD tail -f /dev/null