# src/CMakeLists.txt

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS system thread)  # for WebSocket later

add_executable(goquant
    main.cpp
    deribit_api.cpp
    # order_manager.cpp
    # websocket_server.cpp
    # logger.cpp
)

target_include_directories(goquant PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(goquant PRIVATE
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    nlohmann_json::nlohmann_json
)