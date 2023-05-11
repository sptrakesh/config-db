from asyncio import open_connection
from ssl import SSLContext, PROTOCOL_TLS_CLIENT, TLSVersion
from sys import byteorder
from typing import Optional, Tuple, List

from flatbuffers import Builder as _Builder

from spt.configdb.model import Request as _Request, KeyValue as _KeyValue, Action as _Action, \
    Options as _Options, Response as _Response, ResultVariant as _ResultVariant, \
    KeyValueResult as _KeyValueResult, KeyValueResults as _KeyValueResults, ValueVariant as _ValueVariant
from logger import log as _log


class Client:
    def __init__(self, host: str, port: int, ssl_verify_file: Optional[str] = None,
                 ssl_certificate_file: Optional[str] = None, ssl_key_file: Optional[str] = None):
        assert host
        self._host = host
        assert port > 0
        self._port = port
        self._verify_file = ssl_verify_file
        self._certificate_file = ssl_certificate_file
        self._key_file = ssl_key_file

    async def _async_init(self):
        if self._verify_file:
            context = SSLContext(PROTOCOL_TLS_CLIENT)
            context.load_verify_locations(self._verify_file)
            context.load_cert_chain(self._certificate_file, self._key_file)
            context.check_hostname = False
            context.minimum_version = TLSVersion.TLSv1_3
        else:
            context = None

        self._reader, self._writer = await open_connection(host=self._host, port=self._port, ssl=context)
        _log.info(f"Connected to {self._host}:{self._port}")
        return self

    def __await__(self):
        return self._async_init().__await__()

    async def __aenter__(self):
        await self._async_init()
        return self

    async def __aexit__(self, exc_type, exc_value, traceback):
        await self.close()

    async def get(self, key: str) -> Optional[str]:
        builder = _Builder(256)

        kv = _KeyValue.KeyValueT()
        kv.key = key

        req = _Request.RequestT()
        req.action = _Action.Action.Get
        req.data = [kv]
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        if resp.valueType != _ResultVariant.ResultVariant.KeyValueResults:
            _log.warning(f"Error retrieving key: {key}")
            return None

        kvrs: _KeyValueResults.KeyValueResultsT = resp.value
        if not kvrs.value:
            _log.warning(f"Error retrieving key: {key}")
            return None

        kvr: _KeyValueResult.KeyValueResultT = kvrs.value[0]
        if kvr.valueType != _ValueVariant.ValueVariant.Value:
            _log.warning(f"Error retrieving key: {key}")
            return None

        return kvr.value.value.decode("utf-8")

    async def set(self, key: str, value: str, if_not_exists: bool = False, expiration_in_seconds: int = 0) -> bool:
        kv = _KeyValue.KeyValueT()
        kv.key = key
        kv.value = value
        kv.options = _Options.OptionsT()
        kv.options.ifNotExists = if_not_exists
        kv.options.expirationInSeconds = expiration_in_seconds

        req = _Request.RequestT()
        req.action = _Action.Action.Put
        req.data = [kv]

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error setting key: {key}")

    async def remove(self, key: str) -> bool:
        builder = _Builder(256)

        kv = _KeyValue.KeyValueT()
        kv.key = key
        req = _Request.RequestT()
        req.action = _Action.Action.Delete
        req.data = [kv]
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error deleting key: {key}")

    async def move(self, key: str, dest: str, if_not_exists: bool = False) -> bool:
        builder = _Builder(256)

        _Options.Start(builder)
        _Options.AddIfNotExists(builder, if_not_exists)
        opts = _Options.End(builder)

        ks = builder.CreateString(key)
        vs = builder.CreateString(dest)
        _KeyValue.Start(builder)
        _KeyValue.AddKey(builder, ks)
        _KeyValue.AddValue(builder, vs)
        _KeyValue.AddOptions(builder, opts)
        kv = _KeyValue.End(builder)

        _Request.StartDataVector(builder, 1)
        builder.PrependUOffsetTRelative(kv)
        vec = builder.EndVector()

        _Request.Start(builder)
        _Request.AddAction(builder, _Action.Action.Move)
        _Request.AddData(builder, vec)
        req = _Request.End(builder)

        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error moving key: {key} to {dest}")

    async def list(self, path: str) -> [str]:
        builder = _Builder(256)

        kv = _KeyValue.KeyValueT()
        kv.key = path
        req = _Request.RequestT()
        req.action = _Action.Action.List
        req.data = [kv]
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        if resp.valueType != _ResultVariant.ResultVariant.KeyValueResults:
            _log.warning(f"Error retrieving path: {path}")
            return None

        kvrs: _KeyValueResults.KeyValueResultsT = resp.value
        if not kvrs or len(kvrs.value) == 0:
            _log.warning(f"Error retrieving path: {path}")
            return None

        paths = []
        for i in range(len(kvrs.value)):
            kvr: _KeyValueResult.KeyValueResultT = kvrs.value[i]
            if kvr.valueType != _ValueVariant.ValueVariant.Children:
                _log.warning(f"Error retrieving item at {i} for : {path}")
            else:
                for v in kvr.value.value:
                    paths.append(v.decode("utf-8"))

        return paths

    async def ttl(self, key: str) -> Optional[int]:
        builder = _Builder(256)

        kv = _KeyValue.KeyValueT()
        kv.key = key
        req = _Request.RequestT()
        req.action = _Action.Action.TTL
        req.data = [kv]
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        if resp.valueType != _ResultVariant.ResultVariant.KeyValueResults:
            _log.warning(f"Error retrieving TTL for key: {key}")
            return None

        kvrs: _KeyValueResults.KeyValueResultsT = resp.value
        if not kvrs or len(kvrs.value) == 0:
            _log.warning(f"Error retrieving key: {key}")
            return None

        kvr: _KeyValueResult.KeyValueResultT = kvrs.value[0]
        if kvr.valueType != _ValueVariant.ValueVariant.Value:
            _log.warning(f"Error retrieving TTL for key: {key}")
            return None

        return int(kvr.value.value)

    async def get_keys(self, keys: List[str]) -> List[Tuple[str, Optional[str]]]:
        req = _Request.RequestT()
        req.action = _Action.Action.Get
        req.data = []
        for key in keys:
            k = _KeyValue.KeyValueT()
            k.key = key
            req.data.append(k)

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        if resp.valueType != _ResultVariant.ResultVariant.KeyValueResults:
            _log.warning(f"Error retrieving {len(keys)} keys")
            return []

        kvrs: _KeyValueResults.KeyValueResultsT = resp.value
        if not kvrs.value:
            _log.warning(f"Error retrieving {len(keys)} keys")
            return []

        results = []
        for kv in kvrs.value:
            if kv.valueType == _ValueVariant.ValueVariant.Value:
                results.append((kv.key.decode("utf-8"), kv.value.value.decode("utf-8")))
            else:
                results.append((kv.key.decode("utf-8"), None))

        return results

    async def set_key_values(self, key_values: List[Tuple[str, str]]) -> bool:
        req = _Request.RequestT()
        req.action = _Action.Action.Put
        req.data = []
        for tup in key_values:
            kv = _KeyValue.KeyValueT()
            kv.key = tup[0]
            kv.value = tup[1]
            req.data.append(kv)

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error setting {len(key_values)} key-values")

    async def list_paths(self, paths: List[str]) -> List[Tuple[str, List[str]]]:
        req = _Request.RequestT()
        req.action = _Action.Action.List
        req.data = []
        for path in paths:
            kv = _KeyValue.KeyValueT()
            kv.key = path
            req.data.append(kv)

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        if resp.valueType != _ResultVariant.ResultVariant.KeyValueResults:
            _log.warning(f"Error retrieving {len(paths)} paths")
            return []

        kvrs: _KeyValueResults.KeyValueResultsT = resp.value
        if not kvrs or len(kvrs.value) == 0:
            _log.warning(f"Error retrieving {len(paths)} paths")
            return []

        results = []
        for kvr in kvrs.value:
            if kvr.valueType != _ValueVariant.ValueVariant.Children:
                _log.warning(f"Error retrieving path {kvr.key.decode('utf-8')}")
                results.append((kvr.key.decode("utf-8"), []))
            else:
                paths = []
                for v in kvr.value.value:
                    paths.append(v.decode("utf-8"))
                results.append((kvr.key.decode("utf-8"), paths))

        return results

    async def remove_keys(self, keys: List[str]) -> bool:
        req = _Request.RequestT()
        req.action = _Action.Action.Delete
        req.data = []

        for key in keys:
            kv = _KeyValue.KeyValueT()
            kv.key = key
            req.data.append(kv)

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error deleting {len(keys)} keys")

    async def move_keys(self, pairs: List[Tuple[str, str]]) -> bool:
        req = _Request.RequestT()
        req.action = _Action.Action.Move
        req.data = []

        for pair in pairs:
            kv = _KeyValue.KeyValueT()
            kv.key = pair[0]
            kv.value = pair[1]
            req.data.append(kv)

        builder = _Builder()
        req = req.Pack(builder)
        builder.Finish(req)
        buf = builder.Output()

        resp = await self._execute(buf)
        return self._read_success(resp=resp, msg=f"Error moving {len(pairs)} keys")

    async def close(self):
        self._writer.close()
        await self._writer.wait_closed()
        _log.info(f"Disconnected from {self._host}:{self._port}")

    async def _execute(self, buf: bytes) -> _Response.ResponseT:
        lv = len(buf).to_bytes(length=4, byteorder=byteorder)
        ba = b''.join([lv, buf])
        _log.info(f"Writing {len(ba)} bytes to server.")

        self._writer.write(ba)
        await self._writer.drain()

        _log.info("Reading response size to 4 byte array")
        lv = await self._reader.readexactly(4)
        l = int.from_bytes(lv, byteorder)
        _log.info(f"Response size: {l}")

        b = await self._reader.readexactly(l)
        return _Response.ResponseT.InitFromPackedBuf(b)

    def _read_success(self, resp: _Response.ResponseT, msg: str) -> bool:
        if resp.valueType != _ResultVariant.ResultVariant.Success:
            _log.warning(msg)
            return False

        return resp.value.value
