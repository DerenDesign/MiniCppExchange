#pragma once
#include "Order.h"
#include "Trade.h"
#include <cstdint>

enum class MessageType: uint8_t {
    AddOrder,
    CancelOrder,
    TradeExecuted,
    BookUpdate
};

struct MessageOrder {
    MessageType type = MessageType::AddOrder;
    int orderId;
    double price;
    double quantity;
    Side side;
    OrderType orderType = OrderType::LIMIT;
};

struct MessageCancelOrder{
    MessageType type = MessageType::CancelOrder;
    int orderId;
};

struct MessageTradeExecuted {
    MessageType type = MessageType::TradeExecuted;
    Trade trade;
};