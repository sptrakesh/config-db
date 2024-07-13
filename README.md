# Configuration Database

* [Keys](#keys)
* [Protocols](#protocols)
  * [TCP/IP](#tcpip)
  * [HTTP/2](#http2)
    * [Headers](#custom-headers)
* [Messages](#messages)
  * [Request](#request)
  * [Response](#response)
  * [Ping](#ping)
* [API](#api)
* [Utilities](#utilities)
  * [Shell](#shell)
  * [CLI](#cli)
    * [Bulk Import](#bulk-import)
  * [Seed](#seed)
* [Docker](#docker)
* [Build](#build)
  * [Windows](#windows)
  * [API Usage](#api-usage)
* [Run](#run)
  * [Environment Variables](#environment-variables)
  * [Notifications](#notifications)
  * [SSL](#ssl)
* [Acknowledgements](#acknowledgements)

A simple configuration database similar to Apache Zookeeper, Etcd, etc., built
using [RocksDB](https://rocksdb.org/).

All values are stored encrypted using `aes-256-cbc` on disk.


## Keys
All keys are assumed to represent UNIX *path like* structures.  Keys that do
not at least start with a leading `/` character are internally replaced with
a leading `/` character.  This means that keys `key` and `/key` are treated
as identical.

The database maintains a hierarchical model of the keys.  It is possible to
list the child node names under each level in the hierarchy.

**Note:** When listing child node names, the full path of the parent *has* to
be specified.  Relative path names are not supported.

## Implementation
Key-value pairs are stored in a RocksDB column family.  A few addition column families are
used to provide support for additional features provided.
* **data** - The main column family used to store the *key->value* mapping.  The *value* is
  stored encrypted.
* **forrest** - The column family used to store the *tree* hierarchy.
* **expiration** - The column family used to store TTL values for expiring keys.
* **expiring** - The column family used to efficiently determine expired keys.  The TTL value
  (along with a value for disambiguation) is stored as the *key* and the data *key* is stored
  as the *value* in this column.  RocksDB keys are stored sorted, hence using the expiration
  time as the *key* allows for efficiently breaking from the loop when the first non-expired
  key is encountered.  A custom *iterator* is used to retrieve only the *expired* keys from
  this column.
* **cache** - The column family used to indicate that the specified *key* is stored as
  a *cache* entry.

### Backup
Daily backups are performed and saved under the `storage.backupPath` configured.  The maximum
number of previous backups to maintain can be configured using the `storage.maxBackups` property.
RocksDB makes incremental backups, and the cost of initialising the *BackupEngine* is proportional
to the number of old backups.

### Peering
Each instance is designed to run independently.  It is possible to configure instances
to run as a *co-ordinated cluster* by configuring *peers* for each instance.  There is
no complicated leadership election process/protocol using **Raft** or similar consensus
algorithms.  Commands received by each node is relayed to peers.  This helps all nodes
in the cluster remain in sync with each other eventually.

## Commands
The following *actions/commands* are supported by the service:
* **Put** - Set a *key-value* pair.  The *key* should comply with the above
  mentioned scheme.  The *value* is an arbitrary length value.  Can be large
  values such as JSON, BSON, base64 encoded binary data etc. as appropriate.
* **Get** - Get a stored *value* for the specified *key*.
* **Move** - Move the *value* associated with a *key* to a new *destination*.
  Convenience command to perform a `Get-Delete-Put` in a single transaction.
* **Delete** - Delete the *key-value* pair (and path components as appropriate)
  identified by the *key*.
* **List** - List child node names under a specified *parent path*.  Leaf nodes
  will return an error.  **Note** that parent paths are recursively removed during
  a *Delete* action if no other child nodes exist.
* **TTL** - Retrieve the TTL for the specified *key*.  The returned value indicates
  the remaining time relative to the request time, not the original value.  If the
  *key* does not have a TTL policy, `0` is returned.


## Protocols
Service supports both TCP/IP and HTTP/2 connections.  The TCP service uses
[flatbuffers](https://google.github.io/flatbuffers/) as the data interchange format, while the HTTP/2 service uses JSON.

The models were generated from the schema files using the following command:

```shell
(cd <path to project>/src/common/model;
<path to>/flatc --cpp --cpp-std c++17 --cpp-static-reflection --reflect-names request.fbs response.fbs tree.fbs)
```

### TCP/IP
The TCP/IP service uses flatbuffers for data interchange.  The wire protocol
adds an extra 4 bytes at the beginning of each message.  These 4 bytes contain
the size of the message being interchanged.  Client and server will use these
4 bytes to ensure they have read the entire message before attempting to marshall
the data.

All requests are for setting or retrieving multiple *keys*.  When setting or
deleting multiple keys, a single failure will result in rolling back the entire
*transaction*.

### HTTP/2
The HTTP service uses JSON for data interchange.  HTTP/1.1 or upgrade to 2 is
not supported directly.  Clients wishing regular HTTP/1.1 or upgrade feature
can use [envoy](https://www.envoyproxy.io/) to handle these as well as `https`.

The HTTP service always responds with a JSON message.  Input message is only
supported on the `PUT` endpoints, and only accept a plain text body which is
the desired value to set for the key.

The following path prefixes are supported by the service:
* URI paths with the `/key` prefix.  The rest of the path is interpreted
as the desired *key* within the configuration database.  This is used to manage
the configuration data.
* URI paths with the `/list` prefix.  The rest of the path is interpreted
as the desired *key* within the configuration database.  This is used to
retrieve child node names for the specified path.

See [shell](test/integration/curl.sh) and/or [cpr](test/integration/http.cpp) for sample HTTP requests and responses.

#### Custom headers
Request options as defined in the flatbuffers schema can be specified using
custom HTTP headers when making request to the HTTP/2 service.
* `x-config-db-if-not-exists` - Use to specify the `if_not_exists` property.
  A value of `true` is interpreted as the boolean value `true`.  Any other values
  are interpreted as `false`.  See the `PutNotExists` function in the example.
* `x-config-db-ttl` - Use to specify the `expiration_in_seconds` property.  Must
  be a numeric value, and represent the expiration from current time in *seconds*.
* `x-config-db-cache` - Use to specify the `cache` property. A value of `true` is 
  interpreted as the boolean value `true`.  Any other values are interpreted as `false`.

**Note:** The HTTP service does not support batch (multiple *key*) operations.

**Note:** The HTTP service does not support *move* operations.

#### PUT
```shell
# PUT request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XPUT -H "content-type: text/plain" -d "value" $url
```
```json
{"code": 200, "cause": "Ok"}
```

#### PutNotExists
```shell
# PUT request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XPUT -H "content-type: text/plain" -H "x-config-db-if-not-exists: true" -d "value" $url
```
```json
{"code": 412, "cause": "Unable to save"}
```

#### PutWithTTL
```shell
curl -s --http2-prior-knowledge -XPUT \
  -H "content-type: text/plain" \
  -H "x-config-db-if-not-exists: true" \
  -H "x-config-db-ttl: 5" \
  -d "value" 'http://localhost:6006/key/test'
```
```json
{"code": 200, "cause": "Ok"}
```

#### GET
```shell
# GET request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge $url
```
```json
{"key":"/test","value":"value"}
```

#### DELETE
```shell
# DELETE request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XDELETE $url
```
```json
{"code": 200, "cause": "Ok"}
```

#### GET after delete
```shell
# GET request after delete
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge $url
```
```json
{"code": 404, "cause": "Not found"}
```

## Messages
The following messages are transferred between the client and TCP server.

### Request
The [request](src/common/model/request.fbs) message contains the `action` desired
as well as the `key-value` pairs to be sent to the service.  The `value` may
be omitted for all actions other than `Put`.

* **action** - An `enum` used to specify the type of action to perform against
  the service.  Action values are kept similar to their HTTP verb counterparts
  when possible.
* **data** - Array of `key-value` pairs.
  * **key** - The *key* to act upon against the service.
  * **value** - The *value* to set for the *key*.  This is only relevant for
    the `Put` or `Move` actions.  In the case of `Move`, the *value* is the destination
    *key* to move the *value* to.
  * **options** - Additional options that relate to setting (or moving) *keys*.
    * **if_not_exists** - Specify `true` if *key* should be saved only if it does
      not exist in the database.  For `Move`, this applies to the *value* which
      serves as the destination *key*.
    * **expiration_in_seconds** - TTL in seconds from request time.  The *key* will
      be automatically deleted by the system after the specified time has elapsed.
    * **cache** - Indicate that the *key-value* pair is to be treated as a *cache* entry.
      Cached keys are maintained in the *tree* structure, and are meant to be used purely
      as temporary values that can be looked up by *unique* key.  Cached keys **must** have
      a TTL.

### Response
The [response](src/common/model/response.fbs) message contains either the *value*
for the array of *keys* (`Get`, `List`), or a `boolean` indicating success or failure of the 
*transaction* (`Put`, `Delete`).

A **KeyValueResult** structure is used to represent the result for a specific
*key* sent in the request.  The `value` can be one of:
* **Value** - The string value associated with the specified *key* for a `Get` request.
* **Children** - A list of child node names for the specified *key/path* for a `List` request.
* **Success** - A boolean value used to report failure at retrieving the `key` or `path`.

When reading the response, as mentioned above, the first **4** bytes represent
the length of the buffer being returned, and the rest of the bytes (use loops
and similar constructs to read until the full message has been received) will
be the buffer.  See [integration test](test/integration/tcp.cpp) for an
example.

### Ping
Short (less than 5 bytes) messages may be sent to the service as a *keep-alive*
message.  Service will echo the message back to the client.  Examples include
`ping`, `noop`, etc.


## API
A high-level client API to interact with the service is provided.  The interface hides
the complexities involved with making TCP/IP requests using flatbuffers.  The
[api](src/api/api.h) presents an interface that is very similar to the persistence
interface used internally by the service.  The API maintains a connection pool
to the service and performs the required interactions using the flatbuffer models.

**Note:** API **must** be initialised via the `init` function before first use.

See [integration test](test/integration/apicrud.cpp) for sample usage of the API.
The *shell* application is built using the client API.
See [cmake](#api-usage) for including the API in your cmake project.


## Utilities
Utility applications to interact with the database are provided.  These are
deployed to the `/opt/spt/bin` directory.

### Shell
The `configsh` application provides a *shell* to interact with the database.
The server should be running and the TCP port open for the application to connect.

The following command line options are supported by the shell:
* `-s | --server` The TCP server hostname to connect to.  Default `localhost`.
* `-p | --port` The TCP port to connect to.  Default `2020` (`2022` on Mac OS X).
* `-t | --with-ssl` Flag to connect over SSL to the server.
* `-l | --log-level` The level for the logger.  Accepted values `debug|info|warn|critical`.  Default `info`.
* `-o | --log-dir` The directory to write log files to.  The path *must* end with a trailing `/`.  Default `/tmp/`.

The following shows a simple CRUD type interaction via the shell.

<details>
  <summary><strong>Click to expand!</strong></summary>

```shell
/opt/spt/bin/configsh --server localhost --port 2022 --log-level debug --log-dir /tmp/
Enter commands followed by <ENTER>
Enter help for help about commands
Enter exit or quit to exit shell
configdb> help
Available commands
  ls <path> - To list child node names.  Eg. [ls /]
  get <key> - To get configured value for key.  Eg. [get /key1/key2/key3]
  set <key> <value> - To set value for key.  Eg. [set /key1/key2/key3 Some long value. Note no surrounding quotes]
  mv <key> <destination> - To move value for key to destination.  Eg. [mv /key1/key2/key3 /key/key2/key3]
  rm <key> - To remove configured key.  Eg. [rm /key1/key2/key3]
  import <path to file> - To bulk import key-values from file.  Eg. [import /tmp/kvps.txt]
configdb> set /key1/key2/key3 {"glossary":{"title":"example glossary","GlossDiv":{"title":"S","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["GML","XML"]},"GlossSee":"markup"}}}}}
Set key /key1/key2/key3
configdb> ls /
key1
configdb> ls /key1
key2
configdb> ls /key1/key2
key3
configdb> get /key1/key2/key3
{"glossary":{"title":"example glossary","GlossDiv":{"title":"S","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["GML","XML"]},"GlossSee":"markup"}}}}}
configdb> set /key1/key2/key3 some long value with spaces and "quoted" text.
Set key /key1/key2/key3
configdb> get /key1/key2/key3
some long value with spaces and "quoted" text.
configdb> rm /key1/key2/key3
Removed key /key1/key2/key3
configdb> ls /
Error retrieving path /
configdb> import /tmp/import.txt
Imported (5/5) keys from file /tmp/import.txt
configdb> exit
Bye
```

</details>

### CLI
The `configctl` application provides a simple means for interacting with the
database server.  Use it to execute single actions against the service when
required.

**Note:** Unlike the **shell** application, values specified as command line
argument must be quoted if they contain spaces or other special characters.

The server should be running and the TCP port open for the application to connect.

The following command line options are supported by the CLI application:
* `-s | --server` The TCP server hostname to connect to.  Default `localhost`.
* `-p | --port` The TCP port to connect to.  Default `2020` (`2022` on Mac OS X).
* `-t | --with-ssl` Flag to connect over SSL to the server.
* `-l | --log-level` The level for the logger.  Accepted values `debug|info|warn|critical`.  Default `info`.
* `-o | --log-dir` The directory to write log files to.  The path *must* end with a trailing `/`.  Default `/tmp/`.
* `-f | --file` The file to bulk import into the database.  If specified, other commands are ignored.
* `-a | --action` The action to perform.  One of `get|set|move|delete|list`.
* `-k | --key` The *key* to act upon.
* `-v | --value` The *value* to `set`.  For `move` this is the destination path.

See [sample](test/integration/configctl.sh) integration test suite.

The following shows a simple CRUD type interaction via the cli. These were using
the default values for `server [-s|--server]` and `port [-p|--port]` options.

<details>
  <summary><strong>Click to expand!</strong></summary>

```shell
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /
Error retrieving path /
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v value
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /            
test
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v value
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v "value modified"                      
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a get -k /test 
value modified         
spt:/home/spt $ /opt/spt/bin/configctl -a move -k /test -v /test1
Moved key /test to /test1
spt:/home/spt $ /opt/spt/bin/configctl -a delete -k /test1
Removed key /test1
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /
Error listing path /
```

</details>

#### Bulk Import
A special `-f | --file` option is supported for bulk importing *key-value* pairs
from a file.  The input file must contain lines where each line represents a
*key-value* pair.  The text before the first space character is interpreted as
the *key*, and the rest of the line as the *value*.

```shell
/a/b/c string value
/a/b/d 13
```

**Note:** The entire file is imported as a single transaction.  The input file
must be small enough to buffer the requests and transaction in memory.

<details>
  <summary><strong>Click to expand!</strong></summary>

```shell
spt:/home/spt $ cat /tmp/import.txt
/a/b/c  a long string
/a/b/d another long value
/a/c/e  1234
/a/c/f 45765789
/a/c/g 123.347
spt:/home/spt $ /opt/spt/bin/configctl -f /tmp/import.txt
Set 5 keys
```

</details>

### Seed
A `seed-configdb` utility is provided to *seed* the database.  This can be used
to pre-populate the database before the service is started.

**Note:** This will fail if the service is already running (database locked
by running process).

The following command line options are supported by the seed application:
* `-c | --conf` Optional JSON configuration file that controls the database storage location.
* `-f | --file` The data file to import.  The input file must contain lines 
  where each line represents a *key-value* pair.  The text before the first 
  space character is interpreted as the *key*, and the rest of the line as the *value*.
* `-l | --log-level` The level for the logger.  Accepted values `debug|info|warn|critical`.  Default `info`.
* `-o | --log-dir` The directory to write log files to.  The path *must* end with a trailing `/`.  Default `/tmp/`.


## Docker
Docker images are available on [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/config-db).

The database files are stored under `/opt/spt/data`.  If you wish to persist
the database, volume mount this location.

It is possible to run the integration tests against the docker container instead
of the service running locally.

```shell
docker run -d --rm -p 6026:6020 -p 2022:2020 --name config-db sptrakesh/config-db
# or with SSL turned on
docker run -d --rm -p 6026:6020 -p 2022:2020 -e "ENABLE_SSL=true" --name config-db sptrakesh/config-db
```

The following custom environment variables can be used to customise the container (the standard [variables](#environment-variables)
also work):
* `CONFIG_FILE` - The JSON file (volume mount) with full configuration.  All
  other variables/options are ignored.
* `HTTP_PORT` - The port to run the HTTP/2 service on.  Default `6020`.
* `TCP_PORT` - The port to run the TCP service on.  Default `2020`.
* `NOTIFY_PORT` - The port to run the notification service on.  Default `2120`
* `THREADS` - The number of threads to use for the services.  Default `4`.
* `LOG_LEVEL` - The level to use for logging.  One of `debug|info|warn|critical`.  Default `info`.
* `ENABLE_CACHE` - Use to turn off temporary value caching.  Default `true`.
* `ENABLE_SSL` - Use to run SSL services.  Default `false`.
* `ENCRYPTION_SECRET` - Use to specify the secret used to AES encrypt values.  Default is internal to the system.
* `PEERS` - Use to enable notifications on the `NOTIFY_PORT`.  Listeners will be
  started to listen for notifications from the `PEERS`.  Will also enable publishing
  notifications from this instance.

**Note:** An alpine based image is also available.  It crashes at shutdown, so
the database may get corrupted.

## Build
Standard cmake build procedure.  See [Dockerfile](docker/Dockerfile) for build
sequence.  Ensure the dependencies are available under the following paths:
* **MacOSX** - Various dependencies installed under the `/usr/local/<dependency>` path.
  See [dependencies](dependencies.md) for scripts used to install the dependencies.
* **UNIX** - All dependencies installed under the `/opt/local` path.
* **Windows** - Most dependencies installed under the `\opt\local` path.
  A few dependencies also installed via `vcpkg` under `\opt\src\vcpkg`.

### UNIX
Check out the project and build.  Install [dependencies](dependencies.md).  These instructions are for setting up on Mac OS X.  For Linux,
the assumption is that all dependencies are installed under `/opt/local`.

```shell
git clone git@github.com:sptrakesh/config-db.git
cd config-db
cmake -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_PREFIX_PATH=/usr/local/rocksdb \
  -DCMAKE_PREFIX_PATH=/usr/local/flatbuffers \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
```

### Windows

Install pre-requisites for building and running on Windows.   The following instructions at times reference `arm` or `arm64`
architecture.  Modify those values as appropriate for your hardware.  These instructions are based on steps I followed to set 
up the project on a Windows 11 virtual machine running via Parallels Desktop on a M2 Mac.

#### Issues
At present final linking of the *service* fails with unresolved external symbols.  These have not manifested themselves on
Mac OS X.  I will update here if I finally manage to resolve the linking problems.
* `rocksdb::Cleanable::Cleanable(void)`
* `rocksdb::PinnableSlice::PinnableSlice(class rocksdb::PinnableSlice &&)`
* `rocksdb::Status::ToString(void)const`
* `rocksdb::LRUCacheOptions::MakeSharedCache(void)const`
* `rocksdb::ColumnFamilyOptions::OptimizeForPointLookup(unsigned __int64)`
* `rocksdb::ColumnFamilyOptions::ColumnFamilyOptions(void)`
* `rocksdb::DBOptions::OptimizeForSmallDb(class std::shared_ptr<class rocksdb::Cache> *)`
* `rocksdb::DBOptions::DBOptions(void)`
* `rocksdb::TransactionDB::Open(struct rocksdb::DBOptions const &,struct rocksdb::TransactionDBOptions const &,class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > const &,class std::vector<struct rocksdb::ColumnFamilyDescriptor,class std::allocator<struct rocksdb::ColumnFamilyDescriptor> > const &,class std::vector<class rocksdb::ColumnFamilyHandle *,class std::allocator<class rocksdb::ColumnFamilyHandle *> > *,class rocksdb::TransactionDB **)`
* `rocksdb::kDefaultColumnFamilyName`

<details>
<summary>Install <a href="https://boost.org/">Boost</a></summary>

* Download and extract Boost 1.82 (or above) to a temporary location (eg. `\opt\src`).
* Launch the Visual Studio Command utility and cd to the temporary location.
```shell
cd \opt\src
curl -OL https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz
tar -xfz boost_1_82_0.tar.gz
cd boost_1_82_0
.\bootstrap.bat
.\b2 -j8 install threading=multi address-model=64 architecture=arm asynch-exceptions=on --prefix=\opt\local --without-python --without-mpi

cd ..
del /s /q boost_1_82_0
rmdir /s /q boost_1_82_0
```
</details>

<details>
<summary>Install <a href="https://github.com/fmtlib/fmt">fmt</a> library</summary>

Launch the Visual Studio Command utility and cd to a temporary location.
```shell
cd \opt\src
git clone https://github.com/fmtlib/fmt.git --branch 9.1.0
cd fmt
cmake -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=\opt\local -DCMAKE_INSTALL_LIBDIR=lib -DFMT_TEST=OFF -DFMT_MODULE=ON -S . -B build
cmake --build build --target install -j8
```
</details>

<details>
<summary>Install <a href="https://github.com/ericniebler/range-v3">range-v3</a> library</summary>

Launch the Visual Studio Command utility and cd to a temporary location.
```shell
git clone https://github.com/ericniebler/range-v3.git --branch 0.12.0
cd range-v3
cmake -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=\opt\local -DCMAKE_INSTALL_LIBDIR=lib -DRANGE_V3_DOCS=OFF -DRANGE_V3_EXAMPLES=OFF -DRANGE_V3_PERF=OFF -DRANGE_V3_TESTS=OFF -DRANGE_V3_INSTALL=ON -B build -S .
cmake --build build --target install -j8
```
</details>

<details>
<summary>Install <a href="https://github.com/google/flatbuffers">flatbuffers</a></summary>

```shell
cd \opt\src
git clone -b v23.5.9 https://github.com/google/flatbuffers.git
cd flatbuffers
cmake -DFLATBUFFERS_BUILD_TESTS=OFF -DFLATBUFFERS_BUILD_CPP17=ON -DFLATBUFFERS_ENABLE_PCH=ON -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -S . -B build
cmake --build build --target install -j8
cd ..
del /s /q flatbuffers
rmdir /s /q flatbuffers
```

</details>

<details>
<summary>Install <a href="https://vcpkg.io/">vcpkg</a></summary>

```shell
cd \opt\src
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install --vcpkg-root \opt\src\vcpkg
.\vcpkg install openssl:arm64-windows
.\vcpkg install snappy:arm64-windows
.\vcpkg install rocksdb:arm64-windows
.\vcpkg install readline:arm64-windows
```

</details>

<details>
<summary>Install gflags if compiling RocksDB locally</summary>

```shell
cd \opt\src
git clone https://github.com/gflags/gflags.git
cd gflags
cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_gflags_LIBS=ON -DINSTALL_HEADERS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -S . -B cmake-build
cmake --build cmake-build --target install -j8
cd ..
del /s /q gflags
rmdir /s /q gflags
```

</details>

<details>
<summary>Build <a href="https://github.com/facebook/rocksdb">RocksDB</a> locally</summary>

```shell
cd \opt\src
git clone -b v8.3.2 https://github.com/facebook/rocksdb.git
cd rocksdb
cmake -DWITH_TESTS=OFF -DWITH_ALL_TESTS=OFF -DWITH_BENCHMARK_TOOLS=OFF -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Release -DROCKSDB_BUILD_SHARED=OFF -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\local -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -S . -B build -G"Unix Makefiles" -DCMAKE_MAKE_PROGRAM=nmake
set CL=/MP
cmake --build build
cd ..
del /s /q rocksdb
rmdir /s /q rocksdb
```

Note that the current `cmake` build files do not generate any appropriate *install* targets, hence the earlier instruction
to install via `vcpkg`.
</details>

<details>
<summary>Build and test the project</summary>

Launch the Visual Studio Command utility.

**Note:** I have not had any success attempting to build `nghttp2-asio` on Windows ARM64, hence the `-DHTTP_SERVER=OFF`
option is required when invoking `cmake`.

```shell
cd %homepath%\source\repos
git clone https://github.com/sptrakesh/config-db.git
cd config-db
cmake -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\spt -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -DHTTP_SERVER=Off -S . -B build
cmake --build build --target install -j8
```
</details>

### API Usage
The [API](src/api/api.h) can be used to communicate with the TCP service.  Client code bases
can use cmake to use the library.

```shell
# In your CMakeLists.txt file
find_package(ConfigDb REQUIRED COMPONENTS api)
if (APPLE)
  include_directories(/usr/local/spt/include)
else()
  include_directories(/opt/spt/include)
endif (APPLE)
target_link_libraries(${Target_Name} PRIVATE configdb::api ...)

# Run cmake
cmake -DCMAKE_PREFIX_PATH="/usr/local/boost;/usr/local/spt" -S . -B build
cmake --build build -j12
```

## Run
Run the service via the `/opt/spt/bin/configdb` executable.  Command line options
may be specified to override default options.  When running as a Docker container,
use environment variables to specify the command line options.
* **-f | --config-file** - The path to the JSON configuration file.  All other
  options are ignored.  This option provides total control over all configurable
  options including encryption. File must have same structure as the 
  [struct](src/common/model/configuration.h).  See [test](test/unit/configuration.cpp)
  for sample JSON configuration.  For Docker specify the `CONFIG_FILE` 
  environment variable.
* **Logging** - Options related to logging.
  * **-c | --console** - Flag that controls whether logs are echo'ed to `stdout`.
    Default is off.  Always specified in the Docker [entrypoint](docker/scripts/entrypoint.sh).
    No value (`true`, `false`) etc. must be specified.
  * **-l | --log-level** - The log level to set.  Default `info`.  Supported values
    `critical|warn|info|debug`.  Specify via `LOG_LEVEL` environment variable to docker.
  * **-o | --log-dir** - The directory under which log files are written.  Default
    `logs/` relative path. On docker this is set to `/opt/spt/logs`. Files
    are rotated daily.  External scripts (`cron` etc. are needed to remove old files).
* **-p | --http-port** - The port on which the HTTP/2 service listens.  Default
  `6020` (`6026` on Apple).  Specify via `HTTP_PORT` environment variable to docker.
* **-t | --tcp-port** - The port on which the TCP/IP service listens.  Default
  `2020` (`2022` on Apple).  Specify via `TCP_PORT` environment variable to docker.
* **-b | --notify-port** - The port on which notifications are published.  Default
  `2120` (`2122` on Apple).  Specify via `NOTIFY_PORT` environment variable to docker.
* **-s | --with-ssl** - Flag to enable SSL on the HTTP/2 and TCP services.  
  See [SSL](#ssl) for details.  Specify via `ENABLE_SSL` environment variable to docker.
* **-n | --threads** - The number of threads for both TCP/IP and HTTP/2 services.
  Default to number of hardware threads. Specify via `THREADS` environment variable to docker.
* **-e | --encryption-secret** - The secret to use to encrypt values.  Default
  value is internal to the system.  Specify via `ENCRYPTION_SECRET` environment
  variable to docker.
* **-x | --enable-cache** - Flag to enable temporary cache for keys read from the database.
  Default `off`.  Specify via `ENABLE_CACHE` environment variable to docker.
* **-z | --peers** - A comma separated list of peer instances to listen for notifications.
  Specify via the `PEERS` environment variable to docker. Eg. `localhost:2123,localhost:2124`

Sample command to run the service
```shell
# Locally built service
/opt/spt/bin/configdb --console --log-dir /tmp/ --threads 4 --log-level debug
# Docker container
docker run -d --rm -p 6020:6020 -p 2022:2020 \
  -e "ENCRYPTION_SECRET=svn91sc+rlZXlIXz1ZrGP4m3OgznyW5DrWONGjYw4bc=" -e "LOG_LEVEL=debug" \
  --name config-db config-db
```

### Environment Variables
Service [configuration](src/common/model/configuration.h) can be customised using environment variables.
All properties can be specified using a *snake case* convention with the common root `CONFIG_DB_` prefix.

* `CONFIG_DB_THREADS` - use to set the value of the `threads` field.
* `CONFIG_DB_ENABLE_CACHE` - use to set the value of the `enableCache` field.  Set to `false` to disable (default).  Set to `true` to enable.
* `CONFIG_DB_ENCRYPTION_SALT` - use to set the value of the `encryption.salt` field.
* `CONFIG_DB_ENCRYPTION_KEY` - use to set the value of the `encryption.key` field.
* `CONFIG_DB_ENCRYPTION_IV` - use to set the value of the `encryption.iv` field.
* `CONFIG_DB_ENCRYPTION_SECRET` - use to set the value of the `encryption.secret` field.
* `CONFIG_DB_ENCRYPTION_ROUNDS` - use to set the value of the `encryption.rounds` field.
* `CONFIG_DB_LOGGING_LEVEL` - use to set the value of the `logging.level` field.
* `CONFIG_DB_LOGGING_DIR` - use to set the value of the `logging.dir` field.
* `CONFIG_DB_LOGGING_CONSOLE` - use to set the value of the `logging.console` field.
* `CONFIG_DB_SERVICES_HTTP` - use to set the value of the `services.http` field.
* `CONFIG_DB_SERVICES_TCP` - use to set the value of the `services.tcp` field.
* `CONFIG_DB_SERVICES_NOTIFY` - use to set the value of the `services.notify` field.
* `CONFIG_DB_PEERS[0..6]_HOST` - use to set the value of the `peers[n].host` field (eg. `CONFIG_DB_PEERS0_HOST`).
* `CONFIG_DB_PEERS[0..6]_PORT` - use to set the value of the `peers[n].port` field.
* `CONFIG_DB_STORAGE_DBPATH` - use to set the value of the `storage.dbpath` field.
* `CONFIG_DB_STORAGE_BACKUP_PATH` - use to set the value of the `storage.backupPath` field.
* `CONFIG_DB_STORAGE_BLOCK_CACHE_SIZE_MB` - use to set the value of the `storage.blockCacheSizeMb` field.
* `CONFIG_DB_STORAGE_CLEAN_EXPIRED_KEYS_INTERVAL` - use to set the value of the `storage.cleanExpiredKeysInterval` field.
* `CONFIG_DB_STORAGE_ENCRYPTER_INITIAL_POOL_SIZE` - use to set the value of the `storage.encrypterInitialPoolSize` field.
* `CONFIG_DB_STORAGE_MAX_BACKUPS` - use to set the value of the `storage.maxBackups` field.
* `CONFIG_DB_STORAGE_USE_MUTEX` - use to set the value of the `storage.useMutex` field.

### Notifications
A notification system is available when services are run in a cluster.  There
is no complicated leadership election process (using Raft or similar) for cluster 
management/coordination.  Each node is designed to run *stand-alone*.  *Peer* instances are
assumed to run independently (multi-master setup).  A simple notification system
has been implemented which will attempt to keep the independent nodes in sync.
Notifications are primarily useful when using a cluster of instances that are
also used as a L1/L2 cache on top of application databases.  Notifications are
sent asynchronously and hence will only achieve eventual consistency in the best
case.

When operating in multi-master mode, it is important to configure each instance
with the appropriate list of *peers*.  See [integration test](test/integration/multimaster.cpp)
for sample set up and test suite.

Each instance will publish updates (`Put`, `Delete`, and `Move`) via the notification
service.  Corresponding *listener* instances will listen to notifications from
the *peers*.

Notifications are strictly *one-way*.  Notification service only writes
messages to the socket, and the listener instances only read messages from the
socket.  Errors encountered while applying updates on another node do not have
any effect on the publishing node.

Notification service sends periodic *ping* messages to keep the socket connections
alive.


### SSL
SSL wrappers are enabled for both the TCP and HTTP/2 services.  The package includes
self-signed certificates.  The [Makefile](certs/Makefile) can be modified to
generate self-signed certificates for your purposes.  The easier way to override
the certificates is to volume mount the `/opt/spt/certs` directory when running
the docker container.

At present, the file names are hard-coded:
* `ca.crt` - The root CA used to verify.
* `server.crt` - The server certificate file.
* `server.key` - The server key file.
* `client.crt` - The client certificate file.
* `client.key` - The client key file.

**Note:** There is definitely an overhead when using SSL.  The integration test
suite that ran in less than `100ms` now takes about `900ms`.  With `4096`
bit keys, it takes about `1.3s`.

**Note:** The TCP service only supports [TLS 1.3](https://www.ietf.org/blog/tls13/)


## Acknowledgements
The following components are used to build this software:
* **[RocksDB](http://rocksdb.org/)** - The database used to persist configuration.
* **[OpenSSL](https://www.openssl.org/)** - For AES encryption of the values stored in the database.
* **[Boost](https://github.com/boostorg/boost)** - In particular **Asio** for the `TCP socket` server implementation.
* **[nghttp2](https://www.nghttp2.org/)** - for the HTTP/2 server implementation.
* **[flatbuffers](https://google.github.io/flatbuffers)** - The data interchange format for client-server TCP/IP interactions.
* **[nano-signal-slot](https://github.com/NoAvailableAlias/nano-signal-slot)** - Thread safe signal-slot library.
* **[concurrentqueue](https://github.com/cameron314/concurrentqueue)** - Lock free concurrent queue implementation for notifications.
* **[NanoLog](https://github.com/Iyengar111/NanoLog)** - Logging framework used for the server.  I modified the implementation for daily rolling log files.
* **[Clara](https://github.com/catchorg/Clara)** - Command line options parser.
* **[Catch2](https://github.com/catchorg/Catch2)** - Testing framework.
* **[cpr](https://github.com/libcpr/cpr)** - HTTP client library for integration test.
