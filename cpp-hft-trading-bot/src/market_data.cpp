#include "market_data.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace trading {

MarketDataHandler::MarketDataHandler() {}

MarketDataHandler::~MarketDataHandler() {}

void MarketDataHandler::subscribe(const std::string& symbol, std::shared_ptr<IMarketDataListener> listener) {
    std::lock_guard<std::mutex> lock(dataMutex);
    symbolDataMap[symbol].listeners.push_back(listener);
}

void MarketDataHandler::unsubscribe(const std::string& symbol, std::shared_ptr<IMarketDataListener> listener) {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = symbolDataMap.find(symbol);
    if (it != symbolDataMap.end()) {
        auto& listeners = it->second.listeners;
        listeners.erase(std::remove_if(listeners.begin(), listeners.end(),
            [listener](const std::weak_ptr<IMarketDataListener>& l) {
                return l.expired() || l.lock() == listener;
            }), listeners.end());
    }
}

void MarketDataHandler::publishTick(const MarketTick& tick) {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = symbolDataMap.find(tick.symbol);
    if (it != symbolDataMap.end()) {
        it->second.tickHistory.push_back(tick);
        if (it->second.tickHistory.size() > MAX_HISTORY_SIZE) {
            it->second.tickHistory.pop_front();
        }
        
        for (auto& listener : it->second.listeners) {
            if (auto shared = listener.lock()) {
                shared->onTick(tick);
            }
        }
    }
}

void MarketDataHandler::publishOrderBook(const OrderBook& book) {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = symbolDataMap.find(book.symbol);
    if (it != symbolDataMap.end()) {
        it->second.orderBookHistory.push_back(book);
        if (it->second.orderBookHistory.size() > MAX_HISTORY_SIZE) {
            it->second.orderBookHistory.pop_front();
        }
        
        for (auto& listener : it->second.listeners) {
            if (auto shared = listener.lock()) {
                shared->onOrderBook(book);
            }
        }
    }
}

MarketTick MarketDataHandler::getLatestTick(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = symbolDataMap.find(symbol);
    if (it != symbolDataMap.end() && !it->second.tickHistory.empty()) {
        return it->second.tickHistory.back();
    }
    return MarketTick();
}

OrderBook MarketDataHandler::getLatestOrderBook(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = symbolDataMap.find(symbol);
    if (it != symbolDataMap.end() && !it->second.orderBookHistory.empty()) {
        return it->second.orderBookHistory.back();
    }
    return OrderBook();
}

std::vector<MarketTick> MarketDataHandler::getTickHistory(const std::string& symbol, size_t limit) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::vector<MarketTick> result;
    auto it = symbolDataMap.find(symbol);
    if (it != symbolDataMap.end()) {
        size_t start = it->second.tickHistory.size() > limit ? it->second.tickHistory.size() - limit : 0;
        result.assign(it->second.tickHistory.begin() + start, it->second.tickHistory.end());
    }
    return result;
}

double MarketDataHandler::calculateVolatility(const std::string& symbol, size_t window) const {
    auto ticks = getTickHistory(symbol, window);
    if (ticks.size() < 2) return 0.0;
    
    double mean = 0.0;
    for (const auto& tick : ticks) mean += tick.lastPrice;
    mean /= ticks.size();
    
    double variance = 0.0;
    for (const auto& tick : ticks) {
        variance += (tick.lastPrice - mean) * (tick.lastPrice - mean);
    }
    variance /= ticks.size();
    
    return std::sqrt(variance);
}

double MarketDataHandler::calculateMomentum(const std::string& symbol, size_t period) const {
    auto ticks = getTickHistory(symbol, period);
    if (ticks.size() < 2) return 0.0;
    return (ticks.back().lastPrice - ticks.front().lastPrice) / ticks.front().lastPrice * 100.0;
}

double MarketDataHandler::calculateRSI(const std::string& symbol, size_t period) const {
    auto ticks = getTickHistory(symbol, period + 1);
    if (ticks.size() < period + 1) return 50.0;
    
    double gains = 0.0, losses = 0.0;
    for (size_t i = 1; i < ticks.size(); ++i) {
        double change = ticks[i].lastPrice - ticks[i-1].lastPrice;
        if (change > 0) gains += change;
        else losses += -change;
    }
    
    double avgGain = gains / period;
    double avgLoss = losses / period;
    
    if (avgLoss == 0) return 100.0;
    double rs = avgGain / avgLoss;
    return 100.0 - (100.0 / (1.0 + rs));
}

} // namespace trading