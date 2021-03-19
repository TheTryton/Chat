#pragma once

#include <common/includes.hpp>
#include <common/networking/resolver.hpp>
#include <common/networking/sockets.hpp>
#include <common/networking/message_channel_impl.hpp>
#include <client/networking/tcp_connector.hpp>

CLIENT_NAMESPACE_BEGIN
LOGIC_NAMESPACE_BEGIN

namespace networking = ::CLIENT_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL;
namespace messages = ::COMMON_NAMESPACE_FULL::MESSAGES_NAMESPACE_FULL;
namespace data = ::COMMON_NAMESPACE_FULL::DATA_NAMESPACE_FULL;

class QClientLogic final : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        disconnected,
        resolving,
        connecting,
        connected,
        logging_in,
        logged_in
    };
private:
    static networking::QTcpMessageChannelSerializer binaryTcpSerializer;
    static networking::QTcpMessageChannelDeserializer binaryTcpDeserializer;
    static networking::QUdpMessageChannelSerializer binaryUdpSerializer;
    static networking::QUdpMessageChannelDeserializer binaryUdpDeserializer;
    using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    work_guard_type _work_guard;
    boost::asio::io_context& _context;
    networking::QTcpMessageChannel tcpMessageChannel;
    networking::QUdpMessageChannel udpMessageChannel;
    networking::QUdpMessageChannel udpMulticastMessageChannel;
    networking::QTcpResolver tcpResolver;
    networking::QUdpResolver udpResolver;
    networking::QTcpConnector tcpConnector;
    data::QChatParticipant _client;
    data::QChatParticipantList _clients;
    boost::asio::ip::udp::endpoint _server_endpoint;
    State _currentState = State::disconnected;
    QQueue<std::pair<boost::uuids::uuid, QString>> _unidentified_messages;
    bool waitingForParticipantsUpdate = false;
public:
    explicit QClientLogic(boost::asio::io_context& context, QObject* parent = nullptr);
public:
    const State& currentState() const;
protected:
    void setCurrentState(const State& newState);
public slots:
    void resolveAndConnect(const QString& address);
    void logIn(const QString& name);
    void changeName(const QString& newName);
    void requestParticipantListUpdate();
    void disconnect();
    void quit();

    void sendChatMessage(const QString& chatMessage);
private:
    void sendSimpleMessage(const QString& simpleMessage);
    void sendUdpComplexMessage(const QString& complexMessage);
    void sendMulticastUdpComplexMessage(const QString& complexMessage);

    std::optional<data::QChatParticipant> findClient(const boost::uuids::uuid& uuid) const;
signals:
    void currentStateChanged(const State& newState);
    void errorOccured(boost::system::error_code ec);
    void failedToConnect(boost::system::error_code reason);
    void failedToLogIn(messages::login_error reason);
    void connectionLost(boost::system::error_code reason);
    void failedToChangeClientName(messages::login_error reason);
    void clientNameChanged(const QString& newName);
    void clientUuidChanged(const boost::uuids::uuid& newUuid);
    void clientsListChanged(const data::QChatParticipantList& participantsList);
    void chatMessageReceived(data::QChatParticipant sender, QString chatMessage);
private slots:
    void onResolved(const boost::asio::ip::tcp::resolver::results_type& results);
    void onFailedToResolve(const boost::system::error_code& reason);
    void onConnected(const std::shared_ptr<networking::QTcpAsioSocket>& socket,
                     const boost::asio::ip::tcp::endpoint& destination);
    void onFailedToConnect(const boost::system::error_code& reason);
    void onTcpMessageReceived(const networking::QTcpMessageChannel::messages_variant& message);
    void onUdpMessageReceived(const boost::asio::ip::udp::endpoint& sender,
                              const networking::QUdpMessageChannel::messages_variant& message);
    void onLoggedIn(const boost::uuids::uuid& uuid);
    void onFailedToLogIn(const messages::login_error&);
    void onConnectionLost(const boost::system::error_code& reason);
private: // message handling
    void onMessageReceived(const messages::login_request&);
    void onMessageReceived(const messages::login_attempt&);
    void onMessageReceived(const messages::login_reply&);
    void onMessageReceived(const messages::chat_message_begin&);
    void onMessageReceived(const messages::chat_message_text_part&);
    void onMessageReceived(const messages::chat_message_image_part&);
    void onMessageReceived(const messages::chat_message_end&);
    void onMessageReceived(const messages::request_chat_message_part_retransmission&);
    void onMessageReceived(const messages::chat_message_simple&);
    void onMessageReceived(const messages::chat_message_complex&);
    void onMessageReceived(const messages::udp_hi&);
    void onMessageReceived(const messages::request_change_name&);
    void onMessageReceived(const messages::reply_change_name&);
    void onMessageReceived(const messages::participant_list_update&);
    void onMessageReceived(const messages::request_participant_list_update&);
};

LOGIC_NAMESPACE_END
CLIENT_NAMESPACE_END