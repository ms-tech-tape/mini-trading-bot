#include "order_manager.h"

namespace trading {

OrderManager::OrderManager() : orderIdCounter(1000), tradeIdCounter(1) {}

OrderManager::~OrderManager() {}

uint64_t OrderManager::placeOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(orderMutex);
    
    Order newOrder = order;
    newOrder.orderId = orderIdCounter++;
    newOrder.status = OrderStatus::PENDING;
    
    orders[newOrder.orderId] = newOrder;
    totalOrdersPlaced++;
    
    notifyOrderChanged(newOrder);
    return newOrder.orderId;
}

bool OrderManager::cancelOrder(uint64_t orderId) {
    std::lock_guard<std::mutex> lock(orderMutex);
    
    auto it = orders.find(orderId);
    if (it != orders.end() && it->second.status == OrderStatus::PENDING) {
        it->second.status = OrderStatus::CANCELLED;
        notifyOrderChanged(it->second);
        return true;
    }
    return false;
}

bool OrderManager::modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity) {
    std::lock_guard<std::mutex> lock(orderMutex);
    
    auto it = orders.find(orderId);
    if (it != orders.end() && it->second.status == OrderStatus::PENDING) {
        it->second.price = newPrice;
        it->second.quantity = newQuantity;
        notifyOrderChanged(it->second);
        return true;
    }
    return false;
}

Order OrderManager::getOrder(uint64_t orderId) const {
    std::lock_guard<std::mutex> lock(orderMutex);
    auto it = orders.find(orderId);
    if (it != orders.end()) {
        return it->second;
    }
    return Order();
}

std::vector<Order> OrderManager::getActiveOrders() const {
    std::lock_guard<std::mutex> lock(orderMutex);
    std::vector<Order> result;
    for (const auto& pair : orders) {
        if (pair.second.status == OrderStatus::PENDING || pair.second.status == OrderStatus::PARTIAL) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<Order> OrderManager::getOrderHistory() const {
    std::lock_guard<std::mutex> lock(orderMutex);
    std::vector<Order> result;
    for (const auto& pair : orders) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Order> OrderManager::getOrdersBySymbol(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(orderMutex);
    std::vector<Order> result;
    for (const auto& pair : orders) {
        if (pair.second.symbol == symbol) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void OrderManager::registerOrderCallback(OrderCallback callback) {
    callbacks.push_back(callback);
}

void OrderManager::simulateOrderExecution(uint64_t orderId, double executionPrice, uint64_t executedQuantity) {
    std::lock_guard<std::mutex> lock(orderMutex);
    
    auto it = orders.find(orderId);
    if (it != orders.end()) {
        it->second.filledQuantity = executedQuantity;
        it->second.status = (executedQuantity == it->second.quantity) ? OrderStatus::FILLED : OrderStatus::PARTIAL;
        notifyOrderChanged(it->second);
    }
}

void OrderManager::simulateTrade(const Trade& trade) {
    std::lock_guard<std::mutex> lock(tradeMutex);
    trades.push_back(trade);
    totalTradesExecuted++;
}

std::vector<Trade> OrderManager::getTradeHistory() const {
    std::lock_guard<std::mutex> lock(tradeMutex);
    return trades;
}

Trade OrderManager::getTrade(uint64_t tradeId) const {
    std::lock_guard<std::mutex> lock(tradeMutex);
    for (const auto& trade : trades) {
        if (trade.tradeId == tradeId) {
            return trade;
        }
    }
    return Trade();
}

void OrderManager::notifyOrderChanged(const Order& order) {
    for (auto& callback : callbacks) {
        callback(order);
    }
}

} // namespace trading