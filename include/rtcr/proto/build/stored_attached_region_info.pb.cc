// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: stored_attached_region_info.proto

#include "stored_attached_region_info.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)
namespace protobuf {
class Stored_attached_region_infoDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<Stored_attached_region_info>
      _instance;
} _Stored_attached_region_info_default_instance_;
}  // namespace protobuf
namespace protobuf_stored_5fattached_5fregion_5finfo_2eproto {
void InitDefaultsStored_attached_region_infoImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  {
    void* ptr = &::protobuf::_Stored_attached_region_info_default_instance_;
    new (ptr) ::protobuf::Stored_attached_region_info();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::protobuf::Stored_attached_region_info::InitAsDefaultInstance();
}

void InitDefaultsStored_attached_region_info() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsStored_attached_region_infoImpl);
}

::google::protobuf::Metadata file_level_metadata[1];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, attached_ds_badge_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, size_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, offset_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, rel_addr_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf::Stored_attached_region_info, executable_),
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::protobuf::Stored_attached_region_info)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::protobuf::_Stored_attached_region_info_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "stored_attached_region_info.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n!stored_attached_region_info.proto\022\010pro"
      "tobuf\"|\n\033Stored_attached_region_info\022\031\n\021"
      "attached_ds_badge\030\001 \001(\r\022\014\n\004size\030\002 \001(\r\022\016\n"
      "\006offset\030\003 \001(\r\022\020\n\010rel_addr\030\004 \001(\r\022\022\n\nexecu"
      "table\030\005 \001(\010b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 179);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "stored_attached_region_info.proto", &protobuf_RegisterTypes);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_stored_5fattached_5fregion_5finfo_2eproto
namespace protobuf {

// ===================================================================

void Stored_attached_region_info::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int Stored_attached_region_info::kAttachedDsBadgeFieldNumber;
const int Stored_attached_region_info::kSizeFieldNumber;
const int Stored_attached_region_info::kOffsetFieldNumber;
const int Stored_attached_region_info::kRelAddrFieldNumber;
const int Stored_attached_region_info::kExecutableFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

Stored_attached_region_info::Stored_attached_region_info()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_stored_5fattached_5fregion_5finfo_2eproto::InitDefaultsStored_attached_region_info();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:protobuf.Stored_attached_region_info)
}
Stored_attached_region_info::Stored_attached_region_info(const Stored_attached_region_info& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&attached_ds_badge_, &from.attached_ds_badge_,
    static_cast<size_t>(reinterpret_cast<char*>(&executable_) -
    reinterpret_cast<char*>(&attached_ds_badge_)) + sizeof(executable_));
  // @@protoc_insertion_point(copy_constructor:protobuf.Stored_attached_region_info)
}

void Stored_attached_region_info::SharedCtor() {
  ::memset(&attached_ds_badge_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&executable_) -
      reinterpret_cast<char*>(&attached_ds_badge_)) + sizeof(executable_));
  _cached_size_ = 0;
}

Stored_attached_region_info::~Stored_attached_region_info() {
  // @@protoc_insertion_point(destructor:protobuf.Stored_attached_region_info)
  SharedDtor();
}

void Stored_attached_region_info::SharedDtor() {
}

void Stored_attached_region_info::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Stored_attached_region_info::descriptor() {
  ::protobuf_stored_5fattached_5fregion_5finfo_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_stored_5fattached_5fregion_5finfo_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const Stored_attached_region_info& Stored_attached_region_info::default_instance() {
  ::protobuf_stored_5fattached_5fregion_5finfo_2eproto::InitDefaultsStored_attached_region_info();
  return *internal_default_instance();
}

Stored_attached_region_info* Stored_attached_region_info::New(::google::protobuf::Arena* arena) const {
  Stored_attached_region_info* n = new Stored_attached_region_info;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void Stored_attached_region_info::Clear() {
// @@protoc_insertion_point(message_clear_start:protobuf.Stored_attached_region_info)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&attached_ds_badge_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&executable_) -
      reinterpret_cast<char*>(&attached_ds_badge_)) + sizeof(executable_));
  _internal_metadata_.Clear();
}

bool Stored_attached_region_info::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:protobuf.Stored_attached_region_info)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // uint32 attached_ds_badge = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u /* 8 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &attached_ds_badge_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 size = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u /* 16 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &size_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 offset = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(24u /* 24 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &offset_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // uint32 rel_addr = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(32u /* 32 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &rel_addr_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // bool executable = 5;
      case 5: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(40u /* 40 & 0xFF */)) {

          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   bool, ::google::protobuf::internal::WireFormatLite::TYPE_BOOL>(
                 input, &executable_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:protobuf.Stored_attached_region_info)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:protobuf.Stored_attached_region_info)
  return false;
#undef DO_
}

void Stored_attached_region_info::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:protobuf.Stored_attached_region_info)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 attached_ds_badge = 1;
  if (this->attached_ds_badge() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->attached_ds_badge(), output);
  }

  // uint32 size = 2;
  if (this->size() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->size(), output);
  }

  // uint32 offset = 3;
  if (this->offset() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(3, this->offset(), output);
  }

  // uint32 rel_addr = 4;
  if (this->rel_addr() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(4, this->rel_addr(), output);
  }

  // bool executable = 5;
  if (this->executable() != 0) {
    ::google::protobuf::internal::WireFormatLite::WriteBool(5, this->executable(), output);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), output);
  }
  // @@protoc_insertion_point(serialize_end:protobuf.Stored_attached_region_info)
}

::google::protobuf::uint8* Stored_attached_region_info::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:protobuf.Stored_attached_region_info)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 attached_ds_badge = 1;
  if (this->attached_ds_badge() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(1, this->attached_ds_badge(), target);
  }

  // uint32 size = 2;
  if (this->size() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->size(), target);
  }

  // uint32 offset = 3;
  if (this->offset() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(3, this->offset(), target);
  }

  // uint32 rel_addr = 4;
  if (this->rel_addr() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(4, this->rel_addr(), target);
  }

  // bool executable = 5;
  if (this->executable() != 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteBoolToArray(5, this->executable(), target);
  }

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:protobuf.Stored_attached_region_info)
  return target;
}

size_t Stored_attached_region_info::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:protobuf.Stored_attached_region_info)
  size_t total_size = 0;

  if ((_internal_metadata_.have_unknown_fields() &&  ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        (::google::protobuf::internal::GetProto3PreserveUnknownsDefault()   ? _internal_metadata_.unknown_fields()   : _internal_metadata_.default_instance()));
  }
  // uint32 attached_ds_badge = 1;
  if (this->attached_ds_badge() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->attached_ds_badge());
  }

  // uint32 size = 2;
  if (this->size() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->size());
  }

  // uint32 offset = 3;
  if (this->offset() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->offset());
  }

  // uint32 rel_addr = 4;
  if (this->rel_addr() != 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->rel_addr());
  }

  // bool executable = 5;
  if (this->executable() != 0) {
    total_size += 1 + 1;
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Stored_attached_region_info::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:protobuf.Stored_attached_region_info)
  GOOGLE_DCHECK_NE(&from, this);
  const Stored_attached_region_info* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const Stored_attached_region_info>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:protobuf.Stored_attached_region_info)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:protobuf.Stored_attached_region_info)
    MergeFrom(*source);
  }
}

void Stored_attached_region_info::MergeFrom(const Stored_attached_region_info& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:protobuf.Stored_attached_region_info)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.attached_ds_badge() != 0) {
    set_attached_ds_badge(from.attached_ds_badge());
  }
  if (from.size() != 0) {
    set_size(from.size());
  }
  if (from.offset() != 0) {
    set_offset(from.offset());
  }
  if (from.rel_addr() != 0) {
    set_rel_addr(from.rel_addr());
  }
  if (from.executable() != 0) {
    set_executable(from.executable());
  }
}

void Stored_attached_region_info::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:protobuf.Stored_attached_region_info)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Stored_attached_region_info::CopyFrom(const Stored_attached_region_info& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:protobuf.Stored_attached_region_info)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Stored_attached_region_info::IsInitialized() const {
  return true;
}

void Stored_attached_region_info::Swap(Stored_attached_region_info* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Stored_attached_region_info::InternalSwap(Stored_attached_region_info* other) {
  using std::swap;
  swap(attached_ds_badge_, other->attached_ds_badge_);
  swap(size_, other->size_);
  swap(offset_, other->offset_);
  swap(rel_addr_, other->rel_addr_);
  swap(executable_, other->executable_);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata Stored_attached_region_info::GetMetadata() const {
  protobuf_stored_5fattached_5fregion_5finfo_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_stored_5fattached_5fregion_5finfo_2eproto::file_level_metadata[kIndexInFileMessages];
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf

// @@protoc_insertion_point(global_scope)