// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_
#define FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_

#include "flatbuffers/flatbuffers.h"

namespace spt {
namespace configdb {
namespace model {

struct Request;
struct RequestBuilder;

inline const flatbuffers::TypeTable *RequestTypeTable();

enum class Action : int8_t {
  Get = 0,
  Put = 1,
  Delete = 2,
  List = 3,
  MIN = Get,
  MAX = List
};

inline const Action (&EnumValuesAction())[4] {
  static const Action values[] = {
    Action::Get,
    Action::Put,
    Action::Delete,
    Action::List
  };
  return values;
}

inline const char * const *EnumNamesAction() {
  static const char * const names[5] = {
    "Get",
    "Put",
    "Delete",
    "List",
    nullptr
  };
  return names;
}

inline const char *EnumNameAction(Action e) {
  if (flatbuffers::IsOutRange(e, Action::Get, Action::List)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesAction()[index];
}

struct Request FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RequestBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return RequestTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACTION = 4,
    VT_KEY = 6,
    VT_VALUE = 8
  };
  spt::configdb::model::Action action() const {
    return static_cast<spt::configdb::model::Action>(GetField<int8_t>(VT_ACTION, 0));
  }
  const flatbuffers::String *key() const {
    return GetPointer<const flatbuffers::String *>(VT_KEY);
  }
  const flatbuffers::String *value() const {
    return GetPointer<const flatbuffers::String *>(VT_VALUE);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return action();
    else if constexpr (Index == 1) return key();
    else if constexpr (Index == 2) return value();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_ACTION) &&
           VerifyOffset(verifier, VT_KEY) &&
           verifier.VerifyString(key()) &&
           VerifyOffset(verifier, VT_VALUE) &&
           verifier.VerifyString(value()) &&
           verifier.EndTable();
  }
};

struct RequestBuilder {
  typedef Request Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_action(spt::configdb::model::Action action) {
    fbb_.AddElement<int8_t>(Request::VT_ACTION, static_cast<int8_t>(action), 0);
  }
  void add_key(flatbuffers::Offset<flatbuffers::String> key) {
    fbb_.AddOffset(Request::VT_KEY, key);
  }
  void add_value(flatbuffers::Offset<flatbuffers::String> value) {
    fbb_.AddOffset(Request::VT_VALUE, value);
  }
  explicit RequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline flatbuffers::Offset<Request> CreateRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    spt::configdb::model::Action action = spt::configdb::model::Action::Get,
    flatbuffers::Offset<flatbuffers::String> key = 0,
    flatbuffers::Offset<flatbuffers::String> value = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_value(value);
  builder_.add_key(key);
  builder_.add_action(action);
  return builder_.Finish();
}

struct Request::Traits {
  using type = Request;
  static auto constexpr Create = CreateRequest;
  static constexpr auto name = "Request";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Request";
  static constexpr std::array<const char *, 3> field_names = {
    "action",
    "key",
    "value"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 3;
};

inline flatbuffers::Offset<Request> CreateRequestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    spt::configdb::model::Action action = spt::configdb::model::Action::Get,
    const char *key = nullptr,
    const char *value = nullptr) {
  auto key__ = key ? _fbb.CreateString(key) : 0;
  auto value__ = value ? _fbb.CreateString(value) : 0;
  return spt::configdb::model::CreateRequest(
      _fbb,
      action,
      key__,
      value__);
}

inline const flatbuffers::TypeTable *ActionTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_CHAR, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ActionTypeTable
  };
  static const char * const names[] = {
    "Get",
    "Put",
    "Delete",
    "List"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_ENUM, 4, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *RequestTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_CHAR, 0, 0 },
    { flatbuffers::ET_STRING, 0, -1 },
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ActionTypeTable
  };
  static const char * const names[] = {
    "action",
    "key",
    "value"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 3, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const spt::configdb::model::Request *GetRequest(const void *buf) {
  return flatbuffers::GetRoot<spt::configdb::model::Request>(buf);
}

inline const spt::configdb::model::Request *GetSizePrefixedRequest(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<spt::configdb::model::Request>(buf);
}

inline bool VerifyRequestBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<spt::configdb::model::Request>(nullptr);
}

inline bool VerifySizePrefixedRequestBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<spt::configdb::model::Request>(nullptr);
}

inline void FinishRequestBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Request> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRequestBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Request> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace model
}  // namespace configdb
}  // namespace spt

#endif  // FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_