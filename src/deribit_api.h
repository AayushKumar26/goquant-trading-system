#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class DeribitClient {
public:
    explicit DeribitClient(const std::string& config_path);
    
    // Core methods
    json get_positions();
    void cancel_order(const std::string& order_id);
    json get_instruments(const std::string& currency,
                        const std::string& kind,
                        bool expired);
    json place_order(const std::string& instrument_name,
                    const std::string& direction,
                    double quantity,
                    double price,
                    const std::string& order_type = "limit");

private:
    std::string client_id_;
    std::string client_secret_;
    std::string access_token_;
    
    void authenticate();
    json http_post(const std::string& path, const json& body);
};

class DeribitWebSocket {
public:
    DeribitWebSocket(net::io_context& ioc, ssl::context& ctx);
    ~DeribitWebSocket();
    
    void connect(const std::string& host, const std::string& port);
    void subscribe_orderbook(const std::string& instrument);
    std::string read();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};