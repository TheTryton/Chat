#pragma once

#include <common/includes.hpp>
#include <common/data/messages/serialization/abstract.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN
MESSAGES_NAMESPACE_BEGIN

template<GuidIdentifiableMessage... MessageTypes>
class QBinaryMessageSerializer final : public QAbstractMessageSerializer<MessageTypes...>
{
public:
    virtual ~QBinaryMessageSerializer() override = default;
public:
    virtual QByteArray serialize(const std::variant<MessageTypes...>& message) const override
    {
        return std::visit([](const auto& msg) -> QByteArray{
            using MessageType = std::remove_cvref_t<decltype(msg)>;
            auto message_id = get_message_guid<MessageType>();
            auto message_id_size = ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(message_id);
            auto message_size = ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(msg);
            QByteArray serializedData(message_id_size + message_size, 0);
            auto warchive =
                ::SERIALIZATION_NAMESPACE_FULL::writeonly_archive_t<
                    ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary
                >(serializedData.data(), serializedData.size());
            ::SERIALIZATION_NAMESPACE_FULL::serialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
                warchive,
                message_id);
            ::SERIALIZATION_NAMESPACE_FULL::serialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
                warchive,
                msg);
            return serializedData;
        }, message);
    }
};

template<GuidIdentifiableMessage... MessageTypes>
class QBinaryMessageDeserializer final : public QAbstractMessageDeserializer<MessageTypes...>
{
private:
    template<GuidIdentifiableMessage MessageType>
    static std::variant<MessageTypes...> deserialize(
        ::SERIALIZATION_NAMESPACE_FULL::readonly_archive_t<
            ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>& rarchive)
    {
        MessageType msg{};
        ::SERIALIZATION_NAMESPACE_FULL::deserialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
            rarchive,
            msg);
        return msg;
    }
    using single_deserializer_type =
    std::variant<MessageTypes...>(*)(::SERIALIZATION_NAMESPACE_FULL::readonly_archive_t<
        ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>& rarchive);
    static const std::map<boost::uuids::uuid, single_deserializer_type>& get_deserializers()
    {
        static const std::map<boost::uuids::uuid, single_deserializer_type> deserializers =
        {
            {get_message_guid<MessageTypes>(), QBinaryMessageDeserializer<MessageTypes...>::deserialize<MessageTypes>}...
        };
        return deserializers;
    }
public:
    virtual ~QBinaryMessageDeserializer() = default;
public:
    virtual std::optional<std::variant<MessageTypes...>> deserialize(const QByteArray& serializedMessage) const override
    {
        boost::uuids::uuid message_id{};
        auto rarchive = ::SERIALIZATION_NAMESPACE_FULL::readonly_archive_t<
            ::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(serializedMessage.data(), serializedMessage.size());
        ::SERIALIZATION_NAMESPACE_FULL::deserialize<::SERIALIZATION_NAMESPACE_FULL::FORMAT_NAMESPACE::binary>(
            rarchive,
            message_id
        );
        auto deserializer_for_message = get_deserializers().find(message_id);
        if(deserializer_for_message != get_deserializers().end())
        {
            return deserializer_for_message->second(rarchive);
        }
        return std::nullopt;
    }
};

MESSAGES_NAMESPACE_END
DATA_NAMESPACE_END
COMMON_NAMESPACE_END