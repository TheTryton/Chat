#pragma once

#include <common/includes.hpp>
#include <common/data/messages/messages.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN
MESSAGES_NAMESPACE_BEGIN

template<GuidIdentifiableMessage... MessageTypes>
class QAbstractMessageSerializer
{
public:
    virtual ~QAbstractMessageSerializer() = default;
public:
    virtual QByteArray serialize(const std::variant<MessageTypes...>& message) const = 0;
};

template<GuidIdentifiableMessage... MessageTypes>
class QAbstractMessageDeserializer
{
public:
    virtual ~QAbstractMessageDeserializer() = default;
public:
    virtual std::optional<std::variant<MessageTypes...>> deserialize(const QByteArray& serializedMessage) const = 0;
};

MESSAGES_NAMESPACE_END
DATA_NAMESPACE_END
COMMON_NAMESPACE_END