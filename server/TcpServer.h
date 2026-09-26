#pragma once
#include <iostream>
#include <asio.hpp>
#include <memory>
#include <vector>
#include <cstring>
#include "MatchingEngine.h"
#include "Session.h"
#include "Protocol.h"

class TcpServer {
public:
    TcpServer(asio::io_context& io, int port, MatchingEngine& engine)
        : acceptor_(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          engine_(engine) {
        std::cout << "[Server] Listening on port " << port << std::endl;
        doAccept();
    }

private:
    void doAccept() {
        acceptor_.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::cout << "[Server] New client connected" << std::endl;

                    auto session = std::make_shared<Session>(
                        std::move(socket), engine_,
                        [this](const Trade& trade) { broadcastTrade(trade); });

                    sessions_.push_back(session);
                    session->start();
                }
                doAccept();
            });
    }

    void broadcastTrade(const Trade& trade) {
        MessageTradeExecuted msg{MessageType::TradeExecuted, trade};
        std::cout << "[Server] Broadcasting trade " << trade.tradeId << std::endl;

        for (auto& session : sessions_) {
            session->send(&msg, sizeof(msg));
        }
    }

    asio::ip::tcp::acceptor acceptor_;
    MatchingEngine& engine_;
    std::vector<std::shared_ptr<Session>> sessions_;
};