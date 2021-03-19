#include <client/networking/tcp_connector.hpp>

CLIENT_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

QTcpConnector::QTcpConnector(QObject* parent)
    : QObject(parent)
{}

void QTcpConnector::setContext(boost::asio::io_context& context)
{
    if(&context != _context)
    {
        _context = &context;
    }
}

void QTcpConnector::startConnection(const boost::asio::ip::tcp::endpoint& destination) {
    Q_ASSERT(_context != nullptr);
    std::shared_ptr<QTcpAsioSocket> socket = std::make_shared<QTcpAsioSocket>(boost::asio::ip::tcp::socket(*_context));
    boost::asio::async_connect(
        socket->socket(),
        std::array{destination},
        [this, socket = socket]
            (boost::system::error_code ec, const boost::asio::ip::tcp::endpoint& destination) mutable{
            if(ec)
            {
                emit failedToEstablishConnection(ec);
            }
            else
            {
                emit connectionEstablished(std::move(socket), destination);
            }
        });
}

NETWORKING_NAMESPACE_END
CLIENT_NAMESPACE_END