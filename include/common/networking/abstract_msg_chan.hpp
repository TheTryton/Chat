#pragma once

#include <common/data/messages/messages.hpp>
#include <common/data/messages/serialization/abstract.hpp>
#include <common/networking/sockets.hpp>
#include <common/data/messages/serialization/binary.hpp>

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

namespace messages = ::COMMON_NAMESPACE_FULL::MESSAGES_NAMESPACE_FULL;

template<messages::GuidIdentifiableMessage... MessageTypes>
class QAbstractTcpMessageChannel : public QObject
{
protected:
    const messages::QAbstractMessageSerializer<MessageTypes...>& serializer;
    const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer;
public:
    using messages_variant = std::variant<MessageTypes...>;
public:
    QAbstractTcpMessageChannel(const messages::QAbstractMessageSerializer<MessageTypes...>& serializer,
                               const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer,
                               QObject* parent = nullptr)
        : QObject(parent)
        , serializer(serializer)
        , deserializer(deserializer)
    {
    }
public:
    template<typename MessageType>
    requires (std::is_same_v<MessageType, MessageTypes> || ...)
    void postMessage(MessageType&& message)
    {
        postSerializedData(serializer.serialize(std::forward<MessageType>(message)));
    }
    void postMessage(const std::variant<MessageTypes...>& message)
    {
        postSerializedData(serializer.serialize(message));
    }
protected:
    virtual void postSerializedData(QByteArray&& serializedData) = 0;
    virtual void handleReceivedData(QByteArray&& serializedData) = 0;
    virtual void handleError(const boost::system::error_code& ec) = 0;
    virtual void handleDisconnect(const boost::system::error_code& ec) = 0;
};

template<messages::GuidIdentifiableMessage... MessageTypes>
class QAbstractUdpMessageChannel : public QObject
{
protected:
    const messages::QAbstractMessageSerializer<MessageTypes...>& serializer;
    const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer;
public:
    using messages_variant = std::variant<MessageTypes...>;
public:
    QAbstractUdpMessageChannel(const messages::QAbstractMessageSerializer<MessageTypes...>& serializer,
                               const messages::QAbstractMessageDeserializer<MessageTypes...>& deserializer,
                               QObject* parent = nullptr)
        : QObject(parent)
        , serializer(serializer)
        , deserializer(deserializer)
    {
    }
public:
    template<typename MessageType>
    requires (std::is_same_v<MessageType, MessageTypes> || ...)
    void postMessage(const boost::asio::ip::udp::endpoint& destination, MessageType&& message)
    {
        postSerializedData(destination,serializer.serialize(std::forward<MessageType>(message)));
    }
    void postMessage(const boost::asio::ip::udp::endpoint& destination, const std::variant<MessageTypes...>& message)
    {
        postSerializedData(destination,serializer.serialize(message));
    }
protected:
    virtual void postSerializedData(const boost::asio::ip::udp::endpoint& destination, QByteArray&& serializedData) = 0;
    virtual void handleReceivedData(const boost::asio::ip::udp::endpoint& sender, QByteArray&& serializedData) = 0;
    virtual void handleError(const boost::system::error_code& ec) = 0;
    virtual void handleDisconnect(const boost::system::error_code& ec) = 0;
};

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END