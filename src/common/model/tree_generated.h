// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TREE_SPT_CONFIGDB_MODEL_H_
#define FLATBUFFERS_GENERATED_TREE_SPT_CONFIGDB_MODEL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 22 &&
              FLATBUFFERS_VERSION_MINOR == 12 &&
              FLATBUFFERS_VERSION_REVISION == 6,
             "Non-compatible flatbuffers version included");

namespace spt {
namespace configdb {
namespace model {

struct Node;
struct NodeBuilder;

inline const flatbuffers::TypeTable *NodeTypeTable();

struct Node FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef NodeBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return NodeTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CHILDREN = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *children() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_CHILDREN);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return children();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_CHILDREN) &&
           verifier.VerifyVector(children()) &&
           verifier.VerifyVectorOfStrings(children()) &&
           verifier.EndTable();
  }
};

struct NodeBuilder {
  typedef Node Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_children(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> children) {
    fbb_.AddOffset(Node::VT_CHILDREN, children);
  }
  explicit NodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Node> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Node>(end);
    return o;
  }
};

inline flatbuffers::Offset<Node> CreateNode(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> children = 0) {
  NodeBuilder builder_(_fbb);
  builder_.add_children(children);
  return builder_.Finish();
}

struct Node::Traits {
  using type = Node;
  static auto constexpr Create = CreateNode;
  static constexpr auto name = "Node";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Node";
  static constexpr size_t fields_number = 1;
  static constexpr std::array<const char *, fields_number> field_names = {
    "children"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
};

inline flatbuffers::Offset<Node> CreateNodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *children = nullptr) {
  auto children__ = children ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*children) : 0;
  return spt::configdb::model::CreateNode(
      _fbb,
      children__);
}

inline const flatbuffers::TypeTable *NodeTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 1, -1 }
  };
  static const char * const names[] = {
    "children"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const spt::configdb::model::Node *GetNode(const void *buf) {
  return flatbuffers::GetRoot<spt::configdb::model::Node>(buf);
}

inline const spt::configdb::model::Node *GetSizePrefixedNode(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<spt::configdb::model::Node>(buf);
}

inline bool VerifyNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<spt::configdb::model::Node>(nullptr);
}

inline bool VerifySizePrefixedNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<spt::configdb::model::Node>(nullptr);
}

inline void FinishNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Node> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Node> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace model
}  // namespace configdb
}  // namespace spt

#endif  // FLATBUFFERS_GENERATED_TREE_SPT_CONFIGDB_MODEL_H_
