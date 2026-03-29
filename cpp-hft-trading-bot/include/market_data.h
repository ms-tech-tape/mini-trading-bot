#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include "data_structures.h"
#include <vector>
#include <map>
#include <deque>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

namespace trading {

// Market data subscriber interface
class IMarketDataListener {
public:
    virtual ~IMarketDataListener() = default;
    virtual void onTick(const MarketTick& tick) = 0;
    virtual void onOrderBook(const OrderBook& book) = 0;
};

// High-speed market data handler
class MarketDataHandler {
public:
    MarketDataHandler();
    ~MarketDataHandler();
    
    // Subscribe to market data
    void subscribe(const std::string& symbol, std::shared_ptr<IMarketDataListener> listener);
    void unsubscribe(const std::string& symbol, std::shared_ptr<IMarketDataListener> listener);
    
    // Simulate market data ingestion
    void publishTick(const MarketTick& tick);
    void publishOrderBook(const OrderBook& book);
    
    // Get latest data
    MarketTick getLatestTick(const std::string& symbol) const;
    OrderBook getLatestOrderBook(const std::string& symbol) const;
    
    // Historical data
    std::vector<MarketTick> getTickHistory(const std::string& symbol, size_t limit = 100) const;
    
    // Statistical analysis
    double calculateVolatility(const std::string& symbol, size_t window = 20) const;
    double calculateMomentum(const std::string& symbol, size_t period = 10) const;
    double calculateRSI(const std::string& symbol, size_t period = 14) const;
    
private:
    struct SymbolData {
        std::deque<MarketTick> tickHistory;
        std::deque<OrderBook> orderBookHistory;
        std::vector<std::weak_ptr<IMarketDataListener>> listeners;
    };
    
    std::map<std::string, SymbolData> symbolDataMap;
    mutable std::mutex dataMutex;
    static constexpr size_t MAX_HISTORY_SIZE = 10000;
};

} // namespace trading

#endif // MARKET_DATA_H