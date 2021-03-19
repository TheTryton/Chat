#include <common/networking/resolver.hpp>

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

QTcpResolver::QTcpResolver(QObject* parent)
    : QObject(parent)
{

}

void QTcpResolver::setContext(boost::asio::io_context& context)
{
    if(_context != &context)
    {
        _context = &context;
        _resolver = std::make_unique<boost::asio::ip::tcp::resolver>(*_context);
    }
}
void QTcpResolver::startResolving(std::string_view host, std::string_view service)
{
    Q_ASSERT(_context != nullptr);

    _resolver->async_resolve(host, service, [this]
    (const boost::system::error_code& ec, const boost::asio::ip::tcp::resolver::results_type& results)
    {
        if(ec)
        {
            emit resolutionFailed(ec);
        }
        else
        {
            emit resolved(results);
        }
    });
}

QUdpResolver::QUdpResolver(QObject* parent)
    : QObject(parent)
{

}
void QUdpResolver::setContext(boost::asio::io_context& context)
{
    if(_context != &context)
    {
        _context = &context;
        _resolver = std::make_unique<boost::asio::ip::udp::resolver>(*_context);
    }
}
void QUdpResolver::startResolving(std::string_view host, std::string_view service)
{
    Q_ASSERT(_context != nullptr);

    _resolver->async_resolve(host, service, [this]
        (const boost::system::error_code& ec, const boost::asio::ip::udp::resolver::results_type& results)
    {
        if(ec)
        {
            emit resolutionFailed(ec);
        }
        else
        {
            emit resolved(results);
        }
    });
}

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END