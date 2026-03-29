#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <memory>

namespace trading {

// Enums for trading operations
enum class OrderSide { BUY = 1, SELL = -1 };
enum class OrderType { MARKET, LIMIT, STOP };
enum class OrderStatus { PENDING, FILLED, PARTIAL, CANCELLED, REJECTED };
enum class PositionStatus { OPEN, CLOSED };

// High-precision timestamp using nanoseconds
struct Timestamp {
    uint64_t nanoseconds;
    
    Timestamp(uint64_t ns = 0) : nanoseconds(ns) {}
    Timestamp(const std::time_t& seconds) : nanoseconds(seconds * 1e9) {}
    
    uint64_t toMicroseconds() const { return nanoseconds / 1000; }
    uint64_t toMilliseconds() const { return nanoseconds / 1e6; }
    double toSeconds() const { return nanoseconds / 1e9; }
};

// Market data tick
struct MarketTick {
    std::string symbol;
    double bid;
    double ask;
    uint64_t bidVolume;
    uint64_t askVolume;
    Timestamp timestamp;
    double lastPrice;
    uint64_t volume;
};

// Order structure
struct Order {
    uint64_t orderId;
    std::string symbol;
    OrderSide side;
    OrderType type;
    double price;
    uint64_t quantity;
    uint64_t filledQuantity;
    OrderStatus status;
    Timestamp createdAt;
    Timestamp filledAt;
    double commission;
    std::string clientOrderId;
};

// Trade execution record
struct Trade {
    uint64_t tradeId;
    uint64_t orderId;
    std::string symbol;
    OrderSide side;
    double price;
    uint64_t quantity;
    Timestamp executedAt;
    double totalCost;
};

// Position tracking
struct Position {
    std::string symbol;
    OrderSide side;
    uint64_t quantity;
    double avgEntryPrice;
    double currentPrice;
    PositionStatus status;
    Timestamp openedAt;
    Timestamp closedAt;
    double unrealizedPnL;
    double realizedPnL;
};

// Market depth/Level 2 data
struct OrderBook {
    std::string symbol;
    std::vector<std::pair<double, uint64_t>> bids;  // price, volume pairs
    std::vector<std::pair<double, uint64_t>> asks;
    Timestamp timestamp;
    
    double getMidPrice() const {
        if (bids.empty() || asks.empty()) return 0;
        return (bids[0].first + asks[0].first) / 2.0;
    }
    
    double getSpread() const {
        if (bids.empty() || asks.empty()) return 0;
        return asks[0].first - bids[0].first;
    }
};

// Portfolio/Account state
struct Portfolio {
    double totalBalance;
    double availableBalance;
    double totalPnL;
    double realizedPnL;
    double unrealizedPnL;
    std::vector<Position> positions;
    uint64_t totalTrades;
    double totalCommissions;
};

// Strategy signal
struct Signal {
    std::string symbol;
    OrderSide side;
    double confidence;
    std::string reason;
    Timestamp generatedAt;
};

// Performance metrics
struct PerformanceMetrics {
    uint64_t totalTrades;
    double totalReturn;
    double winRate;
    double sharpeRatio;
    double maxDrawdown;
    double averageTradeSize;
    double profitFactor;
    Timestamp timestamp;
};

} // namespace trading

#endif // DATA_STRUCTURES_H