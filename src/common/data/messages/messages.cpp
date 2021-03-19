#include <common/data/messages/messages.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN
MESSAGES_NAMESPACE_BEGIN

namespace detail
{
const boost::uuids::name_generator& guid_generator()
{
    static const boost::uuids::name_generator generator = boost::uuids::name_generator{boost::uuids::uuid{}};
    return generator;
}
}

DEFINE_MESSAGE_GUID_IDENTIFIABLE(login_request)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(login_attempt)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(login_reply)

DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_id)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_part_id)

DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_begin)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_text_part)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_image_part)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(request_chat_message_part_retransmission)

DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_simple)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(chat_message_complex)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(udp_hi)

DEFINE_MESSAGE_GUID_IDENTIFIABLE(request_change_name)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(reply_change_name)

DEFINE_MESSAGE_GUID_IDENTIFIABLE(participant_list_update)
DEFINE_MESSAGE_GUID_IDENTIFIABLE(request_participant_list_update)

MESSAGES_NAMESPACE_END
DATA_NAMESPACE_END
COMMON_NAMESPACE_END
