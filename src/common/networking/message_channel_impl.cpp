#include <common/networking/message_channel_impl.hpp>


COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

void QTcpMessageChannel::handleReceivedData(QByteArray&& serializedData) {
    if(auto deserializedMessage = deserializer.deserialize(serializedData))
    {
        emit messageReceived(*deserializedMessage);
    }
    else
    {
        handleError(boost::system::errc::make_error_code(boost::system::errc::bad_message));
    }
}

void QTcpMessageChannel::handleError(const boost::system::error_code& ec) {
    emit errorOccured(ec);
}

void QTcpMessageChannel::handleDisconnect(const boost::system::error_code& ec) {
    emit disconnected(ec);
}

void QUdpMessageChannel::handleReceivedData(const boost::asio::ip::udp::endpoint& sender,
                                                        QByteArray&& serializedData) {
    if(auto deserializedMessage = deserializer.deserialize(serializedData))
    {
        emit messageReceived(sender, *deserializedMessage);
    }
    else
    {
        handleError(boost::system::errc::make_error_code(boost::system::errc::bad_message));
    }
}

void QUdpMessageChannel::handleError(const boost::system::error_code& ec) {
    emit errorOccured(ec);
}

void QUdpMessageChannel::handleDisconnect(const boost::system::error_code& ec) {
    emit disconnected(ec);
}

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END