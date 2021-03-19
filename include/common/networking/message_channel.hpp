#pragma once

#include <common/networking/abstract_msg_chan.hpp>

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

template<messages::GuidIdentifiableMessage... MessageTypes>
class QTcpAsioSocketMessageChannel : public QAbstractTcpMessageChannel<MessageTypes...>
{
private:
    using message_size_type = uint32_t;
private:
    std::unique_ptr<QTcpAsioSocket> socket;
    QQueue<QByteArray> messageQueue;
    QRecursiveMutex messageQueueMutex;
    QByteArray currentReceivedMessageSize;
    QByteArray currentReceivedMessageData;
    QByteArray currentSentMessageData;
    bool started = false;
    bool sendingActive = false;
    std::shared_ptr<bool> stopped = std::make_shared<bool>(false);
public:
    QTcpAsioSocketMessageChannel(const messages::QAbstractMessageSerializer<MessageTypes...>& serializer,
                              const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer,
                              QObject* parent = nullptr)
    : QAbstractTcpMessageChannel<MessageTypes...>(serializer, deserializer, parent)
    {
    }
public:
    virtual ~QTcpAsioSocketMessageChannel() override
    {
        *stopped = true;
    }
public:
    void start(std::unique_ptr<QTcpAsioSocket> socket)
    {
        stop();
        if(!started)
        {
            this->socket = std::move(socket);
            started = true;
            startReceivingMessages();
        }
    }
    void stop()
    {
        socket.reset();
        QMutexLocker lock{&messageQueueMutex};
        messageQueue.clear();
        started = false;
        sendingActive = false;
    }
protected:
    virtual void postSerializedData(QByteArray&& serializedData) override
    {
        enqueueSerializedMessage(std::move(serializedData));
    }
    void handleReadError(const boost::system::error_code& ec)
    {
        if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
        {
            started = false;
            this->handleDisconnect(ec);
        }
        this->handleError(ec);
    }
private:
    void enqueueSerializedMessage(QByteArray&& messageData)
    {
        QMutexLocker lock{&messageQueueMutex};
        messageQueue.enqueue(messageData);
        notifyMessagesAvailable();
    }
    void notifyMessagesAvailable()
    {
        QMutexLocker lock{&messageQueueMutex};
        if(!sendingActive) startSendingMessages();
    }
    void startSendingMessages()
    {
        checkQueueAndSendMessage();
    }
    void checkQueueAndSendMessage()
    {
        QMutexLocker lock{&messageQueueMutex};
        if(!messageQueue.empty())
        {
            sendingActive = true;
            sendMessage(messageQueue.dequeue(), [this](){
                checkQueueAndSendMessage();
            });
        }
        else
        {
            sendingActive = false;
        }
    }
    template<typename MessageSentHandler>
    void sendMessage(QByteArray&& messageData, MessageSentHandler&& handler)
    {
        message_size_type message_size = messageData.size();
        currentSentMessageData.resize(
            ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(message_size) + messageData.size()
            );
        auto warchive =
            ::SERIALIZATION_NAMESPACE_FULL::writeonly_archive_t<
                ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary
                >(currentSentMessageData.data(), currentSentMessageData.size());
        ::SERIALIZATION_NAMESPACE_FULL::serialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
            warchive,
            message_size);
        warchive.insertBytes(messageData.data(), messageData.size());
        socket->asyncWrite(
            boost::asio::buffer(currentSentMessageData.data(), currentSentMessageData.size()),
            [this, stop = this->stopped, handler = std::forward<MessageSentHandler>(handler)]
            (boost::system::error_code ec, std::size_t) mutable
            {
                if(*stop)
                    return;
                if(ec)
                {
                    this->handleError(ec);
                }
                else
                {
                    std::forward<MessageSentHandler>(handler)();
                }
            });
    }

    void startReceivingMessages()
    {
        receiveMessage();
    }
    void receiveMessage()
    {
        currentReceivedMessageSize.resize(sizeof(message_size_type));
        socket->asyncRead(
            boost::asio::buffer(currentReceivedMessageSize.data(), currentReceivedMessageSize.size()),
            [this, stop = this->stopped](boost::system::error_code ec, std::size_t)
            {
                if(*stop)
                    return;
                if(ec)
                {
                    this->handleReadError(ec);
                }
                else
                {
                    message_size_type message_size;
                    auto rarchive =
                        ::SERIALIZATION_NAMESPACE_FULL::readonly_archive_t<
                            ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary
                        >(currentReceivedMessageSize.data(), currentReceivedMessageSize.size());
                    ::SERIALIZATION_NAMESPACE_FULL::deserialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
                        rarchive,
                        message_size);
                    currentReceivedMessageData.resize(message_size);
                    socket->asyncRead(
                        boost::asio::buffer(currentReceivedMessageData.data(), currentReceivedMessageData.size()),
                        [this, stop](boost::system::error_code ec, std::size_t){
                            if(*stop)
                                return;
                            if(ec)
                            {
                                this->handleReadError(ec);
                            }
                            else
                            {
                                this->handleReceivedData(std::move(currentReceivedMessageData));
                                receiveMessage();
                            }
                        });
                }
            });
    }
};

template<messages::GuidIdentifiableMessage... MessageTypes>
class QUdpAsioSocketMessageChannel : public QAbstractUdpMessageChannel<MessageTypes...>
{
private:
    using message_size_type = uint32_t;
private:
    std::unique_ptr<QUdpAsioSocket> socket;
    QQueue<std::pair<boost::asio::ip::udp::endpoint, QByteArray>> messageQueue;
    QRecursiveMutex messageQueueMutex;
    QByteArray currentReceivedMessageData;
    QByteArray currentSentMessageData;
    bool started = false;
    bool sendingActive = false;
    std::shared_ptr<bool> stopped = std::make_shared<bool>(false);
public:
    QUdpAsioSocketMessageChannel(const messages::QAbstractMessageSerializer<MessageTypes...>& serializer,
                             const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer,
                             QObject* parent = nullptr)
        : QAbstractUdpMessageChannel<MessageTypes...>(serializer, deserializer, parent)
    {
    }
public:
    virtual ~QUdpAsioSocketMessageChannel() override
    {
        *stopped = true;
    }
public:
    void start(std::unique_ptr<QUdpAsioSocket> socket)
    {
        stop();
        if(!started)
        {
            this->socket = std::move(socket);
            started = true;
            startReceivingMessages();
        }
    }
    void stop()
    {
        socket.reset();
        QMutexLocker lock{&messageQueueMutex};
        messageQueue.clear();
        started = false;
        sendingActive = false;
    }
protected:
    virtual void postSerializedData(const boost::asio::ip::udp::endpoint& destination, QByteArray&& serializedData) override
    {
        enqueueSerializedMessage(destination, std::move(serializedData));
    }
    void handleReadError(const boost::system::error_code& ec)
    {
        if(ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
        {
            started = false;
            this->handleDisconnect(ec);
        }
        this->handleError(ec);
    }
private:
    void enqueueSerializedMessage(const boost::asio::ip::udp::endpoint& destination, QByteArray&& messageData)
    {
        QMutexLocker lock{&messageQueueMutex};
        messageQueue.enqueue(std::make_pair(destination, std::move(messageData)));
        notifyMessagesAvailable();
    }
    void notifyMessagesAvailable()
    {
        QMutexLocker lock{&messageQueueMutex};
        if(!sendingActive) startSendingMessages();
    }
    void startSendingMessages()
    {
        checkQueueAndSendMessage();
    }
    void checkQueueAndSendMessage()
    {
        QMutexLocker lock{&messageQueueMutex};
        if(!messageQueue.empty())
        {
            sendingActive = true;
            sendMessage(messageQueue.dequeue(), [this](){
                checkQueueAndSendMessage();
            });
        }
        else
        {
            sendingActive = false;
        }
    }
    template<typename MessageSentHandler>
    void sendMessage(std::pair<boost::asio::ip::udp::endpoint, QByteArray>&& messageData, MessageSentHandler&& handler)
    {
        message_size_type message_size = messageData.second.size();
        currentSentMessageData.resize(
            ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(message_size) + messageData.second.size()
        );
        auto warchive =
            ::SERIALIZATION_NAMESPACE_FULL::writeonly_archive_t<
                ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary
            >(currentSentMessageData.data(), currentSentMessageData.size());

        ::SERIALIZATION_NAMESPACE_FULL::serialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
            warchive,
            message_size);
        warchive.insertBytes(messageData.second.data(), messageData.second.size());
        socket->asyncWrite(
            boost::asio::buffer(currentSentMessageData.data(), currentSentMessageData.size()),
            messageData.first,
            [this, stop = this->stopped, handler = std::forward<MessageSentHandler>(handler)]
                (boost::system::error_code ec, std::size_t) mutable
            {
                if(*stop)
                    return;
                if(ec)
                {
                    this->handleError(ec);
                }
                else
                {
                    std::forward<MessageSentHandler>(handler)();
                }
            });
    }

    void startReceivingMessages()
    {
        receiveMessage();
    }
    void receiveMessage()
    {
        currentReceivedMessageData.resize(10000);
        socket->asyncRead(
            boost::asio::buffer(currentReceivedMessageData.data(), currentReceivedMessageData.size()),
            [this, stop = this->stopped](boost::system::error_code ec, const boost::asio::ip::udp::endpoint& sender, std::size_t bt)
            {
                if(*stop)
                    return;
                if(ec)
                {
                    this->handleError(ec);
                }
                else
                {
                    currentReceivedMessageData.resize(bt);

                    message_size_type message_size;
                    auto offset_for_size = ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(message_size);
                    if(currentReceivedMessageData.size() < offset_for_size)
                    {
                        this->handleReadError(boost::asio::error::eof);
                    }
                    else
                    {
                        auto rarchive =
                            ::SERIALIZATION_NAMESPACE_FULL::readonly_archive_t<
                                ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary
                            >(currentReceivedMessageData.data(), currentReceivedMessageData.size());

                        ::SERIALIZATION_NAMESPACE_FULL::deserialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
                            rarchive,
                            message_size);

                        if(currentReceivedMessageData.size() < offset_for_size + message_size)
                        {
                            this->handleReadError(boost::asio::error::eof);
                        }
                        else
                        {
                            QByteArray message;
                            message.resize(message_size);
                            std::copy(
                                std::begin(currentReceivedMessageData) + offset_for_size,
                                std::end(currentReceivedMessageData),
                                std::begin(message));
                            this->handleReceivedData(sender, std::move(message));
                            receiveMessage();
                        }
                    }
                }
            });
    }
};

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END