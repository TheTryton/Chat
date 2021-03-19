#include <client/logic/client.hpp>
#include <common/data/messages/serialization/binary.hpp>

CLIENT_NAMESPACE_BEGIN
LOGIC_NAMESPACE_BEGIN

networking::QTcpMessageChannelSerializer QClientLogic::binaryTcpSerializer{};
networking::QTcpMessageChannelDeserializer QClientLogic::binaryTcpDeserializer{};
networking::QUdpMessageChannelSerializer QClientLogic::binaryUdpSerializer{};
networking::QUdpMessageChannelDeserializer QClientLogic::binaryUdpDeserializer{};

QClientLogic::QClientLogic(boost::asio::io_context& context, QObject* parent)
    : QObject(parent)
    , _work_guard(context.get_executor())
    , _context(context)
    , tcpMessageChannel(binaryTcpSerializer, binaryTcpDeserializer)
    , udpMessageChannel(binaryUdpSerializer, binaryUdpDeserializer)
    , udpMulticastMessageChannel(binaryUdpSerializer, binaryUdpDeserializer)
{
    tcpResolver.setContext(context);
    tcpConnector.setContext(context);
    //udpResolver.setContext(context);

    //QObject::connect(&udpResolver, &networking::QUdpResolver::resolved, this, &QClientLogic::onResolvedUdp, Qt::QueuedConnection);

    QObject::connect(&tcpResolver, &networking::QTcpResolver::resolved, this, &QClientLogic::onResolved, Qt::QueuedConnection);
    QObject::connect(&tcpResolver, &networking::QTcpResolver::resolutionFailed, this, &QClientLogic::onFailedToResolve, Qt::QueuedConnection);

    QObject::connect(&tcpConnector, &networking::QTcpConnector::connectionEstablished, this, &QClientLogic::onConnected, Qt::QueuedConnection);
    QObject::connect(&tcpConnector, &networking::QTcpConnector::failedToEstablishConnection, this, &QClientLogic::onFailedToConnect, Qt::QueuedConnection);

    QObject::connect(&tcpMessageChannel, &networking::QTcpMessageChannel::messageReceived, this, &QClientLogic::onTcpMessageReceived, Qt::QueuedConnection);
    QObject::connect(&tcpMessageChannel, &networking::QTcpMessageChannel::disconnected, this, &QClientLogic::onConnectionLost, Qt::QueuedConnection);

    QObject::connect(&udpMessageChannel, &networking::QUdpMessageChannel::messageReceived, this, &QClientLogic::onUdpMessageReceived, Qt::QueuedConnection);
    QObject::connect(&udpMulticastMessageChannel, &networking::QUdpMessageChannel::messageReceived, this, &QClientLogic::onUdpMessageReceived, Qt::QueuedConnection);
}

const QClientLogic::State& QClientLogic::currentState() const
{
    return _currentState;
}

void QClientLogic::setCurrentState(const State& newState)
{
    if(_currentState != newState)
    {
        _currentState = newState;
        emit currentStateChanged(_currentState);
    }
}

void QClientLogic::resolveAndConnect(const QString& address)
{
    auto parts = address.split(":",Qt::KeepEmptyParts);
    if(parts.size() != 2)
    {
        onFailedToResolve(boost::asio::error::invalid_argument);
    }
    else
    {
        std::string address = parts.front().toStdString();
        std::string service = parts.back().toStdString();
        tcpResolver.startResolving(address, service);
        boost::asio::ip::udp::resolver resolver{_context};
        auto results = resolver.resolve(address, service);
        if(!results.empty()) _server_endpoint = *results.begin();
        boost::asio::ip::udp::socket udp_socket(
            _context,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
        udpMessageChannel.start(std::make_unique<networking::QUdpAsioSocket>(std::move(udp_socket)));
        setCurrentState(State::resolving);
    }
}

void QClientLogic::onResolved(const boost::asio::ip::tcp::resolver::results_type& results)
{
    tcpConnector.startConnection(*results.begin());
    setCurrentState(State::connecting);
}

void QClientLogic::onFailedToResolve(const boost::system::error_code& reason)
{
    emit errorOccured(reason);
    emit failedToConnect(reason);
    setCurrentState(State::disconnected);
}

void QClientLogic::onConnected(const std::shared_ptr<networking::QTcpAsioSocket>& socket,
                               const boost::asio::ip::tcp::endpoint& destination)
{
    _server_endpoint = boost::asio::ip::udp::endpoint(socket->socket().remote_endpoint().address(),
                                                      socket->socket().remote_endpoint().port());
    tcpMessageChannel.start(std::make_unique<networking::QTcpAsioSocket>(std::move(*socket)));

    //boost::asio::ip::udp::socket multicast_socket{_context};

    /*auto loc = boost::asio::ip::address_v4::from_string("0.0.0.0");
    auto mcast = boost::asio::ip::address_v4::from_string(networking::multicast_address);
    auto mcastEndpoint = boost::asio::ip::udp::endpoint{loc, networking::multicast_port};
    multicast_socket.open(mcastEndpoint.protocol());
    multicast_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    multicast_socket.bind(mcastEndpoint);
    multicast_socket.set_option(boost::asio::ip::multicast::join_group(mcast, loc));
    udpMulticastMessageChannel.start(std::make_unique<networking::QUdpAsioSocket>(std::move(multicast_socket)));*/
}

void QClientLogic::onFailedToConnect(const boost::system::error_code& reason)
{
    emit errorOccured(reason);
    emit failedToConnect(reason);
    setCurrentState(State::disconnected);
}

void QClientLogic::logIn(const QString& name)
{
    _client.setName(name);
    tcpMessageChannel.postMessage(messages::login_attempt{.client_name = name});
    setCurrentState(State::logging_in);
}

void QClientLogic::changeName(const QString& newName)
{
    tcpMessageChannel.postMessage(
        messages::request_change_name{.sender = _client.uuid(), .new_name = newName});
}

void QClientLogic::onFailedToLogIn(const messages::login_error& error)
{
    setCurrentState(State::connected);
    emit failedToLogIn(error);
}

void QClientLogic::onLoggedIn(const boost::uuids::uuid& uuid)
{
    _client.setUuid(uuid);
    emit clientUuidChanged(uuid);
    emit clientNameChanged(_client.name());
    udpMessageChannel.postMessage(_server_endpoint, messages::udp_hi{.sender = uuid});
    setCurrentState(State::logged_in);
}

void QClientLogic::onTcpMessageReceived(const networking::QTcpMessageChannel::messages_variant& message)
{
    std::visit([this](const auto& msg){ onMessageReceived(msg); }, message);
}

void QClientLogic::onUdpMessageReceived(const boost::asio::ip::udp::endpoint& sender,
                                        const networking::QUdpMessageChannel::messages_variant& message)
{
    // pretty much assumes without validation
    std::visit([this](const auto& msg){ onMessageReceived(msg); }, message);
}

void QClientLogic::onConnectionLost(const boost::system::error_code& reason)
{
    setCurrentState(State::disconnected);
    emit connectionLost(reason);
}

void QClientLogic::disconnect()
{
    tcpMessageChannel.stop();
    udpMessageChannel.stop();
    udpMulticastMessageChannel.stop();
    onConnectionLost(boost::asio::error::connection_reset);
}

void QClientLogic::requestParticipantListUpdate()
{
    if(!waitingForParticipantsUpdate)
    {
        tcpMessageChannel.postMessage(messages::request_participant_list_update{});
        waitingForParticipantsUpdate = true;
    }
}

void QClientLogic::sendChatMessage(const QString& chatMessage)
{
    if(chatMessage.startsWith("-U"))
    {
        sendUdpComplexMessage(chatMessage);
    }
    else if(chatMessage.startsWith("-M"))
    {
        sendMulticastUdpComplexMessage(chatMessage);
    }
    else
    {
        sendSimpleMessage(chatMessage);
    }
}

void QClientLogic::sendSimpleMessage(const QString& chatMessage)
{
    tcpMessageChannel.postMessage(messages::chat_message_simple{.sender = _client.uuid(), .text = chatMessage});
}

void QClientLogic::sendUdpComplexMessage(const QString& chatMessage)
{
    udpMessageChannel.postMessage(_server_endpoint, messages::chat_message_complex{.sender = _client.uuid(), .long_text = chatMessage});
}

void QClientLogic::sendMulticastUdpComplexMessage(const QString& chatMessage)
{
    // regular udp message channel is used for posting messages, multicast is only for receiving
    udpMessageChannel.postMessage(_server_endpoint, messages::chat_message_complex{.sender = _client.uuid(), .long_text = chatMessage});
}

std::optional<data::QChatParticipant> QClientLogic::findClient(const boost::uuids::uuid& uuid) const
{
    auto it = std::find_if(std::begin(_clients), std::end(_clients), [&uuid](const auto& client){
        return client.uuid() == uuid;
    });
    if(it != std::end(_clients)) return *it;
    return std::nullopt;
}

void QClientLogic::quit()
{
    disconnect();
    QApplication::quit();
}

// static polymorphism for handling incoming messages

void QClientLogic::onMessageReceived(const messages::login_request&)
{
    setCurrentState(State::connected);
}
void QClientLogic::onMessageReceived(const messages::login_attempt&){}
void QClientLogic::onMessageReceived(const messages::login_reply& lreply)
{
    switch(lreply.login_result.index())
    {
        case 0:
        {
            onLoggedIn(std::get<0>(lreply.login_result));
        }
            break;
        case 1:
        {
            onFailedToLogIn(std::get<1>(lreply.login_result));
        }
            break;
    }
}

// TODO complex message handling with images and stuff...
void QClientLogic::onMessageReceived(const messages::chat_message_begin&){}
void QClientLogic::onMessageReceived(const messages::chat_message_text_part&){}
void QClientLogic::onMessageReceived(const messages::chat_message_image_part&){}
void QClientLogic::onMessageReceived(const messages::chat_message_end&){}
void QClientLogic::onMessageReceived(const messages::request_chat_message_part_retransmission&){}

void QClientLogic::onMessageReceived(const messages::chat_message_simple& msg)
{
    if(msg.sender == _client.uuid())
    {
        emit chatMessageReceived(_client, msg.text);
    }
    else if(auto sender = findClient(msg.sender))
    {
        emit chatMessageReceived(*sender, msg.text);
    }
    else
    {
        _unidentified_messages.enqueue(std::make_pair(msg.sender, msg.text));
        requestParticipantListUpdate();
    }
}
void QClientLogic::onMessageReceived(const messages::chat_message_complex& msg)
{
    if(msg.sender == _client.uuid())
    {
        emit chatMessageReceived(_client, msg.long_text);
    }
    else if(auto sender = findClient(msg.sender))
    {
        emit chatMessageReceived(*sender, msg.long_text);
    }
    else
    {
        _unidentified_messages.enqueue(std::make_pair(msg.sender, msg.long_text));
        requestParticipantListUpdate();
    }
}
void QClientLogic::onMessageReceived(const messages::udp_hi&){}
void QClientLogic::onMessageReceived(const messages::request_change_name&){}
void QClientLogic::onMessageReceived(const messages::reply_change_name& rcname)
{
    switch(rcname.result.index())
    {
        case 0:
        {
            _client.setName(std::get<0>(rcname.result));
            emit clientNameChanged(_client.name());
        } break;
        case 1:
        {
            emit failedToChangeClientName(std::get<1>(rcname.result));
        } break;
    }
}
void QClientLogic::onMessageReceived(const messages::participant_list_update& plupdate)
{
    waitingForParticipantsUpdate = false;
    _clients.clear();
    _clients.resize(plupdate.participants.size());
    std::transform(
        std::begin(plupdate.participants),
        std::end(plupdate.participants),
        std::begin(_clients),
        [](const auto& uuid_name_pair){
            return data::QChatParticipant(uuid_name_pair.first, uuid_name_pair.second);
        }
        );
    emit clientsListChanged(_clients);
    while(!_unidentified_messages.empty())
    {
        auto msg = _unidentified_messages.dequeue();
        if(auto sender = findClient(msg.first))
        {
            emit chatMessageReceived(*sender, msg.second);
        }
    }
}
void QClientLogic::onMessageReceived(const messages::request_participant_list_update&){}

LOGIC_NAMESPACE_END
CLIENT_NAMESPACE_END