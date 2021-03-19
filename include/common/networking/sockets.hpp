#pragma once

#include <common/includes.hpp>

Q_DECLARE_METATYPE(boost::system::error_code)

COMMON_NAMESPACE_BEGIN
NETWORKING_NAMESPACE_BEGIN


template<typename SocketType>
class QBaseAsioSocket
{
protected:
    SocketType _socket;
public:
    SocketType& socket() {return _socket;}
    const SocketType& socket() const {return _socket;}
public:
    QBaseAsioSocket(SocketType socket)
        : _socket(std::move(socket))
    {}
};

class QTcpAsioSocket final : public QBaseAsioSocket<boost::asio::ip::tcp::socket>
{
public:
    using QBaseAsioSocket<boost::asio::ip::tcp::socket>::QBaseAsioSocket;
    friend class QTcpConnector;
public:
    template<typename F>
    void asyncWrite(boost::asio::const_buffer buffer, F&& handler)
    {
        boost::asio::async_write(
            socket(),
            buffer,
            std::forward<F>(handler)
        );
    }
    template<typename F>
    void asyncRead(boost::asio::mutable_buffer buffer, F&& handler)
    {
        boost::asio::async_read(
            socket(),
            buffer,
            std::forward<F>(handler)
        );
    }
};

/*
class QUdpAsioDemultiplexer final
{
    friend class QBoundUdpAsioSocket;
private:
    boost::asio::ip::udp::socket _socket;

    std::mutex _sending_m;
    struct send_request
    {
        boost::asio::const_buffer buffer;
        boost::asio::ip::udp::endpoint destination;
        std::function<void(boost::system::error_code, size_t)> handler;
    };
    std::queue<send_request> _send_requests;
    bool _sending_ongoing = false;

    std::mutex _handlers_m;
    struct handler_with_buffer
    {
        boost::asio::mutable_buffer buffer;
        size_t transferred = 0;
        std::function<void(boost::system::error_code, size_t)> handler;
    };
    boost::asio::ip::udp::endpoint _current_sender;
    std::map<boost::asio::ip::udp::endpoint, handler_with_buffer> _handlers;
    bool _reading_ongoing = false;

    std::vector<std::byte> _receive_buffer;
private:
    size_t get_max_receive_buffer_size_socket()
    {
        boost::asio::socket_base::receive_buffer_size option;
        _socket.get_option(option);
        return option.value();
    }
public:
    QUdpAsioDemultiplexer(boost::asio::ip::udp::socket socket)
    : _socket(std::move(socket))
    , _receive_buffer(get_max_receive_buffer_size_socket())
    {}
    ~QUdpAsioDemultiplexer()
    {
        _socket.close();
        // dependent handlers must access data from this object
        // thus close must be explicit
    }
public:
    void asyncWriteTo(
        boost::asio::const_buffer buffer,
        const boost::asio::ip::udp::endpoint& destination,
        const std::function<void(boost::system::error_code, size_t)>& handler)
    {
        enqueue_send_request(send_request{.buffer = buffer, .destination = destination, .handler = handler});
        check_queue_and_start_sending();
    }
    void asyncReadFrom(
        boost::asio::mutable_buffer buffer,
        const boost::asio::ip::udp::endpoint& expected_sender,
        const std::function<void(boost::system::error_code, size_t)>& handler)
    {
        register_receive_handler(buffer, expected_sender, handler);
        check_handlers_and_start_receiving();
    }
private:
    void register_receive_handler(
        boost::asio::mutable_buffer buffer,
        const boost::asio::ip::udp::endpoint& expected_sender,
        const std::function<void(boost::system::error_code, size_t)>& handler)
    {
        std::unique_lock l{_handlers_m};
        auto it = _handlers.find(expected_sender);
        if(it != std::end(_handlers)) it->second.handler(boost::asio::error::interrupted, it->second.transferred);
        _handlers[expected_sender] = handler_with_buffer{.buffer = buffer, .transferred = 0, .handler = handler};
    }
    void check_handlers_and_start_receiving(bool in_handler = false)
    {
        bool should_start_new_read = false;
        {
            std::unique_lock l{_handlers_m};
            if(in_handler)
            {
                if(!_handlers.empty())
                {
                    should_start_new_read = true;
                    _reading_ongoing = true;
                }
                else
                {
                    _reading_ongoing = false;
                }
            }
            else
            {
                if(!_sending_ongoing && !_send_requests.empty())
                {
                    should_start_new_read = true;
                    _reading_ongoing = true;
                }
            }
        }

        if(should_start_new_read)
        {
            auto buf = boost::asio::buffer(_receive_buffer);
            _socket.async_receive_from(boost::asio::buffer(_receive_buffer), _current_sender, [this]
            (boost::system::error_code ec, size_t bt){
                using it_type =  std::map<boost::asio::ip::udp::endpoint, handler_with_buffer>::iterator;
                if(ec)
                {
                    std::unique_lock l{_handlers_m};
                    for(auto& handler : _handlers)
                    {
                        handler.second.handler(ec, handler.second.transferred);
                    }
                    _handlers.clear();
                    _reading_ongoing=false;
                }
                else
                {
                    std::optional<it_type> it_h;
                    {
                        std::unique_lock l{_handlers_m};
                        auto it = _handlers.find(_current_sender);
                        if(it != std::end(_handlers))
                        {
                            it_h = it;
                        }
                    }

                    if(it_h)
                    {
                        auto it = *it_h;
                        auto to_transfer = std::min(it->second.buffer.size(), bt);
                        std::memcpy(it->second.buffer.data(), _receive_buffer.data(), to_transfer);
                        it->second.transferred += to_transfer;
                        it->second.buffer += to_transfer;
                        if(it->second.buffer.size() == 0)
                        {
                            std::unique_lock l{_handlers_m};
                            it->second.handler(ec, it->second.transferred);
                            _handlers.erase(it);
                        }
                        check_handlers_and_start_receiving(true);
                    }
                    else
                    {
                        check_handlers_and_start_receiving(true);
                    }
                }
            });
        }
    }
    void enqueue_send_request(send_request&& sr)
    {
        std::unique_lock l{_sending_m};
        _send_requests.push(std::move(sr));
    }
    void check_queue_and_start_sending(bool in_handler = false)
    {
        std::optional<send_request> send_to_start;
        {
            std::unique_lock l{_sending_m};
            if(in_handler)
            {
                if(!_send_requests.empty())
                {
                    send_to_start = std::move(_send_requests.front());
                    _send_requests.pop();
                    _sending_ongoing = true;
                }
                else
                {
                    _sending_ongoing = false;
                }
            }
            else
            {
                if(!_sending_ongoing && !_send_requests.empty())
                {
                    send_to_start = std::move(_send_requests.front());
                    _send_requests.pop();
                    _sending_ongoing = true;
                }
            }
        }

        if(send_to_start)
        {
            auto& sr = *send_to_start;
            _socket.async_send_to(sr.buffer, sr.destination, [this,
                handler = std::move(sr.handler)](boost::system::error_code ec, size_t t)
            {
                if(ec)
                {
                    handler(ec, t);
                    std::unique_lock l{_sending_m};
                    _sending_ongoing = false;
                }
                else
                {
                    check_queue_and_start_sending(true);
                    handler(ec, t);
                }
            });
        }
    }
};*/

class QUdpAsioSocket final : public QBaseAsioSocket<boost::asio::ip::udp::socket>
{
private:
    boost::asio::ip::udp::endpoint sender;
public:
    QUdpAsioSocket(boost::asio::ip::udp::socket socket)
        : QBaseAsioSocket<boost::asio::ip::udp::socket>(std::move(socket))
    {}
public:
    template<typename F>
    void asyncWrite(boost::asio::const_buffer buffer,
                    const boost::asio::ip::udp::endpoint& destination,
                    F&& handler)
    {
        socket().async_send_to(buffer, destination, std::forward<F>(handler));
    }
    template<typename F>
    void asyncRead(boost::asio::mutable_buffer buffer,
                   F&& handler)
    {
        socket().async_receive_from(buffer, sender, [this, handler = std::forward<F>(handler)](boost::system::error_code ec, size_t bt){
            if(ec)
            {
                handler(ec, boost::asio::ip::udp::endpoint(), bt);
            }
            else
            {
                handler(ec, sender, bt);
            }
        });
    }
};

constexpr const char* multicast_address = "239.1.2.3";
constexpr uint16_t multicast_port = 5556;

NETWORKING_NAMESPACE_END
COMMON_NAMESPACE_END