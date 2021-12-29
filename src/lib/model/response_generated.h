// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RESPONSE_SPT_CONFIGDB_MODEL_H_
#define FLATBUFFERS_GENERATED_RESPONSE_SPT_CONFIGDB_MODEL_H_

#include "flatbuffers/flatbuffers.h"

namespace spt {
namespace configdb {
namespace model {

struct Value;
struct ValueBuilder;

struct Children;
struct ChildrenBuilder;

struct Success;
struct SuccessBuilder;

struct Error;
struct ErrorBuilder;

struct Response;
struct ResponseBuilder;

inline const flatbuffers::TypeTable *ValueTypeTable();

inline const flatbuffers::TypeTable *ChildrenTypeTable();

inline const flatbuffers::TypeTable *SuccessTypeTable();

inline const flatbuffers::TypeTable *ErrorTypeTable();

inline const flatbuffers::TypeTable *ResponseTypeTable();

enum class ResponseValue : uint8_t {
  NONE = 0,
  Value = 1,
  Children = 2,
  Success = 3,
  Error = 4,
  MIN = NONE,
  MAX = Error
};

inline const ResponseValue (&EnumValuesResponseValue())[5] {
  static const ResponseValue values[] = {
    ResponseValue::NONE,
    ResponseValue::Value,
    ResponseValue::Children,
    ResponseValue::Success,
    ResponseValue::Error
  };
  return values;
}

inline const char * const *EnumNamesResponseValue() {
  static const char * const names[6] = {
    "NONE",
    "Value",
    "Children",
    "Success",
    "Error",
    nullptr
  };
  return names;
}

inline const char *EnumNameResponseValue(ResponseValue e) {
  if (flatbuffers::IsOutRange(e, ResponseValue::NONE, ResponseValue::Error)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesResponseValue()[index];
}

template<typename T> struct ResponseValueTraits {
  static const ResponseValue enum_value = ResponseValue::NONE;
};

template<> struct ResponseValueTraits<spt::configdb::model::Value> {
  static const ResponseValue enum_value = ResponseValue::Value;
};

template<> struct ResponseValueTraits<spt::configdb::model::Children> {
  static const ResponseValue enum_value = ResponseValue::Children;
};

template<> struct ResponseValueTraits<spt::configdb::model::Success> {
  static const ResponseValue enum_value = ResponseValue::Success;
};

template<> struct ResponseValueTraits<spt::configdb::model::Error> {
  static const ResponseValue enum_value = ResponseValue::Error;
};

bool VerifyResponseValue(flatbuffers::Verifier &verifier, const void *obj, ResponseValue type);
bool VerifyResponseValueVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct Value FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ValueBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ValueTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4
  };
  const flatbuffers::String *value() const {
    return GetPointer<const flatbuffers::String *>(VT_VALUE);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return value();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_VALUE) &&
           verifier.VerifyString(value()) &&
           verifier.EndTable();
  }
};

struct ValueBuilder {
  typedef Value Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_value(flatbuffers::Offset<flatbuffers::String> value) {
    fbb_.AddOffset(Value::VT_VALUE, value);
  }
  explicit ValueBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Value> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Value>(end);
    return o;
  }
};

inline flatbuffers::Offset<Value> CreateValue(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> value = 0) {
  ValueBuilder builder_(_fbb);
  builder_.add_value(value);
  return builder_.Finish();
}

struct Value::Traits {
  using type = Value;
  static auto constexpr Create = CreateValue;
  static constexpr auto name = "Value";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Value";
  static constexpr std::array<const char *, 1> field_names = {
    "value"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 1;
};

inline flatbuffers::Offset<Value> CreateValueDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *value = nullptr) {
  auto value__ = value ? _fbb.CreateString(value) : 0;
  return spt::configdb::model::CreateValue(
      _fbb,
      value__);
}

struct Children FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ChildrenBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ChildrenTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *value() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_VALUE);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return value();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_VALUE) &&
           verifier.VerifyVector(value()) &&
           verifier.VerifyVectorOfStrings(value()) &&
           verifier.EndTable();
  }
};

struct ChildrenBuilder {
  typedef Children Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_value(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> value) {
    fbb_.AddOffset(Children::VT_VALUE, value);
  }
  explicit ChildrenBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Children> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Children>(end);
    return o;
  }
};

inline flatbuffers::Offset<Children> CreateChildren(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> value = 0) {
  ChildrenBuilder builder_(_fbb);
  builder_.add_value(value);
  return builder_.Finish();
}

struct Children::Traits {
  using type = Children;
  static auto constexpr Create = CreateChildren;
  static constexpr auto name = "Children";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Children";
  static constexpr std::array<const char *, 1> field_names = {
    "value"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 1;
};

inline flatbuffers::Offset<Children> CreateChildrenDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *value = nullptr) {
  auto value__ = value ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*value) : 0;
  return spt::configdb::model::CreateChildren(
      _fbb,
      value__);
}

struct Success FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SuccessBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return SuccessTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4
  };
  bool value() const {
    return GetField<uint8_t>(VT_VALUE, 0) != 0;
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return value();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_VALUE) &&
           verifier.EndTable();
  }
};

struct SuccessBuilder {
  typedef Success Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_value(bool value) {
    fbb_.AddElement<uint8_t>(Success::VT_VALUE, static_cast<uint8_t>(value), 0);
  }
  explicit SuccessBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Success> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Success>(end);
    return o;
  }
};

inline flatbuffers::Offset<Success> CreateSuccess(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool value = false) {
  SuccessBuilder builder_(_fbb);
  builder_.add_value(value);
  return builder_.Finish();
}

struct Success::Traits {
  using type = Success;
  static auto constexpr Create = CreateSuccess;
  static constexpr auto name = "Success";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Success";
  static constexpr std::array<const char *, 1> field_names = {
    "value"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 1;
};

struct Error FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ErrorBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ErrorTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE = 4,
    VT_MESSAGE = 6
  };
  bool value() const {
    return GetField<uint8_t>(VT_VALUE, 0) != 0;
  }
  const flatbuffers::String *message() const {
    return GetPointer<const flatbuffers::String *>(VT_MESSAGE);
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return value();
    else if constexpr (Index == 1) return message();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_VALUE) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           verifier.VerifyString(message()) &&
           verifier.EndTable();
  }
};

struct ErrorBuilder {
  typedef Error Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_value(bool value) {
    fbb_.AddElement<uint8_t>(Error::VT_VALUE, static_cast<uint8_t>(value), 0);
  }
  void add_message(flatbuffers::Offset<flatbuffers::String> message) {
    fbb_.AddOffset(Error::VT_MESSAGE, message);
  }
  explicit ErrorBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Error> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Error>(end);
    return o;
  }
};

inline flatbuffers::Offset<Error> CreateError(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool value = false,
    flatbuffers::Offset<flatbuffers::String> message = 0) {
  ErrorBuilder builder_(_fbb);
  builder_.add_message(message);
  builder_.add_value(value);
  return builder_.Finish();
}

struct Error::Traits {
  using type = Error;
  static auto constexpr Create = CreateError;
  static constexpr auto name = "Error";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Error";
  static constexpr std::array<const char *, 2> field_names = {
    "value",
    "message"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 2;
};

inline flatbuffers::Offset<Error> CreateErrorDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool value = false,
    const char *message = nullptr) {
  auto message__ = message ? _fbb.CreateString(message) : 0;
  return spt::configdb::model::CreateError(
      _fbb,
      value,
      message__);
}

struct Response FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ResponseBuilder Builder;
  struct Traits;
  static const flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ResponseTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VALUE_TYPE = 4,
    VT_VALUE = 6
  };
  spt::configdb::model::ResponseValue value_type() const {
    return static_cast<spt::configdb::model::ResponseValue>(GetField<uint8_t>(VT_VALUE_TYPE, 0));
  }
  const void *value() const {
    return GetPointer<const void *>(VT_VALUE);
  }
  template<typename T> const T *value_as() const;
  const spt::configdb::model::Value *value_as_Value() const {
    return value_type() == spt::configdb::model::ResponseValue::Value ? static_cast<const spt::configdb::model::Value *>(value()) : nullptr;
  }
  const spt::configdb::model::Children *value_as_Children() const {
    return value_type() == spt::configdb::model::ResponseValue::Children ? static_cast<const spt::configdb::model::Children *>(value()) : nullptr;
  }
  const spt::configdb::model::Success *value_as_Success() const {
    return value_type() == spt::configdb::model::ResponseValue::Success ? static_cast<const spt::configdb::model::Success *>(value()) : nullptr;
  }
  const spt::configdb::model::Error *value_as_Error() const {
    return value_type() == spt::configdb::model::ResponseValue::Error ? static_cast<const spt::configdb::model::Error *>(value()) : nullptr;
  }
  template<size_t Index>
  auto get_field() const {
         if constexpr (Index == 0) return value_type();
    else if constexpr (Index == 1) return value();
    else static_assert(Index != Index, "Invalid Field Index");
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_VALUE_TYPE) &&
           VerifyOffset(verifier, VT_VALUE) &&
           VerifyResponseValue(verifier, value(), value_type()) &&
           verifier.EndTable();
  }
};

template<> inline const spt::configdb::model::Value *Response::value_as<spt::configdb::model::Value>() const {
  return value_as_Value();
}

template<> inline const spt::configdb::model::Children *Response::value_as<spt::configdb::model::Children>() const {
  return value_as_Children();
}

template<> inline const spt::configdb::model::Success *Response::value_as<spt::configdb::model::Success>() const {
  return value_as_Success();
}

template<> inline const spt::configdb::model::Error *Response::value_as<spt::configdb::model::Error>() const {
  return value_as_Error();
}

struct ResponseBuilder {
  typedef Response Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_value_type(spt::configdb::model::ResponseValue value_type) {
    fbb_.AddElement<uint8_t>(Response::VT_VALUE_TYPE, static_cast<uint8_t>(value_type), 0);
  }
  void add_value(flatbuffers::Offset<void> value) {
    fbb_.AddOffset(Response::VT_VALUE, value);
  }
  explicit ResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Response> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Response>(end);
    return o;
  }
};

inline flatbuffers::Offset<Response> CreateResponse(
    flatbuffers::FlatBufferBuilder &_fbb,
    spt::configdb::model::ResponseValue value_type = spt::configdb::model::ResponseValue::NONE,
    flatbuffers::Offset<void> value = 0) {
  ResponseBuilder builder_(_fbb);
  builder_.add_value(value);
  builder_.add_value_type(value_type);
  return builder_.Finish();
}

struct Response::Traits {
  using type = Response;
  static auto constexpr Create = CreateResponse;
  static constexpr auto name = "Response";
  static constexpr auto fully_qualified_name = "spt.configdb.model.Response";
  static constexpr std::array<const char *, 2> field_names = {
    "value_type",
    "value"
  };
  template<size_t Index>
  using FieldType = decltype(std::declval<type>().get_field<Index>());
  static constexpr size_t fields_number = 2;
};

inline bool VerifyResponseValue(flatbuffers::Verifier &verifier, const void *obj, ResponseValue type) {
  switch (type) {
    case ResponseValue::NONE: {
      return true;
    }
    case ResponseValue::Value: {
      auto ptr = reinterpret_cast<const spt::configdb::model::Value *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ResponseValue::Children: {
      auto ptr = reinterpret_cast<const spt::configdb::model::Children *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ResponseValue::Success: {
      auto ptr = reinterpret_cast<const spt::configdb::model::Success *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ResponseValue::Error: {
      auto ptr = reinterpret_cast<const spt::configdb::model::Error *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyResponseValueVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyResponseValue(
        verifier,  values->Get(i), types->GetEnum<ResponseValue>(i))) {
      return false;
    }
  }
  return true;
}

inline const flatbuffers::TypeTable *ResponseValueTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_SEQUENCE, 0, -1 },
    { flatbuffers::ET_SEQUENCE, 0, 0 },
    { flatbuffers::ET_SEQUENCE, 0, 1 },
    { flatbuffers::ET_SEQUENCE, 0, 2 },
    { flatbuffers::ET_SEQUENCE, 0, 3 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ValueTypeTable,
    spt::configdb::model::ChildrenTypeTable,
    spt::configdb::model::SuccessTypeTable,
    spt::configdb::model::ErrorTypeTable
  };
  static const char * const names[] = {
    "NONE",
    "Value",
    "Children",
    "Success",
    "Error"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_UNION, 5, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ValueTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const char * const names[] = {
    "value"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ChildrenTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_STRING, 1, -1 }
  };
  static const char * const names[] = {
    "value"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *SuccessTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_BOOL, 0, -1 }
  };
  static const char * const names[] = {
    "value"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 1, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ErrorTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_BOOL, 0, -1 },
    { flatbuffers::ET_STRING, 0, -1 }
  };
  static const char * const names[] = {
    "value",
    "message"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 2, type_codes, nullptr, nullptr, nullptr, names
  };
  return &tt;
}

inline const flatbuffers::TypeTable *ResponseTypeTable() {
  static const flatbuffers::TypeCode type_codes[] = {
    { flatbuffers::ET_UTYPE, 0, 0 },
    { flatbuffers::ET_SEQUENCE, 0, 0 }
  };
  static const flatbuffers::TypeFunction type_refs[] = {
    spt::configdb::model::ResponseValueTypeTable
  };
  static const char * const names[] = {
    "value_type",
    "value"
  };
  static const flatbuffers::TypeTable tt = {
    flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const spt::configdb::model::Response *GetResponse(const void *buf) {
  return flatbuffers::GetRoot<spt::configdb::model::Response>(buf);
}

inline const spt::configdb::model::Response *GetSizePrefixedResponse(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<spt::configdb::model::Response>(buf);
}

inline bool VerifyResponseBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<spt::configdb::model::Response>(nullptr);
}

inline bool VerifySizePrefixedResponseBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<spt::configdb::model::Response>(nullptr);
}

inline void FinishResponseBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Response> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedResponseBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<spt::configdb::model::Response> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace model
}  // namespace configdb
}  // namespace spt

#endif  // FLATBUFFERS_GENERATED_RESPONSE_SPT_CONFIGDB_MODEL_H_
