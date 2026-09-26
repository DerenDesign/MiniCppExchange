#include <iostream>
#include "OrderBook.h"
#include "Order.h"



void OrderBook::insertOrder(const Order& o) {

    if(o.side == Side::BUY){
        bids_[o.price].addOrder(o);
        orderIdMap_[o.orderId] = {o.price, o.side}; // Store the orderId mapping for quick cancellation

    }
    else if(o.side == Side::SELL){
        asks_[o.price].addOrder(o);
        orderIdMap_[o.orderId] = {o.price, o.side}; // Store the orderId mapping for quick cancellation
    }
    else {
        std::cout << "Invalid order side for order Id: " << o.orderId << std::endl;
    }

    
}

bool OrderBook::cancelOrder(int orderId) {
    
    auto it = orderIdMap_.find(orderId);
    if(it != orderIdMap_.end()){
        double price = it->second.first;
        Side side = it->second.second;
        
        if(side == Side::BUY){
            if(bids_.count(price) && bids_[price].removeOrder(orderId)){
                std::cout << "Order with ID: " << orderId << " canceled from bids." << std::endl;
                orderIdMap_.erase(it);
                if(bids_[price].empty()){
                    bids_.erase(price);
                }
                return true;
            }
        }
        else if(side == Side::SELL){
            if(asks_.count(price) && asks_[price].removeOrder(orderId)){
                std::cout << "Order with ID: " << orderId << " canceled from asks." << std::endl;
                orderIdMap_.erase(it);
                if(asks_[price].empty()){
                    asks_.erase(price);
                }
                return true;
            }
        }

        orderIdMap_.erase(it);
    }

    std::cout << "Canceling order with ID: " << orderId << std::endl;
    return false;
}

bool OrderBook::hasOrder(int orderId) const {
    return orderIdMap_.find(orderId) != orderIdMap_.end();
}

void OrderBook::removeOrderId(int orderId) {
    orderIdMap_.erase(orderId);
}

double OrderBook::bestBid() const {

    return bids_.empty() ? 0.0 : bids_.begin()->first;
}

double OrderBook::bestAsk() const {

    return asks_.empty() ? 0.0 : asks_.begin()->first;

}

void OrderBook::PriceLevel::addOrder(const Order& o) {
    orders_.push_back(o);
}

bool OrderBook::PriceLevel::removeOrder(int orderId){

    for(auto it = orders_.begin(); it != orders_.end(); ++it){

        if(it->orderId == orderId) {

            orders_.erase(it);
            std::cout << "Order with ID: " << orderId << " removed from price level." << std::endl;
            return true;
        }

    } 
    std::cout << "Order with ID: " << orderId << " not found in price level." << std::endl;
    return false;
}

double OrderBook::PriceLevel::totalQuantity() const {
    double total = 0.0;
    for(const auto& order : orders_){
        total += order.quantity;
    }
    std::cout << "Total quantity for price level: " << total << std::endl;
    return total;
}
