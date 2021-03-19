#include <server/networking/tcp_acceptor.hpp>

SERVER_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

QTcpAcceptor::QTcpAcceptor(QObject* parent)
    : QObject(parent)
{

}

void QTcpAcceptor::setContext(boost::asio::io_context& context)
{
    if(&context != _context)
    {
        stopAccepting();
        _context = &context;
    }
}

void QTcpAcceptor::startAccepting(uint16_t port)
{
    Q_ASSERT(_context != nullptr);

    boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), port};
    _acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(*_context, endpoint);
    acceptingHandlerLoop(std::make_unique<boost::asio::ip::tcp::socket>(*_context));
}

void QTcpAcceptor::acceptingHandlerLoop(std::unique_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto& sock = *socket;
    _acceptor->async_accept(sock, [this, socket = std::move(socket)](boost::system::error_code ec) mutable
    {
        if(ec)
        {
            emit failedToAccept(ec);
        }
        else
        {
            auto endpoint = socket->remote_endpoint();
            emit connectionEstablished(std::make_shared<QTcpAsioSocket>(std::move(*socket)), endpoint);
        }
        acceptingHandlerLoop(std::move(socket));
    });
}

void QTcpAcceptor::stopAccepting()
{
    if(_acceptor)
    {
        _acceptor->cancel();
    }
}

NETWORKING_NAMESPACE_END
SERVER_NAMESPACE_END