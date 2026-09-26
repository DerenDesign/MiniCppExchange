#pragma once

struct Trade {
    int buyOrderId;
    int sellOrderId;
    double executionPrice;
    double quantity;
    long timestamp; // Unix timestamp in milliseconds
    int tradeId; // Internal incrementing trade ID for tracking



};