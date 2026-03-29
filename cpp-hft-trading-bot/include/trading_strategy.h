#ifndef TRADING_STRATEGY_H
#define TRADING_STRATEGY_H

#include "data_structures.h"
#include "market_data.h"
#include <vector>
#include <memory>

namespace trading {

// Base trading strategy interface
class TradingStrategy : public IMarketDataListener {
public:
    virtual ~TradingStrategy() = default;
    
    virtual void initialize() = 0;
    virtual Signal generateSignal(const MarketTick& tick) = 0;
    virtual void onTick(const MarketTick& tick) override = 0;
    virtual void onOrderBook(const OrderBook& book) override = 0;
    virtual std::string getName() const = 0;
};

// Mean Reversion Strategy
class MeanReversionStrategy : public TradingStrategy {
public:
    MeanReversionStrategy(const std::string& symbol, size_t period = 20, double threshold = 2.0);
    
    void initialize() override;
    Signal generateSignal(const MarketTick& tick) override;
    void onTick(const MarketTick& tick) override;
    void onOrderBook(const OrderBook& book) override;
    std::string getName() const override { return "MeanReversion"; }
    
private:
    std::string symbol;
    size_t period;
    double threshold;
    std::vector<double> prices;
    
    double calculateSMA() const;
    double calculateStdDev() const;
};

// Momentum Strategy
class MomentumStrategy : public TradingStrategy {
public:
    MomentumStrategy(const std::string& symbol, size_t lookback = 10, double sensitivity = 0.5);
    
    void initialize() override;
    Signal generateSignal(const MarketTick& tick) override;
    void onTick(const MarketTick& tick) override;
    void onOrderBook(const OrderBook& book) override;
    std::string getName() const override { return "Momentum"; }
    
private:
    std::string symbol;
    size_t lookback;
    double sensitivity;
    std::vector<double> prices;
    
    double calculateMomentum() const;
};

// Spread Trading Strategy (Arbitrage)
class SpreadTradingStrategy : public TradingStrategy {
public:
    SpreadTradingStrategy(const std::string& symbol1, const std::string& symbol2, 
                         double spreadThreshold = 0.5);
    
    void initialize() override;
    Signal generateSignal(const MarketTick& tick) override;
    void onTick(const MarketTick& tick) override;
    void onOrderBook(const OrderBook& book) override;
    std::string getName() const override { return "SpreadTrading"; }
    
private:
    std::string symbol1, symbol2;
    double spreadThreshold;
    MarketTick lastTick1, lastTick2;
};

// Market Making Strategy
class MarketMakingStrategy : public TradingStrategy {
public:
    MarketMakingStrategy(const std::string& symbol, double spreadWidth = 0.1, uint64_t quoteSize = 100);
    
    void initialize() override;
    Signal generateSignal(const MarketTick& tick) override;
    void onTick(const MarketTick& tick) override;
    void onOrderBook(const OrderBook& book) override;
    std::string getName() const override { return "MarketMaking"; }
    
private:
    std::string symbol;
    double spreadWidth;
    uint64_t quoteSize;
    MarketTick lastTick;
};

} // namespace trading

#endif // TRADING_STRATEGY_H