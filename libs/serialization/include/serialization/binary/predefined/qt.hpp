#pragma once

#include <serialization/binary/format.hpp>
#include <serialization/binary/predefined/fundamental.hpp>
#include <serialization/serialization.hpp>
#include <QtCore/QByteArray>
#include <QtCore/QBuffer>
#include <QtCore/QString>
#include <QtGui/QImage>

SERIALIZATION_NAMESPACE_BEGIN

template<>
struct serializable<QByteArray, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = QByteArray;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        uint32_t size = value.size();
        if(!::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, size)) return false;
        return warchive.insertBytes(value.data(), value.size());
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        uint32_t size;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, size)) return false;
        value.resize(size);
        return rarchive.readBytes(value.data(), value.size());
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(uint32_t(value.size())) + value.size();
    }
};

template<>
struct serializable<QString, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = QString;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        return ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, value.toLatin1());
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        QByteArray latin1;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, latin1)) return false;
        value = value_type::fromLatin1(latin1);
        return true;
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        return ::SERIALIZATION_NAMESPACE_FULL::calculate_binary_size(value.toLatin1());
    }
};

template<>
struct serializable<QImage, ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary>
{
    using format_type = ::SERIALIZATION_FORMAT_NAMESPACE_FULL::binary;
    using value_type = QImage;

    static bool serialize(writeonly_archive_t<format_type>& warchive, const value_type& value)
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if(!value.save(&buffer, "PNG")) return false;
        return ::SERIALIZATION_NAMESPACE_FULL::serialize<format_type>(warchive, data);
    }
    static bool deserialize(readonly_archive_t<format_type>& rarchive, value_type& value)
    {
        QByteArray data;
        if(!::SERIALIZATION_NAMESPACE_FULL::deserialize<format_type>(rarchive, data)) return false;
        return value.loadFromData(data, "PNG");
    }
    static size_t calculate_binary_size(const value_type& value)
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if(!value.save(&buffer, "PNG")) return 0;
        return data.size();
    }
};

SERIALIZATION_NAMESPACE_END