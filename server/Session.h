#pragma once
#include <asio.hpp>
#include <memory>
#include <array>
#include <iostream>
#include <cstring>
#include <functional>
#include <deque>
#include <vector>
#include "MatchingEngine.h"
#include "Protocol.h"

class Session : public std::enable_shared_from_this<Session> {
public:
    using TradeCallback = std::function<void(const Trade&)>;

    Session(asio::ip::tcp::socket socket, MatchingEngine& engine, TradeCallback onTrade)
        : socket_(std::move(socket)), engine_(engine), onTrade_(std::move(onTrade)) {}

    void start() {
        std::cout << "[Session] Starting session" << std::endl;
        doRead();
    }

    void send(const void* data, size_t length) {
        auto self = shared_from_this();
        std::vector<char> message(static_cast<const char*>(data),
                                  static_cast<const char*>(data) + length);
        bool startWrite = outgoingMessages_.empty();
        outgoingMessages_.push_back(std::move(message));
        if (startWrite) {
            doWrite();
        }
    }

private:
    void doRead() {
        auto self = shared_from_this();
        socket_.async_read_some(asio::buffer(buffer_),
            [self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "[Session] Received " << length << " bytes" << std::endl;
                    self->incomingData_.insert(self->incomingData_.end(),
                                               self->buffer_.data(),
                                               self->buffer_.data() + length);
                    self->processMessages();
                    self->doRead();
                } else {
                    std::cerr << "[Session] Client disconnected: " << ec.message() << std::endl;
                }
            });
    }

    void doWrite() {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(outgoingMessages_.front()),
            [self](std::error_code ec, size_t /*length*/) {
                if (ec) {
                    std::cerr << "Error sending data: " << ec.message() << std::endl;
                    self->outgoingMessages_.clear();
                    return;
                }
                self->outgoingMessages_.pop_front();
                if (!self->outgoingMessages_.empty()) {
                    self->doWrite();
                }
            });
    }

    void processMessages() {
        while (incomingData_.size() >= sizeof(MessageType)) {
            uint8_t rawType;
            std::memcpy(&rawType, incomingData_.data(), sizeof(rawType));
            MessageType type = static_cast<MessageType>(rawType);
            size_t messageSize = 0;

            if (type == MessageType::AddOrder) {
                messageSize = sizeof(MessageOrder);
            } else if (type == MessageType::CancelOrder) {
                messageSize = sizeof(MessageCancelOrder);
            } else {
                incomingData_.erase(incomingData_.begin());
                continue;
            }

            if (incomingData_.size() < messageSize) {
                return;
            }

            handleMessage(incomingData_.data(), messageSize);
            incomingData_.erase(incomingData_.begin(), incomingData_.begin() + messageSize);
        }
    }

    void handleMessage(const char* data, size_t length) {
        if (length < sizeof(MessageType)) return;

        uint8_t rawType;
        std::memcpy(&rawType, data, sizeof(rawType));
        MessageType type = static_cast<MessageType>(rawType);

        if (type == MessageType::AddOrder && length >= sizeof(MessageOrder)) {
            MessageOrder msg;
            std::memcpy(&msg, data, sizeof(MessageOrder));

            std::cout << "[Session] Processing AddOrder for order " << msg.orderId << std::endl;
            Order order{msg.orderId, msg.price, msg.quantity, msg.side, msg.orderType};
            auto trades = engine_.submitOrder(order);

            std::cout << "[Session] Order " << msg.orderId << " produced " << trades.size() << " trade(s)" << std::endl;
            for (const auto& trade : trades) {
                onTrade_(trade);
            }
        }
        else if (type == MessageType::CancelOrder && length >= sizeof(MessageCancelOrder)) {
            MessageCancelOrder msg;
            std::memcpy(&msg, data, sizeof(MessageCancelOrder));
            std::cout << "[Session] Processing CancelOrder for order " << msg.orderId << std::endl;
            engine_.cancelOrder(msg.orderId);
        }
        else {
            std::cout << "[Session] Ignoring unsupported message type" << std::endl;
        }
    }

    asio::ip::tcp::socket socket_;
    std::array<char, 1024> buffer_;
    std::vector<char> incomingData_;
    std::deque<std::vector<char>> outgoingMessages_;
    MatchingEngine& engine_;
    TradeCallback onTrade_;
};