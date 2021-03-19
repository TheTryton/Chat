#pragma once

#include <common/includes.hpp>
#include <common/networking/resolver.hpp>
#include <common/networking/sockets.hpp>
#include <common/networking/message_channel_impl.hpp>
#include <server/networking/tcp_acceptor.hpp>

SERVER_NAMESPACE_BEGIN
LOGIC_NAMESPACE_BEGIN

namespace networking = ::SERVER_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL;
namespace messages = ::COMMON_NAMESPACE_FULL::MESSAGES_NAMESPACE_FULL;
namespace data = ::COMMON_NAMESPACE_FULL::DATA_NAMESPACE_FULL;

class QServerLogic final : public QObject
{
Q_OBJECT
public:
    enum class State
    {
        not_started,
        started,
    };
private:
    static boost::uuids::name_generator client_uuid_generator;
    static networking::QTcpMessageChannelSerializer binaryTcpSerializer;
    static networking::QTcpMessageChannelDeserializer binaryTcpDeserializer;
    static networking::QUdpMessageChannelSerializer binaryUdpSerializer;
    static networking::QUdpMessageChannelDeserializer binaryUdpDeserializer;
    using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    work_guard_type _work_guard;
    boost::asio::io_context& _context;
    QRecursiveMutex clients_mutex;
    struct client
    {
        std::unique_ptr<networking::QTcpMessageChannel> tcpMessageChannel;
        boost::asio::ip::udp::endpoint udpEndpoint;
        data::QChatParticipant chatParticipant;
        bool logged_in = false;
    };
    data::QChatParticipant _server;
    std::list<client> _clients;
    networking::QUdpMessageChannel udpMessageChannel;
    networking::QUdpMessageChannel udpMulticastMessageChannel;
    networking::QTcpAcceptor tcpAcceptor;
    State _currentState = State::not_started;
    bool blockDisconnectHandling = false;
public:
    explicit QServerLogic(boost::asio::io_context& context, QObject* parent = nullptr);
public:
    const State& currentState() const;
protected:
    void setCurrentState(const State& newState);
public slots:
    void start(const QString& port);
    void stop();
    void quit();

    void sendChatMessage(const QString& chatMessage);
private:
    void sendChatMessageWithSender(const QString& chatMessage, const data::QChatParticipant& sender);
    void sendSimpleMessage(const QString& simpleMessage, const data::QChatParticipant& sender);
    void sendUdpComplexMessage(const QString& complexMessage, const data::QChatParticipant& sender);
    void sendMulticastUdpComplexMessage(const QString& complexMessage, const data::QChatParticipant& sender);

    void broadcastMessageThroughTcp(const networking::QTcpMessageChannel::messages_variant& message);
    void broadcastMessageThroughUdp(const networking::QUdpMessageChannel::messages_variant& message);
    void broadcastMessageThroughUdpMulticast(const networking::QUdpMessageChannel::messages_variant& message);

    void informClientsListChanged();
signals:
    void currentStateChanged(const State& newState);
    void errorOccured(boost::system::error_code ec);
    void failedToAccept(boost::system::error_code reason);
    void stopped(boost::system::error_code reason);
    void clientsListChanged(const data::QChatParticipantList& participantsList);
    void chatMessageReceived(const data::QChatParticipant& sender, const QString& text);
private slots:
    void onClientConnected(const std::shared_ptr<networking::QTcpAsioSocket>& socket,
                           const boost::asio::ip::tcp::endpoint& destination);
    void onClientDisconnected(std::list<client>::iterator client, const boost::system::error_code& reason);
    void onFailedToAccept(const boost::system::error_code& reason);
    void onTcpMessageReceived(client& sender, const networking::QTcpMessageChannel::messages_variant& message);
    void onUdpMessageReceived(const boost::asio::ip::udp::endpoint& sender, const networking::QUdpMessageChannel::messages_variant& message);
private: // message handling
    void onMessageReceived(client& sender, const messages::login_request&);
    void onMessageReceived(client& sender, const messages::login_attempt&);
    void onMessageReceived(client& sender, const messages::login_reply&);
    void onMessageReceived(client& sender, const messages::chat_message_begin&);
    void onMessageReceived(client& sender, const messages::chat_message_text_part&);
    void onMessageReceived(client& sender, const messages::chat_message_image_part&);
    void onMessageReceived(client& sender, const messages::chat_message_end&);
    void onMessageReceived(client& sender, const messages::request_chat_message_part_retransmission&);
    void onMessageReceived(client& sender, const messages::chat_message_simple&);
    void onMessageReceived(client& sender, const messages::chat_message_complex&);
    void onMessageReceived(client& sender, const messages::udp_hi&);
    void onMessageReceived(client& sender, const messages::request_change_name&);
    void onMessageReceived(client& sender, const messages::reply_change_name&);
    void onMessageReceived(client& sender, const messages::participant_list_update&);
    void onMessageReceived(client& sender, const messages::request_participant_list_update&);
};

LOGIC_NAMESPACE_END
SERVER_NAMESPACE_END