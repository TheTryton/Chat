#pragma once

#include <serialization/serialization.hpp>
#include <serialization/binary/predefined/fundamental.hpp>
#include <serialization/binary/predefined/detail/algorithm.hpp>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <utility>
#include <tuple>
#include <variant>
#include <algorithm>
#include <numeric>

SERIALIZATION_NAMESPACE_BEGIN

template<typename Type, typename Traits, typename Allocator>
requires Serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
struct serializable<std::basic_string<Type, Traits, Allocator>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::basic_string<Type, Traits, Allocator>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        std::uint64_t size = value.size();
        if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, size)) return false;
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return warchive.insertBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(const auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        std::uint64_t size;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, size)) return false;
        value.resize(size);
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return rarchive.readBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        std::uint64_t size;
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.size()) + value.size() * sizeof(Type);
        }
        else
        {
            return std::accumulate(std::begin(value), std::end(value), size_t(0), [](const auto& l, const auto& r)
            {
                return l + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(r);
            }) + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(size);
        }
    }
};

template<typename Type, typename Allocator>
requires Serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
struct serializable<std::vector<Type, Allocator>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::vector<Type, Allocator>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        std::uint64_t size = value.size();
        if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, size)) return false;
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return warchive.insertBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(const auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        std::uint64_t size;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, size)) return false;
        value.resize(size);
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return rarchive.readBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        std::uint64_t size;
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.size()) + value.size() * sizeof(Type);
        }
        else
        {
            return std::accumulate(std::begin(value), std::end(value), size_t(0), [](const auto& l, const auto& r)
            {
                return l + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(r);
            }) + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(size);
        }
    }
};

template<typename Type, size_t Size>
requires Serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
struct serializable<std::array<Type, Size>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::array<Type, Size>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return warchive.insertBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(const auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return rarchive.readBytes(value.data(), value.size() * sizeof(Type));
        }
        else
        {
            for(auto& e: value)
            {
                if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, e))
                {
                    return false;
                }
            }
            return true;
        }
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        if constexpr (is_binary_fundamental_v<Type>)
        {
            return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.size()) + value.size() * sizeof(Type);
        }
        else
        {
            return std::accumulate(std::begin(value), std::end(value), size_t(0), [](const auto& l, const auto& r)
            {
                return l + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(r);
            });
        }
    }
};

template<typename Type0, typename Type1>
requires
Serializable<Type0, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
&& Serializable<Type1, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
struct serializable<std::pair<Type0, Type1>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::pair<Type0, Type1>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, value.first)) return false;
        return ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, value.second);
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, value.first)) return false;
        return ::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, value.second);
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.first)
        + ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.second);
    }
};

template<typename... Types>
requires (Serializable<Types, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary> && ...)
struct serializable<std::tuple<Types...>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::tuple<Types...>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        bool any = true;
        detail::for_each(value, [&any, &warchive](const auto& e){
            any &= ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, e);
        });
        return any;
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        bool any = true;
        detail::for_each(value, [&any, &rarchive](auto& e){
            any &= ::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, e);
        });
        return any;
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        size_t sum = 0;
        detail::for_each(value, [&sum](auto& e){
            sum += ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(e);
        });
        return sum;
    }
};

template<typename... Types>
requires (
    (Serializable<Types, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary> && ...)
    && Serializable<std::uint32_t, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>)
struct serializable<std::variant<Types...>, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = std::variant<Types...>;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        std::uint32_t index = value.index();
        if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, index)) return false;
        bool ok = true;
        std::visit([&ok, &warchive](const auto& e) { ok = ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, e); }, value);
        return ok;
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        std::uint32_t index;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, index)) return false;
        return deserialize_variant(rarchive, value, index, std::make_index_sequence<sizeof...(Types)>());
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        std::uint32_t index;
        size_t size = ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(index);
        std::visit([&size](const auto& e) { size += ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(e); }, value);
        return size;
    }
private:
    template<size_t... I>
    constexpr static bool deserialize_variant(
        readonly_archive_t<format_type>& rarchive,
        std::variant<Types...>& value,
        size_t index,
        std::index_sequence<I...>)
    {
        using deserializer_type = bool(*)(
            readonly_archive_t<format_type>&,
            std::variant<Types...>&);
        constexpr deserializer_type deserializers[] = {
            [](
                readonly_archive_t<format_type>& rarchive,
                std::variant<Types...>& value){
                using i_t = std::tuple_element_t<I, std::tuple<Types...>>;
                i_t ith_e;
                if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, ith_e)) return false;
                value = std::variant<Types...>(std::in_place_index_t<I>{},std::move(ith_e));
                return true;
            }...
        };
        if(index >= sizeof...(Types)) return false;
        return deserializers[index](rarchive, value);
    }
};

SERIALIZATION_NAMESPACE_END