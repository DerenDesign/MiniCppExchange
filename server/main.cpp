#include <iostream>
#include <asio.hpp>
#include "MatchingEngine.h"
#include "TcpServer.h"

int main() {
    try {
        asio::io_context io;
        MatchingEngine engine;
        TcpServer server(io, 12345, engine);

        std::cout << "[Server] Running. Press Ctrl+C to stop." << std::endl;
        io.run();
    } catch (const std::exception& ex) {
        std::cerr << "[Server] Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
