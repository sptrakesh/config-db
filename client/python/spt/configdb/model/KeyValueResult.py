# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class KeyValueResult(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = KeyValueResult()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsKeyValueResult(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # KeyValueResult
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # KeyValueResult
    def Key(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # KeyValueResult
    def ValueType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # KeyValueResult
    def Value(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

def KeyValueResultStart(builder):
    builder.StartObject(3)

def Start(builder):
    KeyValueResultStart(builder)

def KeyValueResultAddKey(builder, key):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(key), 0)

def AddKey(builder, key):
    KeyValueResultAddKey(builder, key)

def KeyValueResultAddValueType(builder, valueType):
    builder.PrependUint8Slot(1, valueType, 0)

def AddValueType(builder, valueType):
    KeyValueResultAddValueType(builder, valueType)

def KeyValueResultAddValue(builder, value):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(value), 0)

def AddValue(builder, value):
    KeyValueResultAddValue(builder, value)

def KeyValueResultEnd(builder):
    return builder.EndObject()

def End(builder):
    return KeyValueResultEnd(builder)

import spt.configdb.model.Children
import spt.configdb.model.Success
import spt.configdb.model.Value
import spt.configdb.model.ValueVariant
try:
    from typing import Union
except:
    pass

class KeyValueResultT(object):

    # KeyValueResultT
    def __init__(self):
        self.key = None  # type: str
        self.valueType = 0  # type: int
        self.value = None  # type: Union[None, spt.configdb.model.Value.ValueT, spt.configdb.model.Children.ChildrenT, spt.configdb.model.Success.SuccessT]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        keyValueResult = KeyValueResult()
        keyValueResult.Init(buf, pos)
        return cls.InitFromObj(keyValueResult)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, keyValueResult):
        x = KeyValueResultT()
        x._UnPack(keyValueResult)
        return x

    # KeyValueResultT
    def _UnPack(self, keyValueResult):
        if keyValueResult is None:
            return
        self.key = keyValueResult.Key()
        self.valueType = keyValueResult.ValueType()
        self.value = spt.configdb.model.ValueVariant.ValueVariantCreator(self.valueType, keyValueResult.Value())

    # KeyValueResultT
    def Pack(self, builder):
        if self.key is not None:
            key = builder.CreateString(self.key)
        if self.value is not None:
            value = self.value.Pack(builder)
        KeyValueResultStart(builder)
        if self.key is not None:
            KeyValueResultAddKey(builder, key)
        KeyValueResultAddValueType(builder, self.valueType)
        if self.value is not None:
            KeyValueResultAddValue(builder, value)
        keyValueResult = KeyValueResultEnd(builder)
        return keyValueResult
