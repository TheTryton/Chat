#include <server/logic/server.hpp>

SERVER_NAMESPACE_BEGIN
LOGIC_NAMESPACE_BEGIN

networking::QTcpMessageChannelSerializer QServerLogic::binaryTcpSerializer{};
networking::QTcpMessageChannelDeserializer QServerLogic::binaryTcpDeserializer{};
networking::QUdpMessageChannelSerializer QServerLogic::binaryUdpSerializer{};
networking::QUdpMessageChannelDeserializer QServerLogic::binaryUdpDeserializer{};
boost::uuids::name_generator QServerLogic::client_uuid_generator{boost::uuids::uuid{}};

QServerLogic::QServerLogic(boost::asio::io_context& context, QObject* parent)
    : QObject(parent)
    , _work_guard(context.get_executor())
    , _context(context)
    , udpMessageChannel(binaryUdpSerializer, binaryUdpDeserializer)
    , udpMulticastMessageChannel(binaryUdpSerializer, binaryUdpDeserializer)
    , _server(client_uuid_generator("Server"), "Server")
{
    tcpAcceptor.setContext(context);

    QObject::connect(&tcpAcceptor, &networking::QTcpAcceptor::connectionEstablished, this, &QServerLogic::onClientConnected, Qt::QueuedConnection);
    QObject::connect(&tcpAcceptor, &networking::QTcpAcceptor::failedToAccept, this, &QServerLogic::failedToAccept, Qt::QueuedConnection);

    QObject::connect(&udpMessageChannel, &networking::QUdpMessageChannel::messageReceived, this, &QServerLogic::onUdpMessageReceived, Qt::QueuedConnection);
}

const QServerLogic::State& QServerLogic::currentState() const
{
    return _currentState;
}

void QServerLogic::setCurrentState(const State& newState)
{
    if(_currentState != newState)
    {
        _currentState = newState;
        emit currentStateChanged(_currentState);
    }
}

void QServerLogic::start(const QString& portStr)
{
    bool ok = false;
    auto portInt = portStr.toInt(&ok);
    if(!ok || portInt > std::numeric_limits<uint16_t>::max() || portInt < 0)
    {
        onFailedToAccept(boost::asio::error::invalid_argument);
    }
    else
    {
        tcpAcceptor.startAccepting(static_cast<uint16_t>(portInt));
        boost::asio::ip::udp::socket udp_socket{
            _context,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), static_cast<uint16_t>(portInt))};
        udpMessageChannel.start(std::make_unique<networking::QUdpAsioSocket>(std::move(udp_socket)));
        /*boost::asio::ip::udp::socket multicast_socket{_context};
        udpMulticastMessageChannel.start(std::make_unique<networking::QUdpAsioSocket>(std::move(multicast_socket)));*/
        setCurrentState(State::started);
        informClientsListChanged();
    }
}

void QServerLogic::stop()
{
    blockDisconnectHandling=true;
    _clients.clear();
    blockDisconnectHandling=false;
    informClientsListChanged();
    udpMessageChannel.stop();
    udpMulticastMessageChannel.stop();
    tcpAcceptor.stopAccepting();
    setCurrentState(State::not_started);
    emit stopped(boost::asio::error::connection_reset);
}

void QServerLogic::quit()
{
    stop();
    QApplication::quit();
}

void QServerLogic::sendChatMessageWithSender(const QString& chatMessage, const data::QChatParticipant& sender)
{
    if(chatMessage.startsWith("-U"))
    {
        sendUdpComplexMessage(chatMessage, sender);
    }
    else if(chatMessage.startsWith("-M"))
    {
        sendMulticastUdpComplexMessage(chatMessage, sender);
    }
    else
    {
        sendSimpleMessage(chatMessage, sender);
    }
    emit chatMessageReceived(sender, chatMessage);
}

void QServerLogic::sendChatMessage(const QString& chatMessage)
{
    sendChatMessageWithSender(chatMessage, _server);
}

void QServerLogic::sendSimpleMessage(const QString& chatMessage, const data::QChatParticipant& sender)
{
    broadcastMessageThroughTcp(messages::chat_message_simple{.sender = sender.uuid(), .text = chatMessage});
}

void QServerLogic::sendUdpComplexMessage(const QString& chatMessage, const data::QChatParticipant& sender)
{
    broadcastMessageThroughUdp(messages::chat_message_complex{.sender = sender.uuid(), .long_text = chatMessage});
}

void QServerLogic::sendMulticastUdpComplexMessage(const QString& chatMessage, const data::QChatParticipant& sender)
{
    broadcastMessageThroughUdpMulticast(messages::chat_message_complex{.sender = sender.uuid(), .long_text = chatMessage});
}

void QServerLogic::broadcastMessageThroughTcp(const networking::QTcpMessageChannel::messages_variant& message)
{
    std::for_each(std::begin(_clients), std::end(_clients), [&message](auto& client){
       if(client.logged_in)
       {
           client.tcpMessageChannel->postMessage(message);
       }
    });
}

void QServerLogic::broadcastMessageThroughUdp(const networking::QUdpMessageChannel::messages_variant& message)
{
    std::for_each(std::begin(_clients), std::end(_clients), [this, &message](auto& client){
        if(client.logged_in)
        {
            udpMessageChannel.postMessage(client.udpEndpoint, message);
        }
    });
}

void QServerLogic::broadcastMessageThroughUdpMulticast(const networking::QUdpMessageChannel::messages_variant& message)
{
    /*
    udpMulticastMessageChannel.postMessage(boost::asio::ip::udp::endpoint{
        boost::asio::ip::address::from_string(networking::multicast_address),
        networking::multicast_port
    }, message);*/
}

void QServerLogic::informClientsListChanged()
{
    QMutexLocker l{&clients_mutex};
    data::QChatParticipantList clients;
    std::for_each(std::begin(_clients), std::end(_clients),[&clients](const auto& c){
        if(c.logged_in) clients.push_back(c.chatParticipant);
    });
    clients.push_back(_server);
    emit clientsListChanged(clients);
    std::vector<std::pair<boost::uuids::uuid, QString>> participants(clients.size());
    std::transform(std::begin(clients), std::end(clients), std::begin(participants), [](const auto& part){
        return std::make_pair(part.uuid(), part.name());
    });
    for(auto& c : _clients)
    {
        c.tcpMessageChannel->postMessage(messages::participant_list_update{.participants = participants});
    }
}

void QServerLogic::onClientConnected(const std::shared_ptr<networking::QTcpAsioSocket>& socket,
                                     const boost::asio::ip::tcp::endpoint& destination)
{
    auto remoteEndpoint = destination;
    auto added_client_it = _clients.emplace(
        _clients.end(),
            std::make_unique<networking::QTcpMessageChannel>(binaryTcpSerializer, binaryTcpDeserializer),
            boost::asio::ip::udp::endpoint(),
            data::QChatParticipant(),
            false
        );
    auto& added_client = *added_client_it;

    QObject::connect(added_client.tcpMessageChannel.get(), &networking::QTcpMessageChannel::messageReceived,
                     this, [this, added_client_it](const auto& message){
            QMutexLocker l{&clients_mutex};
            onTcpMessageReceived(*added_client_it, message);
        }, Qt::QueuedConnection);

    QObject::connect(added_client.tcpMessageChannel.get(), &networking::QTcpMessageChannel::disconnected,
                     this, [this, added_client_it](const auto& reason){
            QMutexLocker l{&clients_mutex};
            onClientDisconnected(added_client_it, reason);
        }, Qt::QueuedConnection);

    added_client.tcpMessageChannel->start(std::make_unique<networking::QTcpAsioSocket>(std::move(*socket)));

    added_client.tcpMessageChannel->postMessage(messages::login_request{});
}

void QServerLogic::onClientDisconnected(std::list<client>::iterator client, const boost::system::error_code& reason)
{
    if(!blockDisconnectHandling)
    {
        bool was_logged_in = client->logged_in;
        _clients.erase(client);

        emit errorOccured(reason);
        if(was_logged_in) informClientsListChanged();
    }
}

void QServerLogic::onFailedToAccept(const boost::system::error_code& reason)
{
    emit errorOccured(reason);
    emit failedToAccept(reason);
    stop();
}

void QServerLogic::onTcpMessageReceived(client& sender,
                                        const networking::QTcpMessageChannel::messages_variant& message)
{
    std::visit([this, &sender](const auto& msg){ onMessageReceived(sender, msg); }, message);
}

void QServerLogic::onUdpMessageReceived(const boost::asio::ip::udp::endpoint& sender,
                                        const networking::QUdpMessageChannel::messages_variant& message)
{
    if(message.index() == 1) // udp_hi
    {
        auto& msg = std::get<1>(message);
        auto it = std::find_if(std::begin(_clients), std::end(_clients), [&](const auto& client){
            return msg.sender == client.chatParticipant.uuid();
        });

        if(it != std::end(_clients))
        {
            it->udpEndpoint = sender;
        }
    }
    auto it = std::find_if(std::begin(_clients), std::end(_clients), [&](const auto& client){
        return sender.address() == client.udpEndpoint.address();
    });

    if(it != std::end(_clients))
    {
        std::visit([this, it](const auto& msg){ onMessageReceived(*it, msg); }, message);
    }
}


void QServerLogic::onMessageReceived(client& sender, const messages::login_request&){}
void QServerLogic::onMessageReceived(client& sender, const messages::login_attempt& attempt)
{
    auto it = std::find_if(std::begin(_clients), std::end(_clients), [&attempt](const auto& client){
        return client.chatParticipant.name() == attempt.client_name;
    });
    if(attempt.client_name == _server.name() || it != std::end(_clients))
    {
        sender.tcpMessageChannel->postMessage(messages::login_reply{.login_result = messages::login_error::duplicate_name});
    }
    else
    {
        sender.logged_in = true;
        sender.chatParticipant.setName(attempt.client_name);
        sender.chatParticipant.setUuid(client_uuid_generator(attempt.client_name.toStdString().c_str()));
        sender.tcpMessageChannel->postMessage(messages::login_reply{.login_result = sender.chatParticipant.uuid()});
        informClientsListChanged();
    }
}
void QServerLogic::onMessageReceived(client& sender, const messages::login_reply& lreply) {}

// TODO complex message handling with images and stuff...
void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_begin&){}
void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_text_part&){}
void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_image_part&){}
void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_end&){}
void QServerLogic::onMessageReceived(client& sender, const messages::request_chat_message_part_retransmission&){}

void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_simple& msg)
{
    sendChatMessageWithSender(msg.text, sender.chatParticipant);
}
void QServerLogic::onMessageReceived(client& sender, const messages::chat_message_complex& msg)
{
    sendChatMessageWithSender(msg.long_text, sender.chatParticipant);
}
void QServerLogic::onMessageReceived(client& sender, const messages::udp_hi&){}
void QServerLogic::onMessageReceived(client& sender, const messages::request_change_name& msg)
{
    auto it = std::find_if(std::begin(_clients), std::end(_clients), [&msg](const auto& client){
        return client.chatParticipant.name() == msg.new_name;
    });
    if(msg.new_name == _server.name() || it != std::end(_clients))
    {
        sender.tcpMessageChannel->postMessage(messages::reply_change_name{.result = messages::login_error::duplicate_name});
    }
    else
    {
        sender.chatParticipant.setName(msg.new_name);
        sender.tcpMessageChannel->postMessage(messages::reply_change_name{.result = msg.new_name});
        informClientsListChanged();
    }
}
void QServerLogic::onMessageReceived(client& sender, const messages::reply_change_name& rcname){}
void QServerLogic::onMessageReceived(client& sender, const messages::participant_list_update& plupdate){}
void QServerLogic::onMessageReceived(client& sender, const messages::request_participant_list_update& req)
{
    std::vector<std::pair<boost::uuids::uuid, QString>> participants;
    std::for_each(std::begin(_clients), std::end(_clients),[&participants](const auto& c){
        if(c.logged_in) participants.push_back(std::make_pair(
            c.chatParticipant.uuid(), c.chatParticipant.name()));
    });
    participants.push_back(std::make_pair(_server.uuid(), _server.name()));
    sender.tcpMessageChannel->postMessage(messages::participant_list_update{.participants = participants});
}

LOGIC_NAMESPACE_END
SERVER_NAMESPACE_END