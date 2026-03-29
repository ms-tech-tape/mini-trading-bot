#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include "data_structures.h"
#include "order_manager.h"
#include "market_data.h"
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <limits>

namespace trading {

// Execution priority levels
enum class ExecutionPriority { HIGH = 0, NORMAL = 1, LOW = 2 };

// Execution request
struct ExecutionRequest {
    uint64_t orderId;
    std::string symbol;
    OrderSide side;
    double price;
    uint64_t quantity;
    ExecutionPriority priority;
    Timestamp timestamp;
    
    // Comparator for priority queue (lower priority value = higher priority)
    bool operator>(const ExecutionRequest& other) const {
        return static_cast<int>(priority) > static_cast<int>(other.priority);
    }
};

// Fast order execution engine optimized for HFT
class ExecutionEngine {
public:
    ExecutionEngine(std::shared_ptr<OrderManager> orderManager, 
                   std::shared_ptr<MarketDataHandler> marketData);
    ~ExecutionEngine();
    
    // Submit order for execution
    void submitOrder(const ExecutionRequest& request);
    
    // Start/stop execution
    void start();
    void stop();
    
    // Statistics
    double getAverageLatency() const;
    uint64_t getExecutedOrdersCount() const;
    double getMaxLatency() const;
    double getMinLatency() const;
    
private:
    struct ExecutionStats {
        uint64_t totalExecuted;
        uint64_t totalLatency; // in microseconds
        double maxLatency;
        double minLatency;
    };
    
    std::shared_ptr<OrderManager> orderManager;
    std::shared_ptr<MarketDataHandler> marketData;
    
    std::priority_queue<ExecutionRequest, 
                       std::vector<ExecutionRequest>, 
                       std::greater<ExecutionRequest>> executionQueue;
    
    std::thread executionThread;
    std::atomic<bool> running{false};
    
    std::mutex queueMutex;
    std::condition_variable queueCV;
    
    ExecutionStats stats;
    mutable std::mutex statsMutex;
    
    void executionLoop();
    void executeOrder(const ExecutionRequest& request);
    void recordLatency(uint64_t latency);
};

} // namespace trading

#endif // EXECUTION_ENGINE_H