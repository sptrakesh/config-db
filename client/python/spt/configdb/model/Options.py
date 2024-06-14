# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Options(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Options()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsOptions(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Options
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Only set the value if key does not exist.  Can be used as a locking mechanism.
    # Options
    def IfNotExists(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

    # Expire key after specified number of seconds.  Default value of 0 indicates no expiration.
    # Options
    def ExpirationInSeconds(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Treat the value as `cache` (temporary storage).
    # Cached keys are not added to the tree structure.
    # If set to `true`, a non-zero `expiration_in_seconds` is required.
    # Options
    def Cache(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def OptionsStart(builder):
    builder.StartObject(3)

def Start(builder):
    OptionsStart(builder)

def OptionsAddIfNotExists(builder, ifNotExists):
    builder.PrependBoolSlot(0, ifNotExists, 0)

def AddIfNotExists(builder, ifNotExists):
    OptionsAddIfNotExists(builder, ifNotExists)

def OptionsAddExpirationInSeconds(builder, expirationInSeconds):
    builder.PrependUint32Slot(1, expirationInSeconds, 0)

def AddExpirationInSeconds(builder, expirationInSeconds):
    OptionsAddExpirationInSeconds(builder, expirationInSeconds)

def OptionsAddCache(builder, cache):
    builder.PrependBoolSlot(2, cache, 0)

def AddCache(builder, cache):
    OptionsAddCache(builder, cache)

def OptionsEnd(builder):
    return builder.EndObject()

def End(builder):
    return OptionsEnd(builder)


class OptionsT(object):

    # OptionsT
    def __init__(self):
        self.ifNotExists = False  # type: bool
        self.expirationInSeconds = 0  # type: int
        self.cache = False  # type: bool

    @classmethod
    def InitFromBuf(cls, buf, pos):
        options = Options()
        options.Init(buf, pos)
        return cls.InitFromObj(options)

    @classmethod
    def InitFromPackedBuf(cls, buf, pos=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, pos)
        return cls.InitFromBuf(buf, pos+n)

    @classmethod
    def InitFromObj(cls, options):
        x = OptionsT()
        x._UnPack(options)
        return x

    # OptionsT
    def _UnPack(self, options):
        if options is None:
            return
        self.ifNotExists = options.IfNotExists()
        self.expirationInSeconds = options.ExpirationInSeconds()
        self.cache = options.Cache()

    # OptionsT
    def Pack(self, builder):
        OptionsStart(builder)
        OptionsAddIfNotExists(builder, self.ifNotExists)
        OptionsAddExpirationInSeconds(builder, self.expirationInSeconds)
        OptionsAddCache(builder, self.cache)
        options = OptionsEnd(builder)
        return options
