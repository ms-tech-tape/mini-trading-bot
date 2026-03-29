#include "risk_manager.h"
#include <algorithm>
#include <cmath>

namespace trading {

RiskManager::RiskManager(std::shared_ptr<OrderManager> om) : orderManager(om) {
    limits.maxPositionSize = 10000;
    limits.maxLossPerTrade = 500.0;
    limits.maxDailyLoss = 2000.0;
    limits.maxLeverage = 2.0;
    limits.maxOrderSize = 1000;
    limits.maxExposure = 50000.0;
}

RiskManager::~RiskManager() {}

void RiskManager::setRiskLimits(const RiskLimits& l) {
    limits = l;
}

RiskLimits RiskManager::getRiskLimits() const {
    return limits;
}

void RiskManager::updatePosition(const Trade& trade) {
    std::lock_guard<std::mutex> lock(positionMutex);
    
    auto it = positions.find(trade.symbol);
    if (it == positions.end()) {
        Position pos;
        pos.symbol = trade.symbol;
        pos.side = trade.side;
        pos.quantity = trade.quantity;
        pos.avgEntryPrice = trade.price;
        pos.status = PositionStatus::OPEN;
        positions[trade.symbol] = pos;
    } else {
        it->second.quantity += trade.quantity;
    }
}

void RiskManager::closePosition(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(positionMutex);
    auto it = positions.find(symbol);
    if (it != positions.end()) {
        it->second.status = PositionStatus::CLOSED;
    }
}

Position RiskManager::getPosition(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(positionMutex);
    auto it = positions.find(symbol);
    if (it != positions.end()) {
        return it->second;
    }
    return Position();
}

std::vector<Position> RiskManager::getAllPositions() const {
    std::lock_guard<std::mutex> lock(positionMutex);
    std::vector<Position> result;
    for (const auto& pair : positions) {
        result.push_back(pair.second);
    }
    return result;
}

double RiskManager::getRealizedPnL() const {
    std::lock_guard<std::mutex> lock(pnlMutex);
    return totalRealizedPnL;
}

double RiskManager::getUnrealizedPnL() const {
    std::lock_guard<std::mutex> lock(positionMutex);
    double unrealized = 0.0;
    for (const auto& pair : positions) {
        if (pair.second.status == PositionStatus::OPEN) {
            unrealized += pair.second.unrealizedPnL;
        }
    }
    return unrealized;
}

double RiskManager::getTotalPnL() const {
    return getRealizedPnL() + getUnrealizedPnL();
}

Portfolio RiskManager::getPortfolioSnapshot() const {
    Portfolio port;
    port.totalBalance = totalBalance;
    port.availableBalance = totalBalance - getUnrealizedPnL();
    port.totalPnL = getTotalPnL();
    port.realizedPnL = getRealizedPnL();
    port.unrealizedPnL = getUnrealizedPnL();
    port.positions = getAllPositions();
    port.totalTrades = orderManager->getTotalTradesExecuted();
    return port;
}

double RiskManager::getMaxDrawdown() const {
    std::lock_guard<std::mutex> lock(pnlMutex);
    return maxDrawdownValue;
}

double RiskManager::getWinRate() const {
    auto trades = orderManager->getTradeHistory();
    if (trades.empty()) return 0.0;
    
    int winCount = 0;
    for (const auto& trade : trades) {
        if (trade.price > 0) winCount++;
    }
    return (static_cast<double>(winCount) / trades.size()) * 100.0;
}

bool RiskManager::canOpenPosition(const std::string& symbol, uint64_t quantity) const {
    if (quantity > limits.maxPositionSize) return false;
    auto pos = getPosition(symbol);
    if (pos.quantity + quantity > limits.maxPositionSize) return false;
    return true;
}

bool RiskManager::canPlaceOrder(const Order& order) const {
    if (order.quantity > limits.maxOrderSize) return false;
    return true;
}

double RiskManager::calculateRequiredCapital(const Order& order) const {
    return order.price * order.quantity / limits.maxLeverage;
}

void RiskManager::updateMarketPrice(const std::string& symbol, double price) {
    std::lock_guard<std::mutex> lock(positionMutex);
    auto it = positions.find(symbol);
    if (it != positions.end()) {
        it->second.currentPrice = price;
        if (it->second.side == OrderSide::BUY) {
            it->second.unrealizedPnL = (price - it->second.avgEntryPrice) * it->second.quantity;
        } else {
            it->second.unrealizedPnL = (it->second.avgEntryPrice - price) * it->second.quantity;
        }
    }
}

void RiskManager::recalculateMetrics() {}

} // namespace trading