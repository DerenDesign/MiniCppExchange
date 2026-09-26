#pragma once
#include <vector>
#include "OrderBook.h"
#include "Trade.h"

class MatchingEngine {

    public:
    std::vector<Trade> submitOrder(const Order& o);
    bool cancelOrder(int orderId);
    double bestBid() const;
    double bestAsk() const;


    private:
    std::vector<Trade> match(Order& incomingOrder);
    OrderBook book_;
    int nextTradeId_ = 0; //For logging and tracking trades internally


};