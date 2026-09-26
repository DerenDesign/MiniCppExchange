#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <cassert>
#include <iostream>
#include "MatchingEngine.h"
#include <assert.h>

TEST_CASE("Resting order"){

    MatchingEngine engine;
    Order buyOrder{1, 100.0, 10.0, Side::BUY};
    auto trades = engine.submitOrder(buyOrder);

    CHECK(trades.empty());
    CHECK(engine.bestBid() == 100.0);

    std::cout << "Test 1 passed: Resting order not crossing." << std::endl;

}

TEST_CASE("Matching buy and sell orders"){

    MatchingEngine engine;
    Order sell{1, 100.0, 50, Side::SELL};
    engine.submitOrder(sell);

    Order buy{2, 100.0, 30, Side::BUY};
    auto trades = engine.submitOrder(buy);

    REQUIRE(trades.size() == 1);
    CHECK(trades[0].buyOrderId == 2);
    CHECK(trades[0].sellOrderId == 1);
    CHECK(trades[0].executionPrice == 100.0);
    CHECK(engine.bestAsk() == 100.0);

    std::cout << "Test 2 passed: Matching buy and sell orders." << std::endl;

}

TEST_CASE("Sweeping multiple price levels"){
    MatchingEngine engine;
    engine.submitOrder(Order{1, 100.0, 50, Side::SELL});
    engine.submitOrder(Order{2, 101.0, 50, Side::SELL});

    Order buy{3, 101.0, 100, Side::BUY};    
    auto trades = engine.submitOrder(buy);

    REQUIRE(trades.size() == 2);
    CHECK(trades[0].executionPrice == 100.0);       
    CHECK(trades[1].executionPrice == 101.0);

    std::cout << "Test 3 passed: Sweeping multiple price levels." << std::endl;

}

TEST_CASE("Market buy consumes best asks and does not rest"){
    MatchingEngine engine;
    engine.submitOrder(Order{1, 100.0, 10, Side::SELL});
    engine.submitOrder(Order{2, 101.0, 10, Side::SELL});

    auto trades = engine.submitOrder(Order{3, 0.0, 15, Side::BUY, OrderType::MARKET});

    REQUIRE(trades.size() == 2);
    CHECK(trades[0].executionPrice == 100.0);
    CHECK(trades[0].quantity == 10.0);
    CHECK(trades[1].executionPrice == 101.0);
    CHECK(trades[1].quantity == 5.0);
    CHECK(engine.bestAsk() == 101.0);
    CHECK(engine.bestBid() == 0.0);
}

TEST_CASE("Market sell does not rest when liquidity is insufficient"){
    MatchingEngine engine;
    engine.submitOrder(Order{1, 100.0, 5, Side::BUY});

    auto trades = engine.submitOrder(Order{2, 0.0, 10, Side::SELL, OrderType::MARKET});

    REQUIRE(trades.size() == 1);
    CHECK(trades[0].executionPrice == 100.0);
    CHECK(trades[0].quantity == 5.0);
    CHECK(engine.bestBid() == 0.0);
    CHECK(engine.bestAsk() == 0.0);
}

