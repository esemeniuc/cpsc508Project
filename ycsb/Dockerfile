FROM alpine:3.9

RUN apk add curl build-base

RUN curl -O --location https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz
RUN tar xfvz ycsb-0.17.0.tar.gz
RUN cd ycsb-0.17.0

