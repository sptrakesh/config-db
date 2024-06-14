# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Children(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Children()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsChildren(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Children
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Children
    def Value(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # Children
    def ValueLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Children
    def ValueIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def ChildrenStart(builder):
    builder.StartObject(1)

def Start(builder):
    ChildrenStart(builder)

def ChildrenAddValue(builder, value):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(value), 0)

def AddValue(builder, value):
    ChildrenAddValue(builder, value)

def ChildrenStartValueVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartValueVector(builder, numElems):
    return ChildrenStartValueVector(builder, numElems)

def ChildrenEnd(builder):
    return builder.EndObject()

def End(builder):
    return ChildrenEnd(builder)

try:
    from typing import List
except:
    pass

class ChildrenT(object):

    # ChildrenT
    def __init__(self):
        self.value = None  # type: List[str]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        children = Children()
        children.Init(buf, pos)
        return cls.InitFromObj(children)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, children):
        x = ChildrenT()
        x._UnPack(children)
        return x

    # ChildrenT
    def _UnPack(self, children):
        if children is None:
            return
        if not children.ValueIsNone():
            self.value = []
            for i in range(children.ValueLength()):
                self.value.append(children.Value(i))

    # ChildrenT
    def Pack(self, builder):
        if self.value is not None:
            valuelist = []
            for i in range(len(self.value)):
                valuelist.append(builder.CreateString(self.value[i]))
            ChildrenStartValueVector(builder, len(self.value))
            for i in reversed(range(len(self.value))):
                builder.PrependUOffsetTRelative(valuelist[i])
            value = builder.EndVector()
        ChildrenStart(builder)
        if self.value is not None:
            ChildrenAddValue(builder, value)
        children = ChildrenEnd(builder)
        return children
