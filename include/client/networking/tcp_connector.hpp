#pragma once

#include <common/includes.hpp>
#include <common/networking/sockets.hpp>

CLIENT_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

using namespace ::COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL;

class QTcpConnector : public QObject
{
    Q_OBJECT
private:
    boost::asio::io_context* _context = nullptr;
public:
    explicit QTcpConnector(QObject* parent = nullptr);
public:
    void setContext(boost::asio::io_context& context);
public:
    void startConnection(const boost::asio::ip::tcp::endpoint& destination);
signals:
    void connectionEstablished(std::shared_ptr<QTcpAsioSocket> socket, boost::asio::ip::tcp::endpoint destination);
    void failedToEstablishConnection(boost::system::error_code ec);
};

NETWORKING_NAMESPACE_END
CLIENT_NAMESPACE_END

Q_DECLARE_METATYPE(std::shared_ptr<::COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL::QTcpAsioSocket>)