#pragma once

#include <common/includes.hpp>
#include <common/networking/sockets.hpp>

SERVER_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

using namespace ::COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL;

class QTcpAcceptor : public QObject
{
    Q_OBJECT
private:
    boost::asio::io_context* _context = nullptr;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
public:
    explicit QTcpAcceptor(QObject* parent = nullptr);
public:
    void setContext(boost::asio::io_context& context);
public slots:
    void startAccepting(uint16_t port);
    void stopAccepting();
private:
    void acceptingHandlerLoop(std::unique_ptr<boost::asio::ip::tcp::socket> socket);
signals:
    void connectionEstablished(std::shared_ptr<QTcpAsioSocket> socket,
                               boost::asio::ip::tcp::endpoint destination);
    void failedToAccept(boost::system::error_code ec);
};

NETWORKING_NAMESPACE_END
SERVER_NAMESPACE_END

Q_DECLARE_METATYPE(std::shared_ptr<::COMMON_NAMESPACE_FULL::NETWORKING_NAMESPACE_FULL::QTcpAsioSocket>)