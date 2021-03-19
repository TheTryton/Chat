#pragma once

#include <serialization/concepts.hpp>

SERIALIZATION_NAMESPACE_BEGIN

template<SerializationFormat Format, typename Type>
requires Serializable<Type, Format>
bool serialize(writeonly_archive_t<Format>& warchive, const Type& value)
{
    return serializable<Type, Format>::serialize(warchive, value);
}

template<SerializationFormat Format, typename Type>
requires Serializable<Type, Format>
bool deserialize(readonly_archive_t<Format>& rarchive, Type& value)
{
    return serializable<Type, Format>::deserialize(rarchive, value);
}

SERIALIZATION_NAMESPACE_END