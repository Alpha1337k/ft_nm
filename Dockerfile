FROM ubuntu:latest

RUN apt-get update && apt-get install file binutils make gcc -y

WORKDIR /app

CMD tail -f /dev/null