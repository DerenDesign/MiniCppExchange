#include <asio.hpp>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <thread>
#include "Protocol.h"

int main() {
    try {
        asio::io_context io;
        asio::ip::tcp::socket socket(io);
        asio::ip::tcp::resolver resolver(io);

        std::cout << "[Client] Connecting to 127.0.0.1:12345" << std::endl;
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        asio::connect(socket, endpoints);
        std::cout << "[Client] Connected" << std::endl;

        MessageOrder msg;
        msg.type = MessageType::AddOrder;
        msg.orderId = 1001;
        msg.price = 100.0;
        msg.quantity = 10.0;
        msg.side = Side::SELL;

        std::cout << "[Client] Sending AddOrder" << std::endl;
        socket.send(asio::buffer(&msg, sizeof(msg)));

        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "[Client] Done" << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "[Client] Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
