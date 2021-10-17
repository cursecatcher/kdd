FROM gcc:9.3.0

RUN apt update && \
    apt install -y \
        autoconf \
        automake \
        libtool \
        libgmp3-dev \
        make \
        time 

#install meddly
COPY meddly /opt/meddly 
WORKDIR /opt/meddly
RUN ./autogen.sh && \
    ./configure CXXFLAGS="-O3" --without-gmp && \
    make && \
    make install && \ 
    make clean 


# RUN apt install -y cmake

# # COPY gatb-core /opt/gatb-core 
# # WORKDIR /opt/gatb-core
# # RUN rm -rf build && \
# #     mkdir build && \
# #     cd build && \
# #     cmake .. && \
# #     make  

COPY *.cpp *.hpp Makefile /opt/
WORKDIR /opt
RUN make ust_dd && mv ust_dd /usr/local/bin/ 
# RUN make kdd && mv kdd /usr/local/bin/
# RUN make kmers && mv kmers /usr/local/bin/



#grapes-dd stuff 
# COPY src/ /opt/src/
# WORKDIR /opt/src 
# RUN make all && \
#     make clean && \
#     cp grapes_dd dd_one heu orderings /usr/bin

# ENTRYPOINT [ "/opt/src/grapes_dd" ]


#ENTRYPOINT [ "grapes_dd" ]


