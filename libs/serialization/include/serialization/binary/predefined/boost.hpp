#pragma once

#include <serialization/binary/format.hpp>
#include <boost/uuid/uuid.hpp>

SERIALIZATION_NAMESPACE_BEGIN

template<>
struct serializable<boost::uuids::uuid, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = boost::uuids::uuid;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        return warchive.insertBytes(value.data, value.size());
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        return rarchive.readBytes(value.data, value.size());
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        return value.size();
    }
};

SERIALIZATION_NAMESPACE_END