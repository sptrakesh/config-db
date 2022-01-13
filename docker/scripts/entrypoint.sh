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

  if [ -z "$ENABLE_SSL" ]
  then
    ENABLE_SSL="false"
    echo "ENABLE_SSL not set.  Will default to $ENABLE_SSL"
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
    echo "DEBUG specified, shell into container and start using gdb.sh"
    tail -f /dev/null
  fi

  echo "Starting config-db service"
  if [ -n "$CONFIG_FILE" ]
  then
    /opt/spt/bin/configdb --config-file $CONFIG_FILE
  else
    /opt/spt/bin/configdb --console true --log-dir ${LOGDIR}/ --log-level $LOG_LEVEL \
      --http-port $HTTP_PORT --tcp-port $TCP_PORT --threads $THREADS \
      --enable-cache $ENABLE_CACHE --with-ssl $ENABLE_SSL $args
  fi
}

Defaults && Args && Service