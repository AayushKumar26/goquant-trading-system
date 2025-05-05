#include "deribit_api.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <memory>

int main() {
   std::filesystem::path config_path = std::filesystem::current_path()  // build
    .parent_path()  // Project root (D:\goquant_trading_system)
    / "config" 
    / "config.json";

    std::cout << "Looking for config at: " << config_path << std::endl;
    
    try {
        // REST API operations
        DeribitClient client(config_path.string());
        auto instruments = client.get_instruments("BTC", "future", false);
        
        // Order placement logic
        auto order = client.place_order("BTC-PERPETUAL", "buy", 1.0, 30000.0);
        std::cout << "Order placed: " << order.dump(2) << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // WebSocket thread
    auto running = std::make_shared<std::atomic<bool>>(true);
    std::thread ws_thread([running]() {
        try {
            net::io_context ioc;
            ssl::context ctx{ssl::context::tlsv12_client};
            DeribitWebSocket ws(ioc, ctx);
            
            ws.connect("test.deribit.com", "443");
            ws.subscribe_orderbook("BTC-PERPETUAL");
            
            while (*running) {
                std::cout << "WS Update: " << ws.read() << "\n";
            }
        } catch (...) {}
    });

    std::cout << "Press ENTER to exit...\n";
    std::cin.ignore();
    *running = false;
    if (ws_thread.joinable()) ws_thread.join();

    return 0;
}