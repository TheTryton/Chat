#pragma once

#include <common/includes.hpp>
#include <common/data/participant.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN
MESSAGES_NAMESPACE_BEGIN

namespace detail
{
const boost::uuids::name_generator& guid_generator();
}

template<typename Message>
struct message_guid {};

template<typename Message>
concept GuidIdentifiableMessage = requires(Message msg){
    message_guid<Message>::guid;
};

template<GuidIdentifiableMessage Message>
boost::uuids::uuid get_message_guid()
{
    return message_guid<Message>::guid;
}

#define DECLARE_MESSAGE_GUID_IDENTIFIABLE(message_type) \
template<>                                              \
struct message_guid<message_type>                       \
{                                                       \
const static boost::uuids::uuid guid;                   \
};

#define DEFINE_MESSAGE_GUID_IDENTIFIABLE(message_type)                                              \
const boost::uuids::uuid message_guid<message_type>::guid = detail::guid_generator()(#message_type);\

MESSAGES_NAMESPACE_END
DATA_NAMESPACE_END
COMMON_NAMESPACE_END

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN
MESSAGES_NAMESPACE_BEGIN

enum class login_error
{
    duplicate_name,
    too_many_clients
};

struct login_request{};
struct login_attempt{QString client_name;};
struct login_reply{std::variant<boost::uuids::uuid, login_error> login_result;};

struct chat_message_id {boost::uuids::uuid sender; uint32_t chat_msg_num;};
struct chat_message_part_id {chat_message_id msg_id; uint16_t part;};

struct chat_message_begin { chat_message_id msg_id; uint16_t parts;};
struct chat_message_text_part { chat_message_part_id part_id; QString text; };
struct chat_message_image_part { chat_message_part_id part_id; QImage image; };
struct chat_message_end {chat_message_id msg_id;};
struct chat_message_simple { boost::uuids::uuid sender; QString text;};
struct udp_hi { boost::uuids::uuid sender;};
struct chat_message_complex { boost::uuids::uuid sender; QString long_text;};
struct request_chat_message_part_retransmission { chat_message_part_id part_id; };

struct request_change_name { boost::uuids::uuid sender; QString new_name;};
struct reply_change_name { std::variant<QString, login_error> result;};

struct participant_list_update { std::vector<std::pair<boost::uuids::uuid, QString>> participants;};
struct request_participant_list_update {};

DECLARE_MESSAGE_GUID_IDENTIFIABLE(login_request)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(login_attempt)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(login_reply)

DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_id)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_part_id)

DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_begin)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_text_part)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_image_part)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_end)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(request_chat_message_part_retransmission)

DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_simple)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(chat_message_complex)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(udp_hi)

DECLARE_MESSAGE_GUID_IDENTIFIABLE(request_change_name)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(reply_change_name)

DECLARE_MESSAGE_GUID_IDENTIFIABLE(participant_list_update)
DECLARE_MESSAGE_GUID_IDENTIFIABLE(request_participant_list_update)

MESSAGES_NAMESPACE_END
DATA_NAMESPACE_END
COMMON_NAMESPACE_END

SERIALIZATION_NAMESPACE_BEGIN

namespace msgs = COMMON_NAMESPACE_FULL::DATA_NAMESPACE::MESSAGES_NAMESPACE;

MAKE_CLASS_SERIALIZABLE(msgs::login_request, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary);
MAKE_CLASS_SERIALIZABLE(msgs::login_attempt, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::login_attempt::client_name);
MAKE_CLASS_SERIALIZABLE(msgs::login_reply, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::login_reply::login_result);

MAKE_CLASS_SERIALIZABLE(msgs::chat_message_id, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_id::sender,
                        &msgs::chat_message_id::chat_msg_num);
MAKE_CLASS_SERIALIZABLE(msgs::chat_message_part_id, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_part_id::msg_id,
                        &msgs::chat_message_part_id::part);

MAKE_CLASS_SERIALIZABLE(msgs::chat_message_begin, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_begin::msg_id,
                        &msgs::chat_message_begin::parts);
MAKE_CLASS_SERIALIZABLE(msgs::chat_message_text_part, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_text_part::part_id,
                        &msgs::chat_message_text_part::text);
MAKE_CLASS_SERIALIZABLE(msgs::chat_message_image_part, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_image_part::part_id,
                        &msgs::chat_message_image_part::image);
MAKE_CLASS_SERIALIZABLE(msgs::chat_message_end, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_end::msg_id);
MAKE_CLASS_SERIALIZABLE(msgs::request_chat_message_part_retransmission, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary);

MAKE_CLASS_SERIALIZABLE(msgs::chat_message_simple, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_simple::sender,
                        &msgs::chat_message_simple::text);
MAKE_CLASS_SERIALIZABLE(msgs::chat_message_complex, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::chat_message_complex::sender,
                        &msgs::chat_message_complex::long_text);
MAKE_CLASS_SERIALIZABLE(msgs::udp_hi, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::udp_hi::sender);

MAKE_CLASS_SERIALIZABLE(msgs::request_change_name, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::request_change_name::sender,
                        &msgs::request_change_name::new_name);
MAKE_CLASS_SERIALIZABLE(msgs::reply_change_name, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::reply_change_name::result);

MAKE_CLASS_SERIALIZABLE(msgs::participant_list_update, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary,
                        &msgs::participant_list_update::participants);
MAKE_CLASS_SERIALIZABLE(msgs::request_participant_list_update, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary);

SERIALIZATION_NAMESPACE_END