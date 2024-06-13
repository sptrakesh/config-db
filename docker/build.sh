#!/bin/sh

cd `dirname $0`/..
. docker/env.sh

if [ "$1" = "local" ]
then
  docker build --compress --force-rm -f docker/Dockerfile -t $NAME .
else
  docker buildx build --builder mybuilder --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile --push -t sptrakesh/$NAME:$VERSION -t sptrakesh/$NAME:alpine -t sptrakesh/$NAME:latest .
  docker pull sptrakesh/$NAME:latest
  docker pull sptrakesh/$NAME:alpine
  #docker buildx build --builder mybuilder --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile.gcc --push -t sptrakesh/$NAME:gcc .
  #docker pull sptrakesh/$NAME:gcc
fi
