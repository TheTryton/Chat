#include <common/data/participant.hpp>

COMMON_NAMESPACE_BEGIN
DATA_NAMESPACE_BEGIN

QChatParticipant::QChatParticipant(boost::uuids::uuid uuid, const QString& name)
    : _uuid(uuid)
    , _name(name)
{
}

void QChatParticipant::setUuid(const boost::uuids::uuid& uuid)
{
    _uuid = uuid;
}

const boost::uuids::uuid& QChatParticipant::uuid() const
{
    return _uuid;
}

void QChatParticipant::setName(const QString& name)
{
    _name = name;
}

const QString& QChatParticipant::name() const
{
    return _name;
}

DATA_NAMESPACE_END
COMMON_NAMESPACE_END