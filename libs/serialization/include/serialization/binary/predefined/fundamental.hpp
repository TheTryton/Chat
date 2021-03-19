#pragma once

#include <serialization/binary/format.hpp>

SERIALIZATION_NAMESPACE_BEGIN

template<typename Type>
constexpr bool is_binary_fundamental_v =
    std::is_integral_v<Type>
    || std::is_floating_point_v<Type>
    || std::is_enum_v<Type>;

template<typename Type>
requires is_binary_fundamental_v<Type>
struct serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = Type;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        return warchive.insertBytes(&value, sizeof(Type));
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        return rarchive.readBytes(&value, sizeof(Type));
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        return sizeof(Type);
    }
};

SERIALIZATION_NAMESPACE_END