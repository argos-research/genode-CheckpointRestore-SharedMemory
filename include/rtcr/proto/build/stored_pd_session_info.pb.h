// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: stored_pd_session_info.proto

#ifndef PROTOBUF_stored_5fpd_5fsession_5finfo_2eproto__INCLUDED
#define PROTOBUF_stored_5fpd_5fsession_5finfo_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3004000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "stored_signal_context_info.pb.h"
#include "stored_signal_source_info.pb.h"
#include "stored_native_capability_info.pb.h"
#include "stored_region_map_info.pb.h"
// @@protoc_insertion_point(includes)

namespace protobuf_stored_5fpd_5fsession_5finfo_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsStored_pd_session_infoImpl();
void InitDefaultsStored_pd_session_info();
inline void InitDefaults() {
  InitDefaultsStored_pd_session_info();
}
}  // namespace protobuf_stored_5fpd_5fsession_5finfo_2eproto
namespace protobuf {
class Stored_pd_session_info;
class Stored_pd_session_infoDefaultTypeInternal;
extern Stored_pd_session_infoDefaultTypeInternal _Stored_pd_session_info_default_instance_;
}  // namespace protobuf
namespace protobuf {

// ===================================================================

class Stored_pd_session_info : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:protobuf.Stored_pd_session_info) */ {
 public:
  Stored_pd_session_info();
  virtual ~Stored_pd_session_info();

  Stored_pd_session_info(const Stored_pd_session_info& from);

  inline Stored_pd_session_info& operator=(const Stored_pd_session_info& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Stored_pd_session_info(Stored_pd_session_info&& from) noexcept
    : Stored_pd_session_info() {
    *this = ::std::move(from);
  }

  inline Stored_pd_session_info& operator=(Stored_pd_session_info&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Stored_pd_session_info& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Stored_pd_session_info* internal_default_instance() {
    return reinterpret_cast<const Stored_pd_session_info*>(
               &_Stored_pd_session_info_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Stored_pd_session_info* other);
  friend void swap(Stored_pd_session_info& a, Stored_pd_session_info& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Stored_pd_session_info* New() const PROTOBUF_FINAL { return New(NULL); }

  Stored_pd_session_info* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Stored_pd_session_info& from);
  void MergeFrom(const Stored_pd_session_info& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Stored_pd_session_info* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .protobuf.Stored_signal_context_info stored_context_infos = 1;
  int stored_context_infos_size() const;
  void clear_stored_context_infos();
  static const int kStoredContextInfosFieldNumber = 1;
  const ::protobuf::Stored_signal_context_info& stored_context_infos(int index) const;
  ::protobuf::Stored_signal_context_info* mutable_stored_context_infos(int index);
  ::protobuf::Stored_signal_context_info* add_stored_context_infos();
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_context_info >*
      mutable_stored_context_infos();
  const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_context_info >&
      stored_context_infos() const;

  // repeated .protobuf.Stored_signal_source_info stored_source_infos = 2;
  int stored_source_infos_size() const;
  void clear_stored_source_infos();
  static const int kStoredSourceInfosFieldNumber = 2;
  const ::protobuf::Stored_signal_source_info& stored_source_infos(int index) const;
  ::protobuf::Stored_signal_source_info* mutable_stored_source_infos(int index);
  ::protobuf::Stored_signal_source_info* add_stored_source_infos();
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_source_info >*
      mutable_stored_source_infos();
  const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_source_info >&
      stored_source_infos() const;

  // repeated .protobuf.Stored_native_capability_info stored_native_cap_infos = 3;
  int stored_native_cap_infos_size() const;
  void clear_stored_native_cap_infos();
  static const int kStoredNativeCapInfosFieldNumber = 3;
  const ::protobuf::Stored_native_capability_info& stored_native_cap_infos(int index) const;
  ::protobuf::Stored_native_capability_info* mutable_stored_native_cap_infos(int index);
  ::protobuf::Stored_native_capability_info* add_stored_native_cap_infos();
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_native_capability_info >*
      mutable_stored_native_cap_infos();
  const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_native_capability_info >&
      stored_native_cap_infos() const;

  // @@protoc_insertion_point(class_scope:protobuf.Stored_pd_session_info)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_context_info > stored_context_infos_;
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_source_info > stored_source_infos_;
  ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_native_capability_info > stored_native_cap_infos_;
  mutable int _cached_size_;
  friend struct ::protobuf_stored_5fpd_5fsession_5finfo_2eproto::TableStruct;
  friend void ::protobuf_stored_5fpd_5fsession_5finfo_2eproto::InitDefaultsStored_pd_session_infoImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Stored_pd_session_info

// repeated .protobuf.Stored_signal_context_info stored_context_infos = 1;
inline int Stored_pd_session_info::stored_context_infos_size() const {
  return stored_context_infos_.size();
}
inline const ::protobuf::Stored_signal_context_info& Stored_pd_session_info::stored_context_infos(int index) const {
  // @@protoc_insertion_point(field_get:protobuf.Stored_pd_session_info.stored_context_infos)
  return stored_context_infos_.Get(index);
}
inline ::protobuf::Stored_signal_context_info* Stored_pd_session_info::mutable_stored_context_infos(int index) {
  // @@protoc_insertion_point(field_mutable:protobuf.Stored_pd_session_info.stored_context_infos)
  return stored_context_infos_.Mutable(index);
}
inline ::protobuf::Stored_signal_context_info* Stored_pd_session_info::add_stored_context_infos() {
  // @@protoc_insertion_point(field_add:protobuf.Stored_pd_session_info.stored_context_infos)
  return stored_context_infos_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_context_info >*
Stored_pd_session_info::mutable_stored_context_infos() {
  // @@protoc_insertion_point(field_mutable_list:protobuf.Stored_pd_session_info.stored_context_infos)
  return &stored_context_infos_;
}
inline const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_context_info >&
Stored_pd_session_info::stored_context_infos() const {
  // @@protoc_insertion_point(field_list:protobuf.Stored_pd_session_info.stored_context_infos)
  return stored_context_infos_;
}

// repeated .protobuf.Stored_signal_source_info stored_source_infos = 2;
inline int Stored_pd_session_info::stored_source_infos_size() const {
  return stored_source_infos_.size();
}
inline const ::protobuf::Stored_signal_source_info& Stored_pd_session_info::stored_source_infos(int index) const {
  // @@protoc_insertion_point(field_get:protobuf.Stored_pd_session_info.stored_source_infos)
  return stored_source_infos_.Get(index);
}
inline ::protobuf::Stored_signal_source_info* Stored_pd_session_info::mutable_stored_source_infos(int index) {
  // @@protoc_insertion_point(field_mutable:protobuf.Stored_pd_session_info.stored_source_infos)
  return stored_source_infos_.Mutable(index);
}
inline ::protobuf::Stored_signal_source_info* Stored_pd_session_info::add_stored_source_infos() {
  // @@protoc_insertion_point(field_add:protobuf.Stored_pd_session_info.stored_source_infos)
  return stored_source_infos_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_source_info >*
Stored_pd_session_info::mutable_stored_source_infos() {
  // @@protoc_insertion_point(field_mutable_list:protobuf.Stored_pd_session_info.stored_source_infos)
  return &stored_source_infos_;
}
inline const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_signal_source_info >&
Stored_pd_session_info::stored_source_infos() const {
  // @@protoc_insertion_point(field_list:protobuf.Stored_pd_session_info.stored_source_infos)
  return stored_source_infos_;
}

// repeated .protobuf.Stored_native_capability_info stored_native_cap_infos = 3;
inline int Stored_pd_session_info::stored_native_cap_infos_size() const {
  return stored_native_cap_infos_.size();
}
inline const ::protobuf::Stored_native_capability_info& Stored_pd_session_info::stored_native_cap_infos(int index) const {
  // @@protoc_insertion_point(field_get:protobuf.Stored_pd_session_info.stored_native_cap_infos)
  return stored_native_cap_infos_.Get(index);
}
inline ::protobuf::Stored_native_capability_info* Stored_pd_session_info::mutable_stored_native_cap_infos(int index) {
  // @@protoc_insertion_point(field_mutable:protobuf.Stored_pd_session_info.stored_native_cap_infos)
  return stored_native_cap_infos_.Mutable(index);
}
inline ::protobuf::Stored_native_capability_info* Stored_pd_session_info::add_stored_native_cap_infos() {
  // @@protoc_insertion_point(field_add:protobuf.Stored_pd_session_info.stored_native_cap_infos)
  return stored_native_cap_infos_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_native_capability_info >*
Stored_pd_session_info::mutable_stored_native_cap_infos() {
  // @@protoc_insertion_point(field_mutable_list:protobuf.Stored_pd_session_info.stored_native_cap_infos)
  return &stored_native_cap_infos_;
}
inline const ::google::protobuf::RepeatedPtrField< ::protobuf::Stored_native_capability_info >&
Stored_pd_session_info::stored_native_cap_infos() const {
  // @@protoc_insertion_point(field_list:protobuf.Stored_pd_session_info.stored_native_cap_infos)
  return stored_native_cap_infos_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_stored_5fpd_5fsession_5finfo_2eproto__INCLUDED