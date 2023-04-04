# Python Client
A simple [client](client.py) for the TCP/IP service.  Uses [models](spt/configdb/model) generated from the flatbuffers
schemas.

See the integration test suite (e.g. [crud](features/steps/crud.py)) for sample use of the client.

## Resource
It is also possible to use the client as an auto-closing resource.

```python
async with Client(host="localhost", port=2020) as client:
    _key = "/key1/key2/key3"
    res = await client.set(_key, "value")
    res = await client.get(_key)
    log.info(f"Read stored value: {res}")
```

## SSL
The Python bundled with Xcode uses an older version of SSL (LibreSSL 2.8.3) which does not support TLS 1.3.
The TCP service only supports TLS 1.3.  To use SSL on Mac OS X, use [homebrew](https://brew.sh/) to install a current
version (e.g. 3.11) which supports TLS 1.3 (OpenSSL 1.1.1t  7 Feb 2023).

```python
async with Client(host="localhost", port=2020,
                  ssl_verify_file="/usr/local/spt/certs/ca.crt",
                  ssl_certificate_file="/usr/local/spt/certs/client.crt",
                  ssl_key_file="/usr/local/spt/certs/client.key") as client:
    _key = "/key1/key2/key3"
    res = await client.set(_key, "value")
    res = await client.get(_key)
    log.info(f"Read stored value: {res}")
```