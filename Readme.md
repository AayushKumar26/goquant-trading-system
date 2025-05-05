# Deribit API Trading System

This project is a trading system that interacts with the Deribit API for cryptocurrency trading. It supports both REST and WebSocket APIs for placing orders, canceling orders, and subscribing to real-time market data.

## Features
- **REST API Integration**:
  - Authenticate using client credentials.
  - Place and cancel orders.
  - Fetch available instruments.
- **WebSocket API Integration**:
  - Subscribe to real-time order book updates.
  - Read live market data.

## Requirements
- **C++ Compiler**: C++17 or later.
- **Build Tools**:
  - CMake 3.15+
  - Boost 1.71+
  - OpenSSL
- **Dependencies**:
  - [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing.
  - [cURL](https://curl.se/) for HTTP requests.
  - [Boost.Asio](https://www.boost.org/) for WebSocket communication.

## Setup

### Clone the Repository
```bash
git clone https://github.com/yourusername/goquant_trading_system.git
cd goquant_trading_system
```

### Configure
1. Copy the sample configuration file:
   ```bash
   cp config/config.json.sample config/config.json
   ```
2. Edit `config/config.json` with your Deribit testnet credentials:
   ```json
   {
       "client_id": "your_client_id",
       "client_secret": "your_client_secret"
   }
   ```

### Build
1. Create a build directory and compile the project:
   ```bash
   mkdir build && cd build
   cmake .. && cmake --build .
   ```
2. The executable will be located in `build/src/Release/goquant.exe` (on Windows).

### Run
1. Navigate to the build directory:
   ```bash
   cd build/src/Release
   ```
2. Run the application:
   ```bash
   ./goquant
   ```

## Usage
### REST API
- The application authenticates with the Deribit API and places a test order for the `BTC-PERPETUAL` instrument.
- Modify the `main.cpp` file to customize the order parameters (e.g., instrument name, quantity, price).

### WebSocket API
- The application subscribes to the real-time order book for the `BTC-PERPETUAL` instrument.
- Press `ENTER` to stop the WebSocket connection.

## Project Structure
```
goquant_trading_system/
├── config/                # Configuration files
│   ├── config.json.sample # Sample configuration file
├── src/                   # Source code
│   ├── deribit_api.h      # Header file for Deribit API client
│   ├── deribit_api.cpp    # Implementation of Deribit API client
│   ├── main.cpp           # Entry point of the application
├── build/                 # Build directory (generated after compilation)
├── CMakeLists.txt         # Build configuration
└── Readme.md              # Project documentation
```

## Dependencies
### Installing Dependencies on Windows
1. Install [Boost](https://www.boost.org/).
2. Install [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html).
3. Install [cURL](https://curl.se/windows/).

### Installing Dependencies on Linux
```bash
sudo apt update
sudo apt install libboost-all-dev libssl-dev libcurl4-openssl-dev
```

## Troubleshooting
### Common Errors
1. **"must be a multiple of contract size"**:
   - Ensure the order quantity is a multiple of the instrument's contract size.
   - Fetch the contract size using the `get_instruments` API.

2. **"Authentication failed"**:
   - Verify your `client_id` and `client_secret` in `config.json`.

3. **Build errors**:
   - Ensure all dependencies (Boost, OpenSSL, cURL) are installed and properly linked.

## Acknowledgments
- [Deribit API Documentation](https://docs.deribit.com/)
- [nlohmann/json](https://github.com/nlohmann/json)
- [Boost.Asio](https://www.boost.org/)
