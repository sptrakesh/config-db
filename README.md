# Configuration Database

* [Keys](#keys)
* [Protocols](#protocols)
    * [TCP/IP](#tcpip)
    * [HTTP/2](#http2)
* [Messages](#messages)
    * [Request](#request)
    * [Response](#response)
    * [Ping](#ping)
* [Utilities](#utilities)
  * [Shell](#shell)
  * [CLI](#cli)
* [Docker](#docker)
* [Acknowledgements](#acknowledgements)

A simple configuration database similar to Apache Zookeeper, Etcd, etc., built
using [RocksDB](https://rocksdb.org/).

All values are stored encrypted using `aes-256-cbc` on disk.

## Keys
All keys are required to represent UNIX *path like* structures.  Keys that do
not at least start with a leading `/` character are internally replaced with
a leading `/` character.  This means that keys `key` and `/key` are treated
as identical.

The database maintains a hierarchical model of the keys.  It is possible to
list the child node names under each level in the hierarchy.

**Note:** When listing child node names, the full path of the parent *has* to
be specified.  Relative path names are not supported.


## Commands
The following *actions/commands* are supported by the service:
* **Put** - Set a *key-value* pair.  The *key* should comply with the above
  mentioned scheme.  The *value* is an arbitrary length value.  Can be large
  values such as JSON, BSON, base64 encoded binary data etc. as appropriate.
* **Get** - Get a stored *value* for the specified *key*.
* **Delete** - Delete the *key-value* pair (and path components as appropriate)
  identified by the *key*.
* **List** - List child node names under a specified *parent path*.  Leaf nodes
  will return an error.  **Note** that parent paths are recursively removed during
  a *Delete* action if no other child nodes exist.

## Protocols
Service supports both TCP/IP and HTTP/2 connections.  The TCP service uses
[flatbuffers](https://google.github.io/flatbuffers/) as the data interchange
format, while the HTTP/2 service uses JSON.

The models were generated from the schema files using the following command:

```shell
(cd <path to project>/src/lib/model;
<path to>/flatc --cpp --cpp-std c++17 --cpp-static-reflection --reflect-names request.fbs response.fbs tree.fbs)
```

### TCP/IP
The TCP/IP service uses flatbuffers for data interchange.  The wire protocol
adds an extra 4 bytes at the beginning of each message.  These 4 bytes contain
the size of the message being interchanged.  Client and server will use these
4 bytes to ensure they have read the entire message before attempting to marshall
the data.

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

See [example](test/integration/curl.sh) for sample HTTP requests and responses.

#### PUT
```shell
# PUT request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XPUT -H "content-type: text/plain" -d "value" $url
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
The [request](src/lib/model/request.fbs) message contains the `action` desired
as well as the `key` and optionally the `value` to be sent to the service.

* **action** - An `enum` used to specify the type of action to perform against
the service.  Action values are kept similar to their HTTP verb counterparts
when possible.
* **key** - The *key* to act upon against the service.
* **value** - The *value* to set for the *key*.  This is only relevant for
the `Put` action.

### Response
The [response](src/lib/model/response.fbs) message contains either the *value*
for a *key*, or a `boolean` indicating success or failure of the action.

* **Value** - The string value associated with the specified *key* for a `Get` request.
* **Children** - A list of child node names for the specified *key/path* for a `List` request.
* **Success** - A boolean value indicating success or failure when setting (`Put`)
or deleting (`Delete`) a *key*.  A `false` value usually indicates a **transaction**
*commit* or *rollback* error.
* **Error** - A boolean value indicating the specified *key* does not exist for
a `Get` request.

When reading the response, as mentioned above, the first **4** bytes represent
the length of the buffer being returned, and the rest of the bytes (use loops
and similar constructs to read until the full message has been received) will
be the buffer.  See [integration test](test/integration/tcp.cpp) for an
example.

### Ping
Short (less than 5 bytes) messages may be sent to the service as a *keep-alive*
message.  Service will echo the message back to the client.  Examples include
`ping`, `noop`, etc.


## Utilities
Utility applications to interact with the database are provided.  These are
deployed to the `/opt/spt/bin` directory.

### Shell
The `configsh` application provides a *shell* to interact with the database.
The server should be running and the TCP port open for the application to connect.

The following shows a simple CRUD type interaction via the shell.

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
  rm <key> - To remove configured key.  Eg. [rm /key1/key2/key3]
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
configdb> exit
Bye
```

### CLI
The `configctl` application provides a simple means for interacting with the
database server.  Use it to execute single actions against the service when
required.

**Note:** Unlike the **shell** application, values specified as command line
argument must be quoted if they contain spaces or other special characters.

The server should be running and the TCP port open for the application to connect.

The following shows a simple CRUD type interaction via the cli. These were using
the default values for `server [-s|--server]` and `port [-p|--port]` options.

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
spt:/home/spt $ /opt/spt/bin/configctl -a delete -k /test
Removed key /test
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /
Error retrieving path /
```

## Docker
Docker images are available on [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/config-db).

The database files are stored under `/opt/spt/data`.  If you wish to persist
the database, volume mount this location.

It is possible to run the integration tests against the docker container instead
of the service running locally.

```shell
docker run -d --rm -p 6006:6000 -p 2022:2020 --name config-db sptrakesh/config-db
```

## Acknowledgements
This software has been developed mainly using work other people/projects have contributed.
The following are the components used to build this software:
* **[RocksDB](http://rocksdb.org/)** - The database used to persist configuration.
* **[OpenSSL](https://www.openssl.org/)** - For AES encryption of the values 
  stored in the database.
* **[Boost](https://github.com/boostorg/boost)** - In particular **Asio** for the
  `TCP socket` server implementation.
* **[nghttp2](https://www.nghttp2.org/)** - for the HTTP/2 server implementation.
* **[flatbuffers](https://google.github.io/flatbuffers)** - The data interchange
  format for client-server TCP/IP interactions.
* **[NanoLog](https://github.com/Iyengar111/NanoLog)** - Logging framework used
  for the server.  I modified the implementation for daily rolling log files.
* **[Clara](https://github.com/catchorg/Clara)** - Command line options parser.
* **[Catch2](https://github.com/catchorg/Catch2)** - Testing framework.
