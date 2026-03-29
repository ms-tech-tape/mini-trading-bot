#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include "data_structures.h"
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

namespace trading {

// Order lifecycle callback
using OrderCallback = std::function<void(const Order&)>;

// Order management system
class OrderManager {
public:
    OrderManager();
    ~OrderManager();
    
    // Place orders
    uint64_t placeOrder(const Order& order);
    bool cancelOrder(uint64_t orderId);
    bool modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity);
    
    // Order tracking
    Order getOrder(uint64_t orderId) const;
    std::vector<Order> getActiveOrders() const;
    std::vector<Order> getOrderHistory() const;
    std::vector<Order> getOrdersBySymbol(const std::string& symbol) const;
    
    // Order execution callback
    void registerOrderCallback(OrderCallback callback);
    void simulateOrderExecution(uint64_t orderId, double executionPrice, uint64_t executedQuantity);
    void simulateTrade(const Trade& trade);
    
    // Statistics
    uint64_t getTotalOrdersPlaced() const { return totalOrdersPlaced; }
    uint64_t getTotalTradesExecuted() const { return totalTradesExecuted; }
    
    // Trade history
    std::vector<Trade> getTradeHistory() const;
    Trade getTrade(uint64_t tradeId) const;
    
private:
    uint64_t orderIdCounter;
    uint64_t tradeIdCounter;
    std::map<uint64_t, Order> orders;
    std::vector<Trade> trades;
    std::vector<OrderCallback> callbacks;
    
    std::atomic<uint64_t> totalOrdersPlaced{0};
    std::atomic<uint64_t> totalTradesExecuted{0};
    
    mutable std::mutex orderMutex;
    mutable std::mutex tradeMutex;
    
    void notifyOrderChanged(const Order& order);
};

} // namespace trading

#endif // ORDER_MANAGER_H