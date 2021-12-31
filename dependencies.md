# Dependencies
Scripts used to install the dependencies on **MacOSX**.

## RocksDB
[RocksDB](https://rocksdb.org/) installed using the following script:

<details>
  <summary>Click to expand!</summary>

```shell
#!/bin/sh

PREFIX=/usr/local/rocksdb
VERSION=6.27.3

cd /tmp
if [ -d rocksdb ]
then
  rm -rf rocksdb gflags
fi

if [ -d $PREFIX ]
then
  sudo rm -rf $PREFIX
fi

(git clone https://github.com/gflags/gflags.git \
  && cd gflags \
  && mkdir build_ && cd build_ \
  && cmake \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_STATIC_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DBUILD_gflags_LIBS=ON \
    -DINSTALL_HEADERS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_INSTALL_LIBDIR=lib \
    .. \
  && make -j12 \
  && sudo make install)

(git clone -b v${VERSION} https://github.com/facebook/rocksdb.git \
  && cd rocksdb \
  && mkdir build && cd build \
  && cmake -DWITH_TESTS=OFF \
    -DWITH_ALL_TESTS=OFF \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_BUILD_TYPE=Release \
    -DROCKSDB_BUILD_SHARED=OFF \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_INSTALL_LIBDIR=lib \
    .. \
  && make -j12 \
  && sudo make install)
```

</details>


## Boost
[Boost](https://boost.org/) installed using the following script:

<details>
  <summary>Click to expand!</summary>

```shell
#!/bin/sh
BOOST_VERSION=1.78.0
INSTALL_DIR=/usr/local/boost

cd /tmp
git clone --branch boost-${BOOST_VERSION} --recursive https://github.com/boostorg/boost.git

sudo rm -rf $INSTALL_DIR
cd boost \
  && ./bootstrap.sh \
  && sudo ./b2 -j8 install link=static threading=multi runtime-link=static --prefix=$INSTALL_DIR --without-python --without-mpi

cd /tmp/boost/tools/build \
  && ./bootstrap.sh \
  && sudo ./b2 -j8 install link=static threading=multi runtime-link=static --prefix=$INSTALL_DIR
```

</details>


## nghttp2
[nghttp2](https://www.nghttp2.org/) installed using the following script:

<details>
  <summary>Click to expand!</summary>

```shell
#!/bin/sh

NGHTTP_VERSION=1.46.0
DEST=/usr/local/nghttp2

cd /tmp
rm -rf nghttp2*
curl -O -L https://github.com/nghttp2/nghttp2/releases/download/v${NGHTTP_VERSION}/nghttp2-${NGHTTP_VERSION}.tar.xz \
&& tar xf nghttp2-${NGHTTP_VERSION}.tar.xz \
&& cd nghttp2-${NGHTTP_VERSION} \
&& CXXFLAGS='-I/opt/local/include' LDFLAGS='-L/opt/local/lib' LIBS='-lssl -lcrypto' ./configure --prefix=$DEST --enable-asio-lib --enable-lib-only --enable-shared=yes --with-boost=/usr/local/boost \
&& make -j8 \
&& sudo rm -rf $DEST \
&& sudo make install
```

</details>


# Flatbuffers
[flatbuffers](https://google.github.io/flatbuffers) installed using the following script:

<details>
  <summary>Click to expand!</summary>

```shell
#!/bin/sh

NAME=flatbuffers
PREFIX=/usr/local/flatbuffers
VERSION=2.0.0

cd /tmp
if [ -d $NAME ]
then
  rm -rf $NAME
fi

if [ -d $PREFIX ]
then
  sudo rm -rf $PREFIX
fi

git clone -b v${VERSION} https://github.com/google/flatbuffers.git \
  && cd $NAME \
  && mkdir build && cd build \
  && cmake -DFLATBUFFERS_BUILD_TESTS=OFF \
    -DFLATBUFFERS_BUILD_CPP17=ON \
    -DFLATBUFFERS_ENABLE_PCH=ON \
    -DCMAKE_PREFIX_PATH=$PREFIX \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_INSTALL_LIBDIR=lib \
    .. \
  && make -j12 \
  && sudo make install
```

</details>
