#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QMap>
#include <QDateTime>
#include <QTimer>
#include <algorithm>
#include <QVector>
#include <cstring>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <asio.hpp>

#include "common/Protocol.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Order Book GUI");
    window.resize(700, 600);

    QLabel *titleLabel = new QLabel("Order Book", &window);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-weight:bold; font-size:16px;");

    QLabel *statusLabel = new QLabel("Disconnected", &window);
    statusLabel->setStyleSheet("color:red; font-weight:bold;");

    QTableWidget *bidTable = new QTableWidget(0, 2, &window);
    bidTable->setHorizontalHeaderLabels({"Bid Size", "Bid Price"});
    bidTable->horizontalHeader()->setStretchLastSection(true);
    bidTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bidTable->setSelectionMode(QAbstractItemView::NoSelection);

    QTableWidget *askTable = new QTableWidget(0, 2, &window);
    askTable->setHorizontalHeaderLabels({"Ask Price", "Ask Size"});
    askTable->horizontalHeader()->setStretchLastSection(true);
    askTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    askTable->setSelectionMode(QAbstractItemView::NoSelection);

    QHBoxLayout *bookLayout = new QHBoxLayout();
    bookLayout->addWidget(bidTable);
    bookLayout->addWidget(askTable);

    QLineEdit *priceInput = new QLineEdit(&window);
    priceInput->setPlaceholderText("Price");
    QLineEdit *sizeInput = new QLineEdit(&window);
    sizeInput->setPlaceholderText("Size");
    QPushButton *buyButton = new QPushButton("Buy", &window);
    QPushButton *sellButton = new QPushButton("Sell", &window);
    buyButton->setStyleSheet("background-color:#2ecc71; color:white; font-weight:bold;");
    sellButton->setStyleSheet("background-color:#e74c3c; color:white; font-weight:bold;");

    QHBoxLayout *entryLayout = new QHBoxLayout();
    entryLayout->addWidget(new QLabel("Price:"));
    entryLayout->addWidget(priceInput);
    entryLayout->addWidget(new QLabel("Size:"));
    entryLayout->addWidget(sizeInput);
    entryLayout->addWidget(buyButton);
    entryLayout->addWidget(sellButton);

    QLineEdit *cancelIdInput = new QLineEdit(&window);
    cancelIdInput->setPlaceholderText("Order ID to cancel");
    QPushButton *cancelButton = new QPushButton("Cancel Order", &window);

    QHBoxLayout *cancelLayout = new QHBoxLayout();
    cancelLayout->addWidget(new QLabel("Cancel ID:"));
    cancelLayout->addWidget(cancelIdInput);
    cancelLayout->addWidget(cancelButton);

    QLabel *myOrdersLabel = new QLabel("My Open Orders:", &window);
    QTableWidget *myOrdersTable = new QTableWidget(0, 4, &window);
    myOrdersTable->setHorizontalHeaderLabels({"ID", "Side", "Price", "Size"});
    myOrdersTable->horizontalHeader()->setStretchLastSection(true);
    myOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    myOrdersTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QLabel *tapeLabel = new QLabel("Tape:", &window);
    QTableWidget *tapeTable = new QTableWidget(0, 4, &window);
    tapeTable->setHorizontalHeaderLabels({"Time", "Side", "Price", "Size"});
    tapeTable->horizontalHeader()->setStretchLastSection(true);
    tapeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tapeTable->setMaximumHeight(150);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->addWidget(titleLabel);
    layout->addWidget(statusLabel);
    layout->addLayout(bookLayout);
    layout->addLayout(entryLayout);
    layout->addLayout(cancelLayout);
    layout->addWidget(myOrdersLabel);
    layout->addWidget(myOrdersTable);
    layout->addWidget(tapeLabel);
    layout->addWidget(tapeTable);
    window.setLayout(layout);


    static QMap<double, double> bidLevels;
    static QMap<double, double> askLevels;
    static int nextOrderId = 1;           


    static QVector<int>    orderIds;
    static QVector<bool>   orderIsBuy;
    static QVector<double> orderPrices;
    static QVector<double> orderSizes;

    auto refreshBook = [=]() {
        bidTable->setRowCount(0);
        QList<double> bidPrices = bidLevels.keys();
        std::sort(bidPrices.begin(), bidPrices.end(), std::greater<double>());
        for (double p : bidPrices) {
            int row = bidTable->rowCount();
            bidTable->insertRow(row);
            bidTable->setItem(row, 0, new QTableWidgetItem(QString::number(bidLevels[p])));
            auto *priceItem = new QTableWidgetItem(QString::number(p, 'f', 2));
            priceItem->setForeground(Qt::green);
            bidTable->setItem(row, 1, priceItem);
        }

        askTable->setRowCount(0);
        QList<double> askPrices = askLevels.keys();
        std::sort(askPrices.begin(), askPrices.end());
        for (double p : askPrices) {
            int row = askTable->rowCount();
            askTable->insertRow(row);
            auto *priceItem = new QTableWidgetItem(QString::number(p, 'f', 2));
            priceItem->setForeground(Qt::red);
            askTable->setItem(row, 0, priceItem);
            askTable->setItem(row, 1, new QTableWidgetItem(QString::number(askLevels[p])));
        }
    };

    auto addTapeEntry = [=](const QString &side, double price, int size) {
        int row = tapeTable->rowCount();
        tapeTable->insertRow(row);
        tapeTable->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
        tapeTable->setItem(row, 1, new QTableWidgetItem(side));
        tapeTable->setItem(row, 2, new QTableWidgetItem(QString::number(price, 'f', 2)));
        tapeTable->setItem(row, 3, new QTableWidgetItem(QString::number(size)));
        tapeTable->scrollToBottom();
    };

    auto refreshMyOrders = [=]() {
        myOrdersTable->setRowCount(0);
        for (int i = 0; i < orderIds.size(); ++i) {
            int row = myOrdersTable->rowCount();
            myOrdersTable->insertRow(row);
            myOrdersTable->setItem(row, 0, new QTableWidgetItem(QString::number(orderIds[i])));
            myOrdersTable->setItem(row, 1, new QTableWidgetItem(orderIsBuy[i] ? "BUY" : "SELL"));
            myOrdersTable->setItem(row, 2, new QTableWidgetItem(QString::number(orderPrices[i], 'f', 2)));
            myOrdersTable->setItem(row, 3, new QTableWidgetItem(QString::number(orderSizes[i])));
        }
    };

    struct TradeQueue {
        std::mutex mtx;
        std::condition_variable cv;
        std::deque<Trade> q;
        void push(const Trade &t) {
            std::lock_guard<std::mutex> lk(mtx);
            q.push_back(t);
            cv.notify_one();
        }
        bool tryPop(Trade &t) {
            std::lock_guard<std::mutex> lk(mtx);
            if (q.empty()) return false;
            t = q.front();
            q.pop_front();
            return true;
        }
    };

    TradeQueue incomingTrades;
    std::atomic<bool> connected{false};
    std::atomic<bool> stopNetwork{false};

    asio::ip::tcp::socket *g_socket = nullptr;
    std::mutex writeMtx;

    auto networkBody = [&]() {
        asio::io_context io;
        asio::ip::tcp::socket sock(io);
        std::thread::id thisId = std::this_thread::get_id();
        (void)thisId;

        try {
            asio::ip::tcp::resolver resolver(io);
            auto endpoints = resolver.resolve("127.0.0.1", "12345");
            asio::connect(sock, endpoints);
            {
                std::lock_guard<std::mutex> lk(writeMtx);
                g_socket = &sock;
            }
            connected = true;

            std::array<char, 4096> buf;
            std::vector<char> rx;
            rx.reserve(4096);

            while (!stopNetwork.load()) {
                std::error_code ec;
                size_t n = sock.read_some(asio::buffer(buf), ec);
                if (ec) break;
                rx.insert(rx.end(), buf.begin(), buf.begin() + n);

                while (rx.size() >= sizeof(MessageTradeExecuted)) {
                    MessageType type = *reinterpret_cast<MessageType*>(rx.data());
                    if (type != MessageType::TradeExecuted) {
                        
                        rx.erase(rx.begin());
                        continue;
                    }
                    MessageTradeExecuted msg;
                    std::memcpy(&msg, rx.data(), sizeof(MessageTradeExecuted));
                    rx.erase(rx.begin(),
                             rx.begin() + sizeof(MessageTradeExecuted));
                    incomingTrades.push(msg.trade);
                }
            }
        } catch (const std::exception &ex) {
            std::cerr << "[Client] Network error: " << ex.what() << std::endl;
        }

        {
            std::lock_guard<std::mutex> lk(writeMtx);
            connected = false;
            g_socket = nullptr;
        }
    };

    std::thread netThread(networkBody);

    auto sendMessage = [&](const void *data, size_t len) {
        std::lock_guard<std::mutex> lk(writeMtx);
        if (!g_socket || !connected.load()) {
            QMessageBox::warning(&window, "Not connected", "Server is not connected.");
            return false;
        }
        std::error_code ec;
        asio::write(*g_socket, asio::buffer(data, len), ec);
        if (ec) {
            QMessageBox::warning(&window, "Send error", "Failed to send message.");
            return false;
        }
        return true;
    };

    auto applyTradeToMyOrders = [&](const Trade &t) {
        auto reduceOrder = [&](int id, double filledQty) {
            int idx = orderIds.indexOf(id);
            if (idx < 0) return;
            orderSizes[idx] -= filledQty;
            if (orderSizes[idx] <= 0) {
                orderIds.remove(idx);
                orderIsBuy.remove(idx);
                orderPrices.remove(idx);
                orderSizes.remove(idx);
            }
        };
        reduceOrder(t.buyOrderId, t.quantity);
        reduceOrder(t.sellOrderId, t.quantity);

        bidLevels.clear();
        askLevels.clear();
        for (int i = 0; i < orderIds.size(); ++i) {
            if (orderIsBuy[i]) bidLevels[orderPrices[i]] += orderSizes[i];
            else               askLevels[orderPrices[i]] += orderSizes[i];
        }
    };

    QTimer *tradeTimer = new QTimer(&window);
    QObject::connect(tradeTimer, &QTimer::timeout, [&]() {
        
        if (connected.load()) {
            if (statusLabel->text() != "Connected to server") {
                statusLabel->setText("Connected to server");
                statusLabel->setStyleSheet("color:green; font-weight:bold;");
            }
        } else {
            if (statusLabel->text() != "Disconnected") {
                statusLabel->setText("Disconnected");
                statusLabel->setStyleSheet("color:red; font-weight:bold;");
            }
        }
        Trade t;
        bool any = false;
        while (incomingTrades.tryPop(t)) {
            any = true;
            addTapeEntry("FILL", t.executionPrice, t.quantity);
            applyTradeToMyOrders(t);
        }
        if (any) {
            refreshBook();
            refreshMyOrders();
        }
    });
    tradeTimer->start(50);

    auto submitOrder = [&](bool isBuy) {
        bool okPrice, okSize;
        double price = priceInput->text().toDouble(&okPrice);
        double size = sizeInput->text().toDouble(&okSize);
        if (!okPrice || !okSize || price <= 0 || size <= 0) {
            QMessageBox::warning(&window, "Invalid input", "Enter a valid price and size.");
            return;
        }

        int id = nextOrderId++;

        MessageOrder msg;
        msg.type     = MessageType::AddOrder;
        msg.orderId  = id;
        msg.price    = price;
        msg.quantity = size;
        msg.side     = isBuy ? Side::BUY : Side::SELL;
        if (!sendMessage(&msg, sizeof(msg))) {
            return;
        }

        orderIds.push_back(id);
        orderIsBuy.push_back(isBuy);
        orderPrices.push_back(price);
        orderSizes.push_back(size);
        if (isBuy) bidLevels[price] += size;
        else       askLevels[price] += size;
        refreshBook();
        refreshMyOrders();

        priceInput->clear();
        sizeInput->clear();
    };

    QObject::connect(buyButton,  &QPushButton::clicked, [&]() { submitOrder(true);  });
    QObject::connect(sellButton, &QPushButton::clicked, [&]() { submitOrder(false); });

    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        bool ok;
        int idToCancel = cancelIdInput->text().toInt(&ok);
        if (!ok) {
            QMessageBox::warning(&window, "Invalid ID", "Enter a valid numeric order ID.");
            return;
        }
        int idx = orderIds.indexOf(idToCancel);
        if (idx == -1) {
            QMessageBox::information(&window, "Not found", "No open order with that ID.");
            return;
        }

        double price = orderPrices[idx];
        int    size  = orderSizes[idx];
        bool   isBuy = orderIsBuy[idx];

        MessageCancelOrder msg;
        msg.type    = MessageType::CancelOrder;
        msg.orderId = idToCancel;
        if (!sendMessage(&msg, sizeof(msg))) {
            return;
        }

        orderIds.remove(idx);
        orderIsBuy.remove(idx);
        orderPrices.remove(idx);
        orderSizes.remove(idx);

        auto &levels = isBuy ? bidLevels : askLevels;
        levels[price] -= size;
        if (levels[price] <= 0) levels.remove(price);
        refreshBook();
        refreshMyOrders();

        addTapeEntry(QString("CANCEL-") + (isBuy ? "BUY" : "SELL"), price, size);

        cancelIdInput->clear();
    });

    refreshBook();
    refreshMyOrders();

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        stopNetwork.store(true);
        {
            std::lock_guard<std::mutex> lk(writeMtx);
            if (g_socket) {
                std::error_code ec;
                g_socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                g_socket->close(ec);
            }
        }
        if (netThread.joinable()) netThread.join();
    });

    window.show();

    return app.exec();
}
