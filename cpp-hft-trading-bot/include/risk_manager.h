#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H

#include "data_structures.h"
#include "order_manager.h"
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>

namespace trading {

// Risk limits configuration
struct RiskLimits {
    double maxPositionSize;
    double maxLossPerTrade;
    double maxDailyLoss;
    double maxLeverage;
    uint64_t maxOrderSize;
    double maxExposure;
};

// Risk manager for position and P&L tracking
class RiskManager {
public:
    RiskManager(std::shared_ptr<OrderManager> orderManager);
    ~RiskManager();
    
    // Configuration
    void setRiskLimits(const RiskLimits& limits);
    RiskLimits getRiskLimits() const;
    
    // Position management
    void updatePosition(const Trade& trade);
    void closePosition(const std::string& symbol);
    
    Position getPosition(const std::string& symbol) const;
    std::vector<Position> getAllPositions() const;
    
    // P&L tracking
    double getRealizedPnL() const;
    double getUnrealizedPnL() const;
    double getTotalPnL() const;
    
    // Portfolio stats
    Portfolio getPortfolioSnapshot() const;
    double getMaxDrawdown() const;
    double getWinRate() const;
    
    // Risk checks
    bool canOpenPosition(const std::string& symbol, uint64_t quantity) const;
    bool canPlaceOrder(const Order& order) const;
    double calculateRequiredCapital(const Order& order) const;
    
    // Update market prices
    void updateMarketPrice(const std::string& symbol, double price);
    
private:
    std::shared_ptr<OrderManager> orderManager;
    RiskLimits limits;
    
    std::map<std::string, Position> positions;
    double totalRealizedPnL{0};
    double dailyLoss{0};
    
    std::vector<double> pnlHistory;
    double maxDrawdownValue{0};
    
    std::atomic<double> totalBalance{100000.0}; // Default starting balance
    
    mutable std::mutex positionMutex;
    mutable std::mutex pnlMutex;
    
    void recalculateMetrics();
};

} // namespace trading

#endif // RISK_MANAGER_H