# automatically generated by the FlatBuffers compiler, do not modify

# namespace: model

class ResultVariant(object):
    NONE = 0
    KeyValueResults = 1
    Success = 2

def ResultVariantCreator(unionType, table):
    from flatbuffers.table import Table
    if not isinstance(table, Table):
        return None
    if unionType == ResultVariant().KeyValueResults:
        import spt.configdb.model.KeyValueResults
        return spt.configdb.model.KeyValueResults.KeyValueResultsT.InitFromBuf(table.Bytes, table.Pos)
    if unionType == ResultVariant().Success:
        import spt.configdb.model.Success
        return spt.configdb.model.Success.SuccessT.InitFromBuf(table.Bytes, table.Pos)
    return None
