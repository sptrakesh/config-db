# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class KeyValueResults(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = KeyValueResults()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsKeyValueResults(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # KeyValueResults
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # KeyValueResults
    def Value(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from spt.configdb.model.KeyValueResult import KeyValueResult
            obj = KeyValueResult()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # KeyValueResults
    def ValueLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # KeyValueResults
    def ValueIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def KeyValueResultsStart(builder):
    builder.StartObject(1)

def Start(builder):
    KeyValueResultsStart(builder)

def KeyValueResultsAddValue(builder, value):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(value), 0)

def AddValue(builder: flatbuffers.Builder, value: int):
    KeyValueResultsAddValue(builder, value)

def KeyValueResultsStartValueVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartValueVector(builder, numElems: int) -> int:
    return KeyValueResultsStartValueVector(builder, numElems)

def KeyValueResultsEnd(builder):
    return builder.EndObject()

def End(builder):
    return KeyValueResultsEnd(builder)

import spt.configdb.model.KeyValueResult
try:
    from typing import List
except:
    pass

class KeyValueResultsT(object):

    # KeyValueResultsT
    def __init__(self):
        self.value = None  # type: List[spt.configdb.model.KeyValueResult.KeyValueResultT]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        keyValueResults = KeyValueResults()
        keyValueResults.Init(buf, pos)
        return cls.InitFromObj(keyValueResults)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, keyValueResults):
        x = KeyValueResultsT()
        x._UnPack(keyValueResults)
        return x

    # KeyValueResultsT
    def _UnPack(self, keyValueResults):
        if keyValueResults is None:
            return
        if not keyValueResults.ValueIsNone():
            self.value = []
            for i in range(keyValueResults.ValueLength()):
                if keyValueResults.Value(i) is None:
                    self.value.append(None)
                else:
                    keyValueResult_ = spt.configdb.model.KeyValueResult.KeyValueResultT.InitFromObj(keyValueResults.Value(i))
                    self.value.append(keyValueResult_)

    # KeyValueResultsT
    def Pack(self, builder):
        if self.value is not None:
            valuelist = []
            for i in range(len(self.value)):
                valuelist.append(self.value[i].Pack(builder))
            KeyValueResultsStartValueVector(builder, len(self.value))
            for i in reversed(range(len(self.value))):
                builder.PrependUOffsetTRelative(valuelist[i])
            value = builder.EndVector()
        KeyValueResultsStart(builder)
        if self.value is not None:
            KeyValueResultsAddValue(builder, value)
        keyValueResults = KeyValueResultsEnd(builder)
        return keyValueResults
