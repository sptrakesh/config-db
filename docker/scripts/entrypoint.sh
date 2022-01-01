#!/bin/sh

LOGDIR=/opt/spt/logs

Defaults()
{
  if [ -z "$HTTP_PORT" ]
  then
    HTTP_PORT=6000
    echo "PORT not set.  Will default to $HTTP_PORT"
  fi

  if [ -z "$TCP_PORT" ]
  then
    TCP_PORT=2020
    echo "PORT not set.  Will default to $TCP_PORT"
  fi

  if [ -z "$THREADS" ]
  then
    THREADS=4
    echo "THREADS not set.  Will default to $THREADS"
  fi

  if [ -z "$LOG_LEVEL" ]
  then
    LOG_LEVEL="info"
    echo "LOG_LEVEL not set.  Will default to $LOG_LEVEL"
  fi

  if [ -z "$ENABLE_CACHE" ]
  then
    ENABLE_CACHE="true"
    echo "ENABLE_CACHE not set.  Will default to $ENABLE_CACHE"
  fi
}

Args()
{
  args=''

  if [ -n "$ENCRYPTION_SECRET" ]
  then
    args=" --encryption-secret $ENCRYPTION_SECRET"
    echo "AES encryption secret specified."
  fi
}

Service()
{
  if [ -n "$DEBUG" ]
  then
    echo "DEBUG specified, shell into container and start up with gdb"
    tail -f /dev/null
  fi

  echo "Starting up config-db service"
  /opt/spt/bin/configdb --console true --dir ${LOGDIR}/ --log-level $LOG_LEVEL \
    --http-port $HTTP_PORT --tcp-port $TCP_PORT --threads $THREADS \
    --enable-cache $ENABLE_CACHE $args
}

Defaults && Args && Service