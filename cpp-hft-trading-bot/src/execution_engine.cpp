#include "execution_engine.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

namespace trading {

ExecutionEngine::ExecutionEngine(std::shared_ptr<OrderManager> om, 
                                 std::shared_ptr<MarketDataHandler> md)
    : orderManager(om), marketData(md) {
    stats.totalExecuted = 0;
    stats.totalLatency = 0;
    stats.maxLatency = 0.0;
    stats.minLatency = std::numeric_limits<double>::max();
}

ExecutionEngine::~ExecutionEngine() {
    stop();
}

void ExecutionEngine::submitOrder(const ExecutionRequest& request) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        executionQueue.push(request);
    }
    queueCV.notify_one();
}

void ExecutionEngine::start() {
    if (running.exchange(true)) {
        return;
    }
    executionThread = std::thread(&ExecutionEngine::executionLoop, this);
}

void ExecutionEngine::stop() {
    running = false;
    queueCV.notify_all();
    if (executionThread.joinable()) {
        executionThread.join();
    }
}

void ExecutionEngine::executionLoop() {
    while (running) {
        ExecutionRequest request;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { return !executionQueue.empty() || !running; });
            if (!running && executionQueue.empty()) break;
            if (executionQueue.empty()) continue;
            
            request = executionQueue.top();
            executionQueue.pop();
        }
        executeOrder(request);
    }
}

void ExecutionEngine::executeOrder(const ExecutionRequest& request) {
    auto submissionTime = std::chrono::high_resolution_clock::now();
    MarketTick latestTick = marketData->getLatestTick(request.symbol);
    double executionPrice = request.price;
    
    if (request.side == OrderSide::BUY) {
        executionPrice = std::min(request.price, latestTick.ask);
    } else {
        executionPrice = std::max(request.price, latestTick.bid);
    }
    
    uint64_t executedQuantity = request.quantity;
    if (executedQuantity > 0) {
        orderManager->simulateOrderExecution(request.orderId, executionPrice, executedQuantity);
        Trade trade;
        trade.tradeId = request.orderId;
        trade.orderId = request.orderId;
        trade.symbol = request.symbol;
        trade.side = request.side;
        trade.price = executionPrice;
        trade.quantity = executedQuantity;
        trade.executedAt = Timestamp(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        trade.totalCost = executionPrice * executedQuantity;
        orderManager->simulateTrade(trade);
    }
    
    auto executionTime = std::chrono::high_resolution_clock::now();
    auto latencyDuration = executionTime - submissionTime;
    uint64_t latencyMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(latencyDuration).count();
    recordLatency(latencyMicroseconds);
}

void ExecutionEngine::recordLatency(uint64_t latency) {
    std::lock_guard<std::mutex> lock(statsMutex);
    stats.totalExecuted++;
    stats.totalLatency += latency;
    double latencyDouble = static_cast<double>(latency);
    if (latencyDouble > stats.maxLatency) stats.maxLatency = latencyDouble;
    if (latencyDouble < stats.minLatency) stats.minLatency = latencyDouble;
}

double ExecutionEngine::getAverageLatency() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    if (stats.totalExecuted == 0) return 0.0;
    return static_cast<double>(stats.totalLatency) / stats.totalExecuted;
}

uint64_t ExecutionEngine::getExecutedOrdersCount() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return stats.totalExecuted;
}

double ExecutionEngine::getMaxLatency() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return stats.maxLatency;
}

double ExecutionEngine::getMinLatency() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    if (stats.minLatency == std::numeric_limits<double>::max()) return 0.0;
    return stats.minLatency;
}

} // namespace trading