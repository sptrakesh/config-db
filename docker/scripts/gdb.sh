#!/bin/sh

CACHE_DIR="/opt/spt/data"
LOGDIR=/opt/spt/logs
HTTP_PORT=6000
TCP_PORT=2020
THREADS=4
gdb -ex run --args /opt/spt/bin/configdb --console true --dir ${LOGDIR}/ --log-level $LOG_LEVEL \
    --http-port $HTTP_PORT --tcp-port $TCP_PORT --threads $THREADS
