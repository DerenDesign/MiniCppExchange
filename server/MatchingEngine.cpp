#include <iostream>
#include "MatchingEngine.h"
#include <algorithm>
#include <chrono>
#include <cmath>

std::vector<Trade> MatchingEngine::submitOrder(const Order& o){
    std::cout << "[MatchingEngine] Submit order " << o.orderId << " side=" << (o.side == Side::BUY ? "BUY" : "SELL")
              << " price=" << o.price << " qty=" << o.quantity << std::endl;
    Order incoming = o;
    return match(incoming);
}

bool MatchingEngine::cancelOrder(int orderId){
    return book_.cancelOrder(orderId);
}

double MatchingEngine::bestBid() const{
    return book_.bestBid();
}

double MatchingEngine::bestAsk() const{
    return book_.bestAsk();
}

std::vector<Trade> MatchingEngine::match(Order& incomingOrder) {
 
    std::vector<Trade> trades;
    if (incomingOrder.orderId < 0 || incomingOrder.quantity <= 0.0 ||
        !std::isfinite(incomingOrder.quantity) || book_.hasOrder(incomingOrder.orderId) ||
        (incomingOrder.type == OrderType::LIMIT &&
         (!std::isfinite(incomingOrder.price) || incomingOrder.price <= 0.0))) {
        return trades;
    }

    Side side = incomingOrder.side;
    if(side == Side::BUY){
        
        

            while(incomingOrder.quantity > 0 && book_.bestAsk() > 0.0 &&
                (incomingOrder.type == OrderType::MARKET || incomingOrder.price >= book_.bestAsk())) {

                auto& level = book_.asksAt(book_.bestAsk());

                Order& restingOrder = level.front();

                double fillQuantity = std::min(incomingOrder.quantity, restingOrder.quantity);

                std::cout << "[MatchingEngine] Creating trade for order " << incomingOrder.orderId
                          << " against resting order " << restingOrder.orderId << std::endl;
                trades.push_back(Trade{
                    incomingOrder.orderId,
                    restingOrder.orderId,
                    restingOrder.price,
                    fillQuantity,
                    static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count()),
                    nextTradeId_++
                });

                if (fillQuantity >= incomingOrder.quantity) {
                    incomingOrder.quantity = 0.0;
                } else {
                    incomingOrder.quantity -= fillQuantity;
                }
                if (fillQuantity >= restingOrder.quantity) {
                    restingOrder.quantity = 0.0;
                } else {
                    restingOrder.quantity -= fillQuantity;
                }

                if(restingOrder.quantity <= 0.0) {
                    book_.removeOrderId(restingOrder.orderId);
                    level.popFront();
                }

                if(level.empty()) {
                    book_.removeAskLevel(book_.bestAsk());
                }
            }
            if(incomingOrder.quantity > 0 && incomingOrder.type == OrderType::LIMIT){
                    book_.insertOrder(incomingOrder);
                }
    }
    else if(side == Side::SELL){
        

            while(incomingOrder.quantity > 0 && book_.bestBid() > 0.0 &&
                (incomingOrder.type == OrderType::MARKET || incomingOrder.price <= book_.bestBid())) {

                auto& level = book_.bidsAt(book_.bestBid());

                Order& restingOrder = level.front();

                double fillQuantity = std::min(incomingOrder.quantity, restingOrder.quantity);

                std::cout << "[MatchingEngine] Creating trade for order " << incomingOrder.orderId
                          << " against resting order " << restingOrder.orderId << std::endl;
                trades.push_back(Trade{
                    restingOrder.orderId,
                    incomingOrder.orderId,
                    restingOrder.price,
                    fillQuantity,
                    static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count()),
                    nextTradeId_++
                });

                if (fillQuantity >= incomingOrder.quantity) {
                    incomingOrder.quantity = 0.0;
                } else {
                    incomingOrder.quantity -= fillQuantity;
                }
                if (fillQuantity >= restingOrder.quantity) {
                    restingOrder.quantity = 0.0;
                } else {
                    restingOrder.quantity -= fillQuantity;
                }

                if(restingOrder.quantity <= 0.0) {
                    book_.removeOrderId(restingOrder.orderId);
                    level.popFront();
                }

                 if(level.empty()) {
                    book_.removeBidLevel(book_.bestBid());
                }

            }
            if(incomingOrder.quantity > 0 && incomingOrder.type == OrderType::LIMIT){
                book_.insertOrder(incomingOrder);
            }
    }
    else {
        std::cerr << "Invalid order side for order Id: " << incomingOrder.orderId << std::endl;
    }


    return trades;
}