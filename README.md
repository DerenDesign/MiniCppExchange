# MiniCppExchange

A simulated order book written in C++ with a price-time-priority matching engine,  
TCP client/server communication, and a Qt-based order book GUI. This project models a simple  
exchange where clients can submit and cancel limit orders, orders are matched on the other  
side of the book, trades are generated, and updates are shared with connected clients.

<img width="524" height="471" alt="image" src="https://github.com/user-attachments/assets/abd71ecb-3f16-4a04-b2bd-4658f9cab722" />

## Features

- Limit buy and sell orders (including fractional shares)
- Market buy and sell orders that execute against available best-price liquidity
- Price-time-priority matching
- Partial order fills
- Order cancellation by ID
- TCP networking using Asio
- Trade broadcasts to connected clients
- Qt Desktop GUI for order book visualization

## Architecture

The project is split into three main components:

```text id="x3r7ys"
MiniCppExchange
│
├── client/           # Client-side networking
├── common/           # Shared protocol and data structures
├── server/           # Matching engine, order book, and TCP server
├── tests/            # Matching engine unit tests
│
└── qt-frontend.cpp   # Qt graphical client
```

## Order Book Design

```cpp id="wtks7u"
std::map<double, PriceLevel, std::greater<>> bids;
std::map<double, PriceLevel> asks;
```

This keeps the highest bid and lowest ask accessible in O(1) time. Adding and removing price levels is O(log n) time.

Each price level has a FIFO queue of orders:

```cpp id="18sxay"
std::deque<Order>
```

This preserves FIFO time priority between orders at the same price, with O(1) access to the oldest resting order.

An order-ID lookup is used for cancellation:

```cpp id="1hho1h"
std::unordered_map<int, std::pair<double, Side>>
```

This allows the exchange to find the order's price level in O(1) average time without searching the entire order book. The order is then located within that price level's FIFO queue.

## Networking

The exchange runs on a TCP connection using Asio.

The server listens on port 12345.

Each client has its own session while sharing the same matching engine and central order book.

The protocol defines the following message types:

```text id="m2l82g"
AddOrder
CancelOrder
TradeExecuted
BookUpdate
```

## Testing

Unit tests with doctest.

## Future Improvements

- Stop and stop-limit orders
- Modify & replace order messages
- Level 2 market data

## Requirements

- C++17-compatible compiler
- GCC / MinGW-w64 or equivalent
- Qt 6
- Standalone Asio
- Git

### MSYS2 Dependencies

Install GCC:

```bash id="7x4d43"
pacman -S mingw-w64-ucrt-x86_64-gcc
```

Install Qt:

```bash id="e1oxx4"
pacman -S mingw-w64-ucrt-x86_64-qt6-base
```

Install standalone Asio:

```bash id="8copqc"
pacman -S mingw-w64-ucrt-x86_64-asio
```

Clone the repository:

```bash id="9dm826"
git clone https://github.com/DerenDesign/MiniCppExchange.git
cd MiniCppExchange
```

Build and start the server first.

The server listens for connections on port 12345.

Then launch either the command-line client or Qt frontend to connect to the exchange.
