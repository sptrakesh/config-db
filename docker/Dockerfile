FROM sptrakesh/rocksdb as rocksdb
FROM sptrakesh/flatbuffers as flatbuffers
FROM sptrakesh/nghttp2 as base

COPY --from=rocksdb /opt/local /opt/local
COPY --from=flatbuffers /opt/local /opt/local
RUN apk add --no-cache readline-dev snappy-dev

COPY CMakeLists.txt /opt/spt/config-db/
COPY ConfigDbConfig.cmake /opt/spt/config-db/
COPY src /opt/spt/config-db/src
COPY certs /opt/spt/config-db/certs
COPY docker/env.sh /opt/spt/config-db/docker/

WORKDIR /opt/spt/config-db/build
RUN rm -rf * \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/local -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=mold" .. \
  && make -j8 \
  && make install

FROM sptrakesh/cppruntime

RUN apk add --no-cache libcurl snappy readline \
  && addgroup spt \
  && adduser -DS -g "SPT User" -G spt -s /bin/sh -h /home/spt spt \
  && mkdir -p /opt/spt/logs /opt/spt/data \
  && chown spt:spt /opt/spt/logs /opt/spt/data

COPY --from=base /opt/spt/bin /opt/spt/bin
COPY --from=base /opt/spt/include /opt/spt/include
COPY --from=base /opt/spt/lib /opt/spt/lib
COPY docker/scripts/*.sh /opt/spt/bin/
COPY certs /opt/spt/certs

WORKDIR /opt/spt
ENV LD_LIBRARY_PATH=/usr/lib \
  PATH=$PATH:/opt/spt/bin
USER spt
ENTRYPOINT [ "/opt/spt/bin/entrypoint.sh" ]