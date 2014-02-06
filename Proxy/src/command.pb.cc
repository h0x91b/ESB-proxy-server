// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: command.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "command.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace ESB {

namespace {

const ::google::protobuf::Descriptor* Command_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Command_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* Command_Cmd_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_command_2eproto() {
  protobuf_AddDesc_command_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "command.proto");
  GOOGLE_CHECK(file != NULL);
  Command_descriptor_ = file->message_type(0);
  static const int Command_offsets_[10] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, cmd_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, identifier_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, guid_from_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, guid_to_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, source_proxy_guid_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, target_proxy_guid_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, version_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, start_time_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, timeout_ms_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, payload_),
  };
  Command_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      Command_descriptor_,
      Command::default_instance_,
      Command_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Command, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(Command));
  Command_Cmd_descriptor_ = Command_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_command_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    Command_descriptor_, &Command::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_command_2eproto() {
  delete Command::default_instance_;
  delete Command_reflection_;
}

void protobuf_AddDesc_command_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rcommand.proto\022\003ESB\"\260\002\n\007Command\022\035\n\003cmd\030"
    "\001 \002(\0162\020.ESB.Command.Cmd\022\022\n\nidentifier\030\002 "
    "\001(\t\022\021\n\tguid_from\030\003 \001(\t\022\017\n\007guid_to\030\004 \001(\t\022"
    "\031\n\021source_proxy_guid\030\005 \001(\t\022\031\n\021target_pro"
    "xy_guid\030\006 \001(\t\022\017\n\007version\030\007 \001(\005\022\022\n\nstart_"
    "time\030\010 \001(\003\022\022\n\ntimeout_ms\030\t \001(\005\022\017\n\007payloa"
    "d\030\n \002(\t\"N\n\003Cmd\022\t\n\005ERROR\020\001\022\014\n\010RESPONSE\020\002\022"
    "\016\n\nNODE_HELLO\020\003\022\010\n\004PING\020\004\022\010\n\004PONG\020\005\022\n\n\006I"
    "NVOKE\020\006", 327);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "command.proto", &protobuf_RegisterTypes);
  Command::default_instance_ = new Command();
  Command::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_command_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_command_2eproto {
  StaticDescriptorInitializer_command_2eproto() {
    protobuf_AddDesc_command_2eproto();
  }
} static_descriptor_initializer_command_2eproto_;

// ===================================================================

const ::google::protobuf::EnumDescriptor* Command_Cmd_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Command_Cmd_descriptor_;
}
bool Command_Cmd_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const Command_Cmd Command::ERROR;
const Command_Cmd Command::RESPONSE;
const Command_Cmd Command::NODE_HELLO;
const Command_Cmd Command::PING;
const Command_Cmd Command::PONG;
const Command_Cmd Command::INVOKE;
const Command_Cmd Command::Cmd_MIN;
const Command_Cmd Command::Cmd_MAX;
const int Command::Cmd_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int Command::kCmdFieldNumber;
const int Command::kIdentifierFieldNumber;
const int Command::kGuidFromFieldNumber;
const int Command::kGuidToFieldNumber;
const int Command::kSourceProxyGuidFieldNumber;
const int Command::kTargetProxyGuidFieldNumber;
const int Command::kVersionFieldNumber;
const int Command::kStartTimeFieldNumber;
const int Command::kTimeoutMsFieldNumber;
const int Command::kPayloadFieldNumber;
#endif  // !_MSC_VER

Command::Command()
  : ::google::protobuf::Message() {
  SharedCtor();
}

void Command::InitAsDefaultInstance() {
}

Command::Command(const Command& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void Command::SharedCtor() {
  _cached_size_ = 0;
  cmd_ = 1;
  identifier_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  guid_from_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  guid_to_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  source_proxy_guid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  target_proxy_guid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  version_ = 0;
  start_time_ = GOOGLE_LONGLONG(0);
  timeout_ms_ = 0;
  payload_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Command::~Command() {
  SharedDtor();
}

void Command::SharedDtor() {
  if (identifier_ != &::google::protobuf::internal::kEmptyString) {
    delete identifier_;
  }
  if (guid_from_ != &::google::protobuf::internal::kEmptyString) {
    delete guid_from_;
  }
  if (guid_to_ != &::google::protobuf::internal::kEmptyString) {
    delete guid_to_;
  }
  if (source_proxy_guid_ != &::google::protobuf::internal::kEmptyString) {
    delete source_proxy_guid_;
  }
  if (target_proxy_guid_ != &::google::protobuf::internal::kEmptyString) {
    delete target_proxy_guid_;
  }
  if (payload_ != &::google::protobuf::internal::kEmptyString) {
    delete payload_;
  }
  if (this != default_instance_) {
  }
}

void Command::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Command::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Command_descriptor_;
}

const Command& Command::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_command_2eproto();
  return *default_instance_;
}

Command* Command::default_instance_ = NULL;

Command* Command::New() const {
  return new Command;
}

void Command::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    cmd_ = 1;
    if (has_identifier()) {
      if (identifier_ != &::google::protobuf::internal::kEmptyString) {
        identifier_->clear();
      }
    }
    if (has_guid_from()) {
      if (guid_from_ != &::google::protobuf::internal::kEmptyString) {
        guid_from_->clear();
      }
    }
    if (has_guid_to()) {
      if (guid_to_ != &::google::protobuf::internal::kEmptyString) {
        guid_to_->clear();
      }
    }
    if (has_source_proxy_guid()) {
      if (source_proxy_guid_ != &::google::protobuf::internal::kEmptyString) {
        source_proxy_guid_->clear();
      }
    }
    if (has_target_proxy_guid()) {
      if (target_proxy_guid_ != &::google::protobuf::internal::kEmptyString) {
        target_proxy_guid_->clear();
      }
    }
    version_ = 0;
    start_time_ = GOOGLE_LONGLONG(0);
  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    timeout_ms_ = 0;
    if (has_payload()) {
      if (payload_ != &::google::protobuf::internal::kEmptyString) {
        payload_->clear();
      }
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool Command::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .ESB.Command.Cmd cmd = 1;
      case 1: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::ESB::Command_Cmd_IsValid(value)) {
            set_cmd(static_cast< ::ESB::Command_Cmd >(value));
          } else {
            mutable_unknown_fields()->AddVarint(1, value);
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(18)) goto parse_identifier;
        break;
      }

      // optional string identifier = 2;
      case 2: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_identifier:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_identifier()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->identifier().data(), this->identifier().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_guid_from;
        break;
      }

      // optional string guid_from = 3;
      case 3: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_guid_from:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_guid_from()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->guid_from().data(), this->guid_from().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(34)) goto parse_guid_to;
        break;
      }

      // optional string guid_to = 4;
      case 4: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_guid_to:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_guid_to()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->guid_to().data(), this->guid_to().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(42)) goto parse_source_proxy_guid;
        break;
      }

      // optional string source_proxy_guid = 5;
      case 5: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_source_proxy_guid:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_source_proxy_guid()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->source_proxy_guid().data(), this->source_proxy_guid().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(50)) goto parse_target_proxy_guid;
        break;
      }

      // optional string target_proxy_guid = 6;
      case 6: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_target_proxy_guid:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_target_proxy_guid()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->target_proxy_guid().data(), this->target_proxy_guid().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(56)) goto parse_version;
        break;
      }

      // optional int32 version = 7;
      case 7: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_version:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &version_)));
          set_has_version();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(64)) goto parse_start_time;
        break;
      }

      // optional int64 start_time = 8;
      case 8: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_start_time:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int64, ::google::protobuf::internal::WireFormatLite::TYPE_INT64>(
                 input, &start_time_)));
          set_has_start_time();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(72)) goto parse_timeout_ms;
        break;
      }

      // optional int32 timeout_ms = 9;
      case 9: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_timeout_ms:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &timeout_ms_)));
          set_has_timeout_ms();
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(82)) goto parse_payload;
        break;
      }

      // required string payload = 10;
      case 10: {
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_payload:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_payload()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8String(
            this->payload().data(), this->payload().length(),
            ::google::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void Command::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // required .ESB.Command.Cmd cmd = 1;
  if (has_cmd()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      1, this->cmd(), output);
  }

  // optional string identifier = 2;
  if (has_identifier()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->identifier().data(), this->identifier().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      2, this->identifier(), output);
  }

  // optional string guid_from = 3;
  if (has_guid_from()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->guid_from().data(), this->guid_from().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      3, this->guid_from(), output);
  }

  // optional string guid_to = 4;
  if (has_guid_to()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->guid_to().data(), this->guid_to().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      4, this->guid_to(), output);
  }

  // optional string source_proxy_guid = 5;
  if (has_source_proxy_guid()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->source_proxy_guid().data(), this->source_proxy_guid().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      5, this->source_proxy_guid(), output);
  }

  // optional string target_proxy_guid = 6;
  if (has_target_proxy_guid()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->target_proxy_guid().data(), this->target_proxy_guid().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      6, this->target_proxy_guid(), output);
  }

  // optional int32 version = 7;
  if (has_version()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(7, this->version(), output);
  }

  // optional int64 start_time = 8;
  if (has_start_time()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64(8, this->start_time(), output);
  }

  // optional int32 timeout_ms = 9;
  if (has_timeout_ms()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(9, this->timeout_ms(), output);
  }

  // required string payload = 10;
  if (has_payload()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->payload().data(), this->payload().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    ::google::protobuf::internal::WireFormatLite::WriteString(
      10, this->payload(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google::protobuf::uint8* Command::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // required .ESB.Command.Cmd cmd = 1;
  if (has_cmd()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      1, this->cmd(), target);
  }

  // optional string identifier = 2;
  if (has_identifier()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->identifier().data(), this->identifier().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->identifier(), target);
  }

  // optional string guid_from = 3;
  if (has_guid_from()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->guid_from().data(), this->guid_from().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->guid_from(), target);
  }

  // optional string guid_to = 4;
  if (has_guid_to()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->guid_to().data(), this->guid_to().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        4, this->guid_to(), target);
  }

  // optional string source_proxy_guid = 5;
  if (has_source_proxy_guid()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->source_proxy_guid().data(), this->source_proxy_guid().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        5, this->source_proxy_guid(), target);
  }

  // optional string target_proxy_guid = 6;
  if (has_target_proxy_guid()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->target_proxy_guid().data(), this->target_proxy_guid().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        6, this->target_proxy_guid(), target);
  }

  // optional int32 version = 7;
  if (has_version()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(7, this->version(), target);
  }

  // optional int64 start_time = 8;
  if (has_start_time()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt64ToArray(8, this->start_time(), target);
  }

  // optional int32 timeout_ms = 9;
  if (has_timeout_ms()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(9, this->timeout_ms(), target);
  }

  // required string payload = 10;
  if (has_payload()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8String(
      this->payload().data(), this->payload().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        10, this->payload(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int Command::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required .ESB.Command.Cmd cmd = 1;
    if (has_cmd()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->cmd());
    }

    // optional string identifier = 2;
    if (has_identifier()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->identifier());
    }

    // optional string guid_from = 3;
    if (has_guid_from()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->guid_from());
    }

    // optional string guid_to = 4;
    if (has_guid_to()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->guid_to());
    }

    // optional string source_proxy_guid = 5;
    if (has_source_proxy_guid()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->source_proxy_guid());
    }

    // optional string target_proxy_guid = 6;
    if (has_target_proxy_guid()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->target_proxy_guid());
    }

    // optional int32 version = 7;
    if (has_version()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->version());
    }

    // optional int64 start_time = 8;
    if (has_start_time()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int64Size(
          this->start_time());
    }

  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    // optional int32 timeout_ms = 9;
    if (has_timeout_ms()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->timeout_ms());
    }

    // required string payload = 10;
    if (has_payload()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->payload());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Command::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const Command* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Command*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Command::MergeFrom(const Command& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_cmd()) {
      set_cmd(from.cmd());
    }
    if (from.has_identifier()) {
      set_identifier(from.identifier());
    }
    if (from.has_guid_from()) {
      set_guid_from(from.guid_from());
    }
    if (from.has_guid_to()) {
      set_guid_to(from.guid_to());
    }
    if (from.has_source_proxy_guid()) {
      set_source_proxy_guid(from.source_proxy_guid());
    }
    if (from.has_target_proxy_guid()) {
      set_target_proxy_guid(from.target_proxy_guid());
    }
    if (from.has_version()) {
      set_version(from.version());
    }
    if (from.has_start_time()) {
      set_start_time(from.start_time());
    }
  }
  if (from._has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (from.has_timeout_ms()) {
      set_timeout_ms(from.timeout_ms());
    }
    if (from.has_payload()) {
      set_payload(from.payload());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void Command::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Command::CopyFrom(const Command& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Command::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000201) != 0x00000201) return false;

  return true;
}

void Command::Swap(Command* other) {
  if (other != this) {
    std::swap(cmd_, other->cmd_);
    std::swap(identifier_, other->identifier_);
    std::swap(guid_from_, other->guid_from_);
    std::swap(guid_to_, other->guid_to_);
    std::swap(source_proxy_guid_, other->source_proxy_guid_);
    std::swap(target_proxy_guid_, other->target_proxy_guid_);
    std::swap(version_, other->version_);
    std::swap(start_time_, other->start_time_);
    std::swap(timeout_ms_, other->timeout_ms_);
    std::swap(payload_, other->payload_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata Command::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Command_descriptor_;
  metadata.reflection = Command_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace ESB

// @@protoc_insertion_point(global_scope)
