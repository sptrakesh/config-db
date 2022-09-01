#!/bin/sh

cd `dirname $0`/..
. docker/env.sh

if [ "$1" = "local" ]
then
  docker build --compress --force-rm -f docker/Dockerfile -t $NAME .
else
  docker buildx build --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile --push -t sptrakesh/$NAME:$VERSION -t sptrakesh/$NAME:gcc -t sptrakesh/$NAME:latest .
  docker pull sptrakesh/$NAME:latest
  docker pull sptrakesh/$NAME:gcc
  docker buildx build --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile.alpine --push -t sptrakesh/$NAME:alpine .
  docker pull sptrakesh/$NAME:alpine
fi
