// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_
#define FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace spt {
namespace configdb {
namespace model {

struct Options;
struct OptionsBuilder;

struct KeyValue;
struct KeyValueBuilder;

struct Request;
struct RequestBuilder;

inline const ::flatbuffers::TypeTable *OptionsTypeTable();

inline const ::flatbuffers::TypeTable *KeyValueTypeTable();

inline const ::flatbuffers::TypeTable *RequestTypeTable();

enum class Action : int8_t {
  Get = 0,
  Put = 1,
  Delete = 2,
  List = 3,
  Move = 4,
  TTL = 5,
  MIN = Get,
  MAX = TTL
};

inline const Action (&EnumValuesAction())[6] {
  static const Action values[] = {
    Action::Get,
    Action::Put,
    Action::Delete,
    Action::List,
    Action::Move,
    Action::TTL
  };
  return values;
}

inline const char * const *EnumNamesAction() {
  static const char * const names[7] = {
    "Get",
    "Put",
    "Delete",
    "List",
    "Move",
    "TTL",
    nullptr
  };
  return names;
}

inline const char *EnumNameAction(Action e) {
  if (::flatbuffers::IsOutRange(e, Action::Get, Action::TTL)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesAction()[index];
}

struct Options FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef OptionsBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return OptionsTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_IF_NOT_EXISTS = 4,
    VT_EXPIRATION_IN_SECONDS = 6
  };
  /// Only set the value if key does not exist.  Can be used as a locking mechanism.
  bool if_not_exists() const {
    return GetField<uint8_t>(VT_IF_NOT_EXISTS, 0) != 0;
  }
  /// Expire key after specified number of seconds.  Default value of 0 indicates no expiration.
  uint32_t expiration_in_seconds() const {
    return GetField<uint32_t>(VT_EXPIRATION_IN_SECONDS, 0);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return if_not_exists();
    else if constexpr (Index == 1) return expiration_in_seconds();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_IF_NOT_EXISTS, 1) &&
           VerifyField<uint32_t>(verifier, VT_EXPIRATION_IN_SECONDS, 4) &&
           verifier.EndTable();
  }
};

struct OptionsBuilder {
  typedef Options Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_if_not_exists(bool if_not_exists) {
    fbb_.AddElement<uint8_t>(Options::VT_IF_NOT_EXISTS, static_cast<uint8_t>(if_not_exists), 0);
  }
  void add_expiration_in_seconds(uint32_t expiration_in_seconds) {
    fbb_.AddElement<uint32_t>(Options::VT_EXPIRATION_IN_SECONDS, expiration_in_seconds, 0);
  }
  explicit OptionsBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Options> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Options>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Options> CreateOptions(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool if_not_exists = false,
    uint32_t expiration_in_seconds = 0) {
  OptionsBuilder builder_(_fbb);
  builder_.add_expiration_in_seconds(expiration_in_seconds);
  builder_.add_if_not_exists(if_not_exists);
  return builder_.Finish();
}

struct Options::Traits {
  using type = Options;
  static auto constexpr Create = CreateOptions;
  static constexpr auto name = "Options";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Options";
  static constexpr size_t fields_number = 2;
  static constexpr std::array<const char *, fields_number> field_names = {
    "if_not_exists",
    "expiration_in_seconds"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
};

struct KeyValue FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef KeyValueBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return KeyValueTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_VALUE = 6,
    VT_OPTIONS = 8
  };
  const ::flatbuffers::String *key() const {
    return GetPointer<const ::flatbuffers::String *>(VT_KEY);
  }
  const ::flatbuffers::String *value() const {
    return GetPointer<const ::flatbuffers::String *>(VT_VALUE);
  }
  const spt::configdb::model::Options *options() const {
    return GetPointer<const spt::configdb::model::Options *>(VT_OPTIONS);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return key();
    else if constexpr (Index == 1) return value();
    else if constexpr (Index == 2) return options();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_KEY) &&
           verifier.VerifyString(key()) &&
           VerifyOffset(verifier, VT_VALUE) &&
           verifier.VerifyString(value()) &&
           VerifyOffset(verifier, VT_OPTIONS) &&
           verifier.VerifyTable(options()) &&
           verifier.EndTable();
  }
};

struct KeyValueBuilder {
  typedef KeyValue Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_key(::flatbuffers::Offset<::flatbuffers::String> key) {
    fbb_.AddOffset(KeyValue::VT_KEY, key);
  }
  void add_value(::flatbuffers::Offset<::flatbuffers::String> value) {
    fbb_.AddOffset(KeyValue::VT_VALUE, value);
  }
  void add_options(::flatbuffers::Offset<spt::configdb::model::Options> options) {
    fbb_.AddOffset(KeyValue::VT_OPTIONS, options);
  }
  explicit KeyValueBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<KeyValue> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<KeyValue>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<KeyValue> CreateKeyValue(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> key = 0,
    ::flatbuffers::Offset<::flatbuffers::String> value = 0,
    ::flatbuffers::Offset<spt::configdb::model::Options> options = 0) {
  KeyValueBuilder builder_(_fbb);
  builder_.add_options(options);
  builder_.add_value(value);
  builder_.add_key(key);
  return builder_.Finish();
}

struct KeyValue::Traits {
  using type = KeyValue;
  static auto constexpr Create = CreateKeyValue;
  static constexpr auto name = "KeyValue";
  static constexpr auto fully_qualified_name = "spt.configdb.model.KeyValue";
  static constexpr size_t fields_number = 3;
  static constexpr std::array<const char *, fields_number> field_names = {
    "key",
    "value",
    "options"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
};

inline ::flatbuffers::Offset<KeyValue> CreateKeyValueDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *key = nullptr,
    const char *value = nullptr,
    ::flatbuffers::Offset<spt::configdb::model::Options> options = 0) {
  auto key__ = key ? _fbb.CreateString(key) : 0;
  auto value__ = value ? _fbb.CreateString(value) : 0;
  return spt::configdb::model::CreateKeyValue(
      _fbb,
      key__,
      value__,
      options);
}

struct Request FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RequestBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return RequestTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ACTION = 4,
    VT_DATA = 6
  };
  spt::configdb::model::Action action() const {
    return static_cast<spt::configdb::model::Action>(GetField<int8_t>(VT_ACTION, 0));
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<spt::configdb::model::KeyValue>> *data() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<spt::configdb::model::KeyValue>> *>(VT_DATA);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return action();
    else if constexpr (Index == 1) return data();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_ACTION, 1) &&
           VerifyOffset(verifier, VT_DATA) &&
           verifier.VerifyVector(data()) &&
           verifier.VerifyVectorOfTables(data()) &&
           verifier.EndTable();
  }
};

struct RequestBuilder {
  typedef Request Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_action(spt::configdb::model::Action action) {
    fbb_.AddElement<int8_t>(Request::VT_ACTION, static_cast<int8_t>(action), 0);
  }
  void add_data(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<spt::configdb::model::KeyValue>>> data) {
    fbb_.AddOffset(Request::VT_DATA, data);
  }
  explicit RequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Request> CreateRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    spt::configdb::model::Action action = spt::configdb::model::Action::Get,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<spt::configdb::model::KeyValue>>> data = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_data(data);
  builder_.add_action(action);
  return builder_.Finish();
}

struct Request::Traits {
  using type = Request;
  static auto constexpr Create = CreateRequest;
  static constexpr auto name = "Request";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Request";
  static constexpr size_t fields_number = 2;
  static constexpr std::array<const char *, fields_number> field_names = {
    "action",
    "data"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
};

inline ::flatbuffers::Offset<Request> CreateRequestDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    spt::configdb::model::Action action = spt::configdb::model::Action::Get,
    const std::vector<::flatbuffers::Offset<spt::configdb::model::KeyValue>> *data = nullptr) {
  auto data__ = data ? _fbb.CreateVector<::flatbuffers::Offset<spt::configdb::model::KeyValue>>(*data) : 0;
  return spt::configdb::model::CreateRequest(
      _fbb,
      action,
      data__);
}

inline const ::flatbuffers::TypeTable *ActionTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_CHAR, 0, 0 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ActionTypeTable
  };
  static const char * const names[] = {
    "Get",
    "Put",
    "Delete",
    "List",
    "Move",
    "TTL"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_ENUM, 6, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const ::flatbuffers::TypeTable *OptionsTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_BOOL, 0, -1 },
    { ::flatbuffers::ET_UINT, 0, -1 }
  };
  static const char * const names[] = {
    "if_not_exists",
    "expiration_in_seconds"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 2, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const ::flatbuffers::TypeTable *KeyValueTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_STRING, 0, -1 },
    { ::flatbuffers::ET_STRING, 0, -1 },
    { ::flatbuffers::ET_SEQUENCE, 0, 0 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::OptionsTypeTable
  };
  static const char * const names[] = {
    "key",
    "value",
    "options"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 3, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const ::flatbuffers::TypeTable *RequestTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_CHAR, 0, 0 },
    { ::flatbuffers::ET_SEQUENCE, 1, 1 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ActionTypeTable,
    spt::configdb::model::KeyValueTypeTable
  };
  static const char * const names[] = {
    "action",
    "data"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const spt::configdb::model::Request *GetRequest(const void *buf) {
  return ::flatbuffers::GetRoot<spt::configdb::model::Request>(buf);
}

inline const spt::configdb::model::Request *GetSizePrefixedRequest(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<spt::configdb::model::Request>(buf);
}

inline bool VerifyRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<spt::configdb::model::Request>(nullptr);
}

inline bool VerifySizePrefixedRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<spt::configdb::model::Request>(nullptr);
}

inline void FinishRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<spt::configdb::model::Request> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<spt::configdb::model::Request> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace model
}  // namespace configdb
}  // namespace spt

#endif  // FLATBUFFERS_GENERATED_REQUEST_SPT_CONFIGDB_MODEL_H_
