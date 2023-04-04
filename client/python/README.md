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
