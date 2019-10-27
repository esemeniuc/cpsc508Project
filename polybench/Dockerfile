FROM alpine:3.9

RUN apk add wget build-base

RUN wget https://downloads.sourceforge.net/project/polybench/polybench-c-4.2.1-beta.tar.gz 
RUN tar -xzf polybench-c-4.2.1-beta.tar.gz
WORKDIR polybench-c-4.2.1-beta
# Compile one (or more) of the polybench benchmarks
RUN gcc -O3 -I utilities -I linear-algebra/kernels/atax utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -o atax_time
# Run the compiled polybench benchmark 
CMD ["./atax_time"]
