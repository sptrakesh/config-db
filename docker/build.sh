#!/bin/sh

cd `dirname $0`/..
. docker/env.sh

Local()
{
  docker build --compress --force-rm -f docker/Dockerfile -t $NAME .
}

Alpine()
{
  docker buildx build --builder mybuilder --platform "$1" --compress --force-rm -f docker/Dockerfile --push -t sptrakesh/$NAME:$VERSION -t sptrakesh/$NAME:latest .
  docker pull sptrakesh/$NAME:latest
}

Ubuntu()
{
  docker buildx build --builder mybuilder --platform "$1" --compress --force-rm -f docker/Dockerfile.gcc --push -t sptrakesh/$NAME:gcc .
  docker pull sptrakesh/$NAME:gcc
}

if [ -z "$2" ]
then
  PLATFORM='linux/arm64,linux/amd64'
else
  PLATFORM="$2"
fi

case "$1" in
  'alpine')
    Alpine "$PLATFORM"
    ;;
  'ubuntu')
    Ubuntu "$PLATFORM"
    ;;
  'local')
    Local
    ;;
  *)
    Alpine "$PLATFORM"
    Ubuntu "$PLATFORM"
    ;;
esac
