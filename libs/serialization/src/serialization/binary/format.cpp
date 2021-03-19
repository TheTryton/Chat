#include <serialization/binary/format.hpp>

SERIALIZATION_FORMAT_NAMESPACE_BEGIN

writeonly_binary_archive::writeonly_binary_archive(void* begin, void* end) noexcept
    : _current(reinterpret_cast<::std::byte*>(begin))
    , _end(reinterpret_cast<::std::byte*>(end))
{
}

writeonly_binary_archive::writeonly_binary_archive(void* data, size_t space) noexcept
    : _current(reinterpret_cast<::std::byte*>(data))
    , _end(reinterpret_cast<::std::byte*>(data) + space)
{
}

bool writeonly_binary_archive::insertBytes(const void* data, size_t size)
{
    if constexpr(!binary_archive_assume_enough_space)
    {
        if(_current + size > _end)
        {
            return false;
        }
    }
    std::memcpy(_current, data, size);
    _current += size;
    return true;
}

readonly_binary_archive::readonly_binary_archive(const void* begin, const void* end) noexcept
    : _current(reinterpret_cast<const ::std::byte*>(begin))
    , _end(reinterpret_cast<const ::std::byte*>(end))
{

}

readonly_binary_archive::readonly_binary_archive(const void* data, size_t size) noexcept
    : _current(reinterpret_cast<const ::std::byte*>(data))
    , _end(reinterpret_cast<const ::std::byte*>(data) + size)
{
}

bool readonly_binary_archive::readBytes(void* data, size_t size)
{
    if constexpr(!binary_archive_assume_enough_space)
    {
        if(_current + size > _end)
        {
            return false;
        }
    }
    std::memcpy(data, _current, size);
    _current += size;
    return true;
}

SERIALIZATION_FORMAT_NAMESPACE_END