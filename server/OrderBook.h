#pragma once
#include <map>
#include <deque>
#include <unordered_map>
#include "Order.h"


class OrderBook {
public:

    class PriceLevel {
    public:
        Order& front() { 
            return orders_.front(); 
        }
        void popFront() { 
            orders_.pop_front(); 
        }
        bool empty() const { 
            return orders_.empty(); 
        }
        void addOrder(const Order& o);
        bool removeOrder(int orderId);
        double totalQuantity() const;
            
        private:
            std::deque<Order> orders_;


        };

        void insertOrder(const Order& o);
        bool cancelOrder(int orderId);
        bool hasOrder(int orderId) const;
        void removeOrderId(int orderId);
        double bestBid() const;
        double bestAsk() const;
        PriceLevel& asksAt(double price) { return asks_[price]; }
        PriceLevel& bidsAt(double price) { return bids_[price]; }
        void removeAskLevel(double price) { asks_.erase(price); }
        void removeBidLevel(double price) { bids_.erase(price); }
        
    private:

        std::map<double, PriceLevel, std::greater<>> bids_;
        std::map<double, PriceLevel> asks_;
        std::unordered_map<int, std::pair<double, Side>> orderIdMap_;
        
};