#pragma once

#include <serialization/includes.hpp>

SERIALIZATION_NAMESPACE_BEGIN

template<typename Format>
concept SerializationFormat =
requires{
    typename Format::writeonly_archive_type;
    typename Format::readonly_archive_type;
};

template<SerializationFormat Format>
using writeonly_archive_t = typename Format::writeonly_archive_type;

template<SerializationFormat Format>
using readonly_archive_t = typename Format::readonly_archive_type;

template<typename Type, SerializationFormat Format>
struct serializable
{
};

template<typename Type, typename Format>
concept Serializable =
SerializationFormat<Format> &&
requires(
    writeonly_archive_t<Format>& warchive,
    readonly_archive_t<Format>& rarchive,
    const Type& cvalue,
    Type& value)
{
    { serializable<Type, Format>::serialize(warchive, cvalue) } -> std::same_as<bool>;
    { serializable<Type, Format>::deserialize(rarchive, value) } -> std::same_as<bool>;
} && Format:: template AdditionalSerializableConstrains<Type>;

SERIALIZATION_NAMESPACE_END