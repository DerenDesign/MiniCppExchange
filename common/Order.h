#pragma once


enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};


struct Order {

    int orderId;
    double price;
    double quantity; //Fractional shares allowed (Most brokerages allow this since 2020)
    Side side;
    OrderType type = OrderType::LIMIT;

};