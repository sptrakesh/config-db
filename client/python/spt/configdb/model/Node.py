# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Node(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Node()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsNode(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Node
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Node
    def Children(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # Node
    def ChildrenLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Node
    def ChildrenIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def NodeStart(builder): builder.StartObject(1)
def Start(builder):
    return NodeStart(builder)
def NodeAddChildren(builder, children): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(children), 0)
def AddChildren(builder, children):
    return NodeAddChildren(builder, children)
def NodeStartChildrenVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartChildrenVector(builder, numElems):
    return NodeStartChildrenVector(builder, numElems)
def NodeEnd(builder): return builder.EndObject()
def End(builder):
    return NodeEnd(builder)
try:
    from typing import List
except:
    pass

class NodeT(object):

    # NodeT
    def __init__(self):
        self.children = None  # type: List[str]

    @classmethod
    def InitFromBuf(cls, buf, pos):
        node = Node()
        node.Init(buf, pos)
        return cls.InitFromObj(node)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, node):
        x = NodeT()
        x._UnPack(node)
        return x

    # NodeT
    def _UnPack(self, node):
        if node is None:
            return
        if not node.ChildrenIsNone():
            self.children = []
            for i in range(node.ChildrenLength()):
                self.children.append(node.Children(i))

    # NodeT
    def Pack(self, builder):
        if self.children is not None:
            childrenlist = []
            for i in range(len(self.children)):
                childrenlist.append(builder.CreateString(self.children[i]))
            NodeStartChildrenVector(builder, len(self.children))
            for i in reversed(range(len(self.children))):
                builder.PrependUOffsetTRelative(childrenlist[i])
            children = builder.EndVector()
        NodeStart(builder)
        if self.children is not None:
            NodeAddChildren(builder, children)
        node = NodeEnd(builder)
        return node
