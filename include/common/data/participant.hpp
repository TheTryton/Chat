#pragma once

#include <common/includes.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN

class QChatParticipant
{
private:
    QString _name;
    boost::uuids::uuid _uuid;
public:
    QChatParticipant() = default;
    QChatParticipant(const QChatParticipant&) = default;
    QChatParticipant(QChatParticipant&&) = default;
    QChatParticipant(boost::uuids::uuid uuid, const QString& name = QString());
public:
    QChatParticipant& operator=(const QChatParticipant&) = default;
    QChatParticipant& operator=(QChatParticipant&&) = default;
public:
    void setUuid(const boost::uuids::uuid& uuid);
    const boost::uuids::uuid& uuid() const;
    void setName(const QString& name);
    const QString& name() const;
};

using QChatParticipantList = std::vector<QChatParticipant>;

DATA_NAMESPACE_END
COMMON_NAMESPACE_END