FROM alpine:3.9

RUN apk add wget build-base

RUN wget https://crd.lbl.gov/assets/Uploads/FTG/Projects/APEX/apex-map-v3.0.tar
RUN mkdir apex-map
RUN tar -xf apex-map-v3.0.tar -C apex-map

WORKDIR apex-map
RUN gcc gen.pub.c
RUN ./a.out
RUN gcc Apex.c
CMD ["./a.out"] 
