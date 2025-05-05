#include "deribit_api.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

// Helper Functions
static std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open " + path);
    return {std::istreambuf_iterator<char>(in), {}};
}

static size_t curl_write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

// DeribitClient Implementation
DeribitClient::DeribitClient(const std::string& config_path) {
    auto cfg = json::parse(read_file(config_path));
    client_id_ = cfg.at("client_id");
    client_secret_ = cfg.at("client_secret");
    authenticate();
}

void DeribitClient::authenticate() {
    json body = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", client_id_},
            {"client_secret", client_secret_}
        }}
    };

    json resp = http_post("/api/v2/public/auth", body);
    std::cout << "Auth Response:\n" << resp.dump(2) << std::endl;

    if (resp.contains("error")) {
        throw std::runtime_error("Auth failed: " + resp.dump());
    }
    access_token_ = resp["result"]["access_token"];
}

json DeribitClient::http_post(const std::string& path, const json& body) {
    CURL* curl = curl_easy_init();
    std::string url = "https://test.deribit.com" + path;
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!access_token_.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + access_token_).c_str());
    }

    // Store payload to keep data valid
    std::string payload = body.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    return json::parse(response);
}

json DeribitClient::place_order(const std::string& instrument, const std::string& direction,
    double quantity, double price, const std::string& order_type) {
// Validate quantity is a positive integer
if (quantity <= 0 || static_cast<int>(quantity) != quantity) {
throw std::invalid_argument("Quantity must be a positive integer");
}

json body = {
{"jsonrpc", "2.0"},
{"id", 2},
{"method", direction == "buy" ? "private/buy" : "private/sell"},
{"params", {
{"instrument_name", instrument},
{"amount", static_cast<int>(quantity)},  // Number of contracts
{"price", price},
{"type", order_type}
}}
};

json resp = http_post("/api/v2/private/" + direction, body);

if (resp.contains("error")) {
throw std::runtime_error("Order failed: " + resp["error"].dump());
}
return resp["result"];
}

void DeribitClient::cancel_order(const std::string& order_id) {
    json body = {
        {"jsonrpc", "2.0"},
        {"id", 3},
        {"method", "private/cancel"},
        {"params", {{"order_id", order_id}}}
    };

    json resp = http_post("/api/v2/private/cancel", body);
    if (resp.contains("error")) {
        throw std::runtime_error("Cancel failed: " + resp["error"].dump());
    }
}

json DeribitClient::get_instruments(const std::string& currency, 
                                   const std::string& kind, 
                                   bool expired) {
    json body = {
        {"jsonrpc", "2.0"},
        {"id", 4},
        {"method", "public/get_instruments"},
        {"params", {
            {"currency", currency},
            {"kind", kind},
            {"expired", expired}
        }}
    };
    return http_post("/api/v2/public/get_instruments", body)["result"];
}

// ====================== WebSocket Implementation ======================
struct DeribitWebSocket::Impl {
    Impl(net::io_context& ioc, ssl::context& ctx)
        : ws_(net::make_strand(ioc), ctx), ctx_(ctx) {}  // Fixed initialization

    beast::websocket::stream<boost::asio::ssl::stream<beast::tcp_stream>> ws_;
    ssl::context& ctx_;

    void connect(const std::string& host, const std::string& port) {
        tcp::resolver resolver(ws_.get_executor());
        auto const results = resolver.resolve(host, port);
        
        // Connect TCP layer
        beast::get_lowest_layer(ws_).connect(results);
        
        // SSL handshake
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str())) {
            throw beast::system_error(beast::error_code(ERR_get_error(), net::error::get_ssl_category()));
        }
        ws_.next_layer().handshake(ssl::stream_base::client);
        
        // WebSocket handshake
        ws_.handshake(host, "/ws/api/v2");
    }

    void subscribe(const std::string& channel) {
        json msg = {
            {"jsonrpc", "2.0"},
            {"id", 9000},
            {"method", "public/subscribe"},
            {"params", {{"channels", {channel}}}}
        };
        ws_.write(net::buffer(msg.dump()));
    }

    std::string read() {
        try {
            beast::flat_buffer buffer;
            ws_.read(buffer);
            return beast::buffers_to_string(buffer.data());
        } catch (const beast::system_error& e) {
            if (e.code() == websocket::error::closed) {
                return "";  // Return empty on graceful close
            }
            throw;  // Re-throw other errors
        }
        return "";  // Ensure all paths return
    }
};

// Rest of WebSocket wrapper remains unchanged

DeribitWebSocket::DeribitWebSocket(net::io_context& ioc, ssl::context& ctx)
    : impl_(std::make_unique<Impl>(ioc, ctx)) {}

DeribitWebSocket::~DeribitWebSocket() = default;

void DeribitWebSocket::connect(const std::string& host, const std::string& port) {
    impl_->connect(host, port);
}

void DeribitWebSocket::subscribe_orderbook(const std::string& instrument) {
    impl_->subscribe("orderbook." + instrument + ".100ms");
}

std::string DeribitWebSocket::read() {
    return impl_->read();
}