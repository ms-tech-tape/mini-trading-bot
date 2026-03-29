#include "trading_strategy.h"
#include <cmath>
#include <algorithm>

namespace trading {

// MeanReversionStrategy
MeanReversionStrategy::MeanReversionStrategy(const std::string& sym, size_t p, double t)
    : symbol(sym), period(p), threshold(t) {}

void MeanReversionStrategy::initialize() {}

Signal MeanReversionStrategy::generateSignal(const MarketTick& tick) {
    Signal signal;
    signal.symbol = tick.symbol;
    signal.generatedAt = tick.timestamp;
    return signal;
}

void MeanReversionStrategy::onTick(const MarketTick& tick) {
    prices.push_back(tick.lastPrice);
}

void MeanReversionStrategy::onOrderBook(const OrderBook& book) {}

double MeanReversionStrategy::calculateSMA() const {
    if (prices.empty()) return 0.0;
    double sum = 0.0;
    for (double price : prices) sum += price;
    return sum / prices.size();
}

double MeanReversionStrategy::calculateStdDev() const {
    if (prices.empty()) return 0.0;
    double mean = calculateSMA();
    double variance = 0.0;
    for (double price : prices) {
        variance += (price - mean) * (price - mean);
    }
    return std::sqrt(variance / prices.size());
}

// MomentumStrategy
MomentumStrategy::MomentumStrategy(const std::string& sym, size_t l, double s)
    : symbol(sym), lookback(l), sensitivity(s) {}

void MomentumStrategy::initialize() {}

Signal MomentumStrategy::generateSignal(const MarketTick& tick) {
    Signal signal;
    signal.symbol = tick.symbol;
    signal.generatedAt = tick.timestamp;
    return signal;
}

void MomentumStrategy::onTick(const MarketTick& tick) {
    prices.push_back(tick.lastPrice);
}

void MomentumStrategy::onOrderBook(const OrderBook& book) {}

double MomentumStrategy::calculateMomentum() const {
    if (prices.size() < 2) return 0.0;
    return ((prices.back() - prices.front()) / prices.front()) * 100.0;
}

// SpreadTradingStrategy
SpreadTradingStrategy::SpreadTradingStrategy(const std::string& s1, const std::string& s2, double t)
    : symbol1(s1), symbol2(s2), spreadThreshold(t) {}

void SpreadTradingStrategy::initialize() {}

Signal SpreadTradingStrategy::generateSignal(const MarketTick& tick) {
    Signal signal;
    signal.symbol = tick.symbol;
    signal.generatedAt = tick.timestamp;
    return signal;
}

void SpreadTradingStrategy::onTick(const MarketTick& tick) {}
void SpreadTradingStrategy::onOrderBook(const OrderBook& book) {}

// MarketMakingStrategy
MarketMakingStrategy::MarketMakingStrategy(const std::string& sym, double w, uint64_t qs)
    : symbol(sym), spreadWidth(w), quoteSize(qs) {}

void MarketMakingStrategy::initialize() {}

Signal MarketMakingStrategy::generateSignal(const MarketTick& tick) {
    Signal signal;
    signal.symbol = tick.symbol;
    signal.generatedAt = tick.timestamp;
    return signal;
}

void MarketMakingStrategy::onTick(const MarketTick& tick) {
    lastTick = tick;
}

void MarketMakingStrategy::onOrderBook(const OrderBook& book) {}

} // namespace trading