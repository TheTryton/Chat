#pragma once

#include <serialization/concepts.hpp>
#include <cstddef>

SERIALIZATION_FORMAT_NAMESPACE_BEGIN

#ifdef BINARY_ARCHIVE_ASSUME_ENOUGH_SPACE
constexpr bool binary_archive_assume_enough_space = true;
#else
constexpr bool binary_archive_assume_enough_space = false;
#endif

class writeonly_binary_archive
{
private:
    ::std::byte* _current;
    ::std::byte* _end;
public:
    constexpr writeonly_binary_archive() = delete;
    writeonly_binary_archive(void* begin, void* end) noexcept;
    writeonly_binary_archive(void* data, size_t space) noexcept;
    constexpr writeonly_binary_archive(const writeonly_binary_archive&) = delete;
    constexpr writeonly_binary_archive(writeonly_binary_archive&&) noexcept = default;
public:
    writeonly_binary_archive& operator=(const writeonly_binary_archive&) = delete;
    writeonly_binary_archive& operator=(writeonly_binary_archive&&) noexcept = default;
public:
    bool insertBytes(const void* data, size_t size);
};

class readonly_binary_archive
{
private:
    const ::std::byte* _current;
    const ::std::byte* _end;
public:
    constexpr readonly_binary_archive() = delete;
    readonly_binary_archive(const void* begin, const void* end) noexcept;
    readonly_binary_archive(const void* data, size_t size) noexcept;
    constexpr readonly_binary_archive(const readonly_binary_archive&) = delete;
    constexpr readonly_binary_archive(readonly_binary_archive&&) noexcept = default;
public:
    readonly_binary_archive& operator=(const readonly_binary_archive&) = delete;
    readonly_binary_archive& operator=(readonly_binary_archive&&) noexcept = default;
public:
    bool readBytes(void* data, size_t size);
};

struct binary
{
    template<typename Type>
    constexpr static bool AdditionalSerializableConstrains = requires (const Type& cvalue)
    {
        { serializable<Type, binary>::calculate_binary_size(cvalue) } -> std::same_as<size_t>;
    };
    using writeonly_archive_type = writeonly_binary_archive;
    using readonly_archive_type = readonly_binary_archive;
};

SERIALIZATION_FORMAT_NAMESPACE_END

SERIALIZATION_NAMESPACE_BEGIN

template<typename Type>
requires Serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
size_t calculate_binary_size(const Type& value)
{
    return serializable<Type, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>::calculate_binary_size(value);
}

SERIALIZATION_NAMESPACE_END