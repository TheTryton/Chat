#pragma once

#include <common/includes.hpp>

Q_DECLARE_METATYPE(boost::asio::ip::tcp::resolver::results_type)
Q_DECLARE_METATYPE(boost::asio::ip::udp::resolver::results_type)

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN

class QTcpResolver : public QObject
{
Q_OBJECT
private:
    boost::asio::io_context* _context = nullptr;
    std::unique_ptr<boost::asio::ip::tcp::resolver> _resolver;
public:
    explicit QTcpResolver(QObject* parent = nullptr);
public:
    void setContext(boost::asio::io_context& context);
public:
    void startResolving(std::string_view host, std::string_view service);
signals:
    void resolved(boost::asio::ip::tcp::resolver::results_type results);
    void resolutionFailed(boost::system::error_code ec);
};

class QUdpResolver : public QObject
{
Q_OBJECT
private:
    boost::asio::io_context* _context = nullptr;
    std::unique_ptr<boost::asio::ip::udp::resolver> _resolver;
public:
    explicit QUdpResolver(QObject* parent = nullptr);
public:
    void setContext(boost::asio::io_context& context);
public:
    void startResolving(std::string_view host, std::string_view service);
signals:
    void resolved(boost::asio::ip::udp::resolver::results_type results);
    void resolutionFailed(boost::system::error_code ec);
};

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END