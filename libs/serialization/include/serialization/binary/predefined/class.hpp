#pragma once

#include <serialization/binary/format.hpp>
#include <serialization/binary/predefined/fundamental.hpp>
#include <serialization/serialization.hpp>
#include <serialization/binary/predefined/detail/algorithm.hpp>

SERIALIZATION_NAMESPACE_BEGIN

template<typename Type, typename Format>
struct serializable_class {};

template<typename Type, typename Format>
concept SerializableClass = requires(Type t) {
    serializable_class<Type, Format>::serializable_fields;
};


namespace detail
{
template<typename Format, typename Class, typename... Types>
requires(Serializable<Types, Format> && ...)
auto serializability_checker(Types
Class::*... ptrs
)
{
}

template<typename Format>
auto serializability_checker()
{
}
}

#define MAKE_CLASS_SERIALIZABLE(Class, Format, ...)                                       \
template<>                                                                          \
struct serializable_class<Class, Format>                                            \
{                                                                                   \
    constexpr static auto serializable_fields = std::make_tuple(__VA_ARGS__);       \
private:                                                                            \
    using dummy_t = decltype(detail::serializability_checker<Format>(__VA_ARGS__)); \
};

template<typename Class>
requires SerializableClass<Class, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
struct serializable<Class, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = Class;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        bool any = true;
        detail::for_each(
            serializable_class<Class, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>::serializable_fields,
            [&any, &warchive, &value](const auto& e)
        {
            any &= ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, value.*e);
        });
        return any;
    }

    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        bool any = true;
        detail::for_each(
            serializable_class<Class, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>::serializable_fields,
            [&any, &rarchive, &value](const auto& e)
            {
                any &= ::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, value.*e);
            });
        return any;
    }

    static size_t calculate_binary_size(const value_type& value)
    {
        size_t sum = 0;
        detail::for_each(
            serializable_class<Class, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>::serializable_fields,
            [&sum, &value](const auto& e)
            {
                sum += ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.*e);
            });
        return sum;
    }

};

SERIALIZATION_NAMESPACE_END