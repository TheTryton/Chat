#pragma once

#include <common/networking/message_channel.hpp>

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

using QTcpMessageChannelSerializer = messages::QBinaryMessageSerializer<
    messages::login_request,
    messages::login_attempt,
    messages::login_reply,
    messages::chat_message_simple,
    messages::request_change_name,
    messages::reply_change_name,
    messages::participant_list_update,
    messages::request_participant_list_update>;
using QTcpMessageChannelDeserializer = messages::QBinaryMessageDeserializer<
    messages::login_request,
    messages::login_attempt,
    messages::login_reply,
    messages::chat_message_simple,
    messages::request_change_name,
    messages::reply_change_name,
    messages::participant_list_update,
    messages::request_participant_list_update>;
using QUdpMessageChannelSerializer = messages::QBinaryMessageSerializer<
    messages::chat_message_complex,
    messages::udp_hi>;
using QUdpMessageChannelDeserializer = messages::QBinaryMessageDeserializer<
    messages::chat_message_complex,
    messages::udp_hi>;

using QTcpMessageChannelBase = QTcpAsioSocketMessageChannel<
    messages::login_request,
    messages::login_attempt,
    messages::login_reply,
    messages::chat_message_simple,
    messages::request_change_name,
    messages::reply_change_name,
    messages::participant_list_update,
    messages::request_participant_list_update
>;
using QUdpMessageChannelBase = QUdpAsioSocketMessageChannel<
    messages::chat_message_complex,
    messages::udp_hi
>;

class QTcpMessageChannel : public QTcpMessageChannelBase
{
Q_OBJECT
public:
    using QTcpMessageChannelBase::QTcpMessageChannelBase;
protected:
    virtual void handleReceivedData(QByteArray&& serializedData) override;
    virtual void handleError(const boost::system::error_code& ec) override;
    virtual void handleDisconnect(const boost::system::error_code& ec) override;
signals:
    void disconnected(boost::system::error_code ec);
    void errorOccured(boost::system::error_code ec);
    void messageReceived(messages_variant message);
};
class QUdpMessageChannel : public QUdpMessageChannelBase
{
Q_OBJECT
public:
    using QUdpMessageChannelBase::QUdpMessageChannelBase;
protected:
    virtual void handleReceivedData(const boost::asio::ip::udp::endpoint& sender, QByteArray&& serializedData) override;
    virtual void handleError(const boost::system::error_code& ec) override;
    virtual void handleDisconnect(const boost::system::error_code& ec) override;
signals:
    void disconnected(boost::system::error_code ec);
    void errorOccured(boost::system::error_code ec);
    void messageReceived(boost::asio::ip::udp::endpoint sender, messages_variant message);
};

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END

Q_DECLARE_METATYPE(COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL::QTcpMessageChannel::messages_variant)
Q_DECLARE_METATYPE(COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL::QUdpMessageChannel::messages_variant)
Q_DECLARE_METATYPE(boost::asio::ip::tcp::endpoint)
Q_DECLARE_METATYPE(boost::asio::ip::udp::endpoint)