#!/bin/sh

cd `dirname $0`/..
. docker/env.sh

docker buildx build --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile --push -t sptrakesh/$NAME:$VERSION -t sptrakesh/$NAME:latest .
docker pull sptrakesh/$NAME:latest
