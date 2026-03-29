#include "data_structures.h"
#include "market_data.h"
#include "order_manager.h"
#include "execution_engine.h"
#include "trading_strategy.h"
#include "risk_manager.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>

using namespace trading;

// Simple market data simulator
class MarketSimulator {
public:
    MarketSimulator(std::shared_ptr<MarketDataHandler> mktData) 
        : marketData(mktData), running(false), basePrice(100.0) {}
    
    void start() {
        running = true;
        simulatorThread = std::thread(&MarketSimulator::simulateMarketData, this);
    }
    
    void stop() {
        running = false;
        if (simulatorThread.joinable()) {
            simulatorThread.join();
        }
    }
    
private:
    std::shared_ptr<MarketDataHandler> marketData;
    std::thread simulatorThread;
    std::atomic<bool> running;
    double basePrice;
    
    void simulateMarketData() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> priceChange(-0.1, 0.1);
        std::uniform_int_distribution<uint64_t> volumeDist(100, 10000);
        
        int tickCount = 0;
        while (running && tickCount < 1000) {
            // Simulate price movement
            basePrice += priceChange(gen);
            basePrice = std::max(basePrice, 50.0); // Prevent negative
            
            MarketTick tick;
            tick.symbol = "AAPL";
            tick.lastPrice = basePrice;
            tick.bid = basePrice - 0.01;
            tick.ask = basePrice + 0.01;
            tick.bidVolume = volumeDist(gen);
            tick.askVolume = volumeDist(gen);
            tick.volume = tick.bidVolume + tick.askVolume;
            tick.timestamp = Timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            
            marketData->publishTick(tick);
            
            // Simulate order book
            OrderBook book;
            book.symbol = "AAPL";
            book.bids.push_back({basePrice - 0.01, volumeDist(gen)});
            book.bids.push_back({basePrice - 0.02, volumeDist(gen)});
            book.asks.push_back({basePrice + 0.01, volumeDist(gen)});
            book.asks.push_back({basePrice + 0.02, volumeDist(gen)});
            book.timestamp = tick.timestamp;
            
            marketData->publishOrderBook(book);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            tickCount++;
        }
    }
};

// Demo trading strategy listener
class DemoStrategyListener : public IMarketDataListener {
public:
    DemoStrategyListener(std::shared_ptr<OrderManager> om, std::shared_ptr<ExecutionEngine> ee)
        : orderManager(om), executionEngine(ee), tradeCount(0) {}
    
    void onTick(const MarketTick& tick) override {
        // Generate simple trading signal
        static double lastPrice = 0;
        
        if (lastPrice > 0 && tradeCount < 5) {
            double priceDelta = tick.lastPrice - lastPrice;
            
            if (priceDelta > 0.1) {
                // Buy signal
                Order order;
                order.orderId = 0;
                order.symbol = tick.symbol;
                order.side = OrderSide::BUY;
                order.type = OrderType::LIMIT;
                order.price = tick.bid;
                order.quantity = 100;
                order.filledQuantity = 0;
                order.status = OrderStatus::PENDING;
                order.commission = 0.0;
                
                uint64_t orderId = orderManager->placeOrder(order);
                
                ExecutionRequest execReq;
                execReq.orderId = orderId;
                execReq.symbol = order.symbol;
                execReq.side = order.side;
                execReq.price = order.price;
                execReq.quantity = order.quantity;
                execReq.priority = ExecutionPriority::HIGH;
                execReq.timestamp = tick.timestamp;
                
                executionEngine->submitOrder(execReq);
                
                std::cout << "[SIGNAL] BUY signal for " << tick.symbol 
                          << " at price: " << std::fixed << std::setprecision(4) 
                          << tick.lastPrice << std::endl;
                tradeCount++;
            }
            else if (priceDelta < -0.1) {
                // Sell signal
                Order order;
                order.orderId = 0;
                order.symbol = tick.symbol;
                order.side = OrderSide::SELL;
                order.type = OrderType::LIMIT;
                order.price = tick.ask;
                order.quantity = 100;
                order.filledQuantity = 0;
                order.status = OrderStatus::PENDING;
                order.commission = 0.0;
                
                uint64_t orderId = orderManager->placeOrder(order);
                
                ExecutionRequest execReq;
                execReq.orderId = orderId;
                execReq.symbol = order.symbol;
                execReq.side = order.side;
                execReq.price = order.price;
                execReq.quantity = order.quantity;
                execReq.priority = ExecutionPriority::HIGH;
                execReq.timestamp = tick.timestamp;
                
                executionEngine->submitOrder(execReq);
                
                std::cout << "[SIGNAL] SELL signal for " << tick.symbol 
                          << " at price: " << std::fixed << std::setprecision(4) 
                          << tick.lastPrice << std::endl;
                tradeCount++;
            }
        }
        
        lastPrice = tick.lastPrice;
    }
    
    void onOrderBook(const OrderBook& book) override {
        // Process order book data if needed
    }
    
private:
    std::shared_ptr<OrderManager> orderManager;
    std::shared_ptr<ExecutionEngine> executionEngine;
    int tradeCount;
};

// Main application
int main() {
    std::cout << "=== C++ HFT Mini Trading Bot ===" << std::endl;
    std::cout << "Starting trading system initialization..." << std::endl;
    
    // Initialize core components
    auto marketData = std::make_shared<MarketDataHandler>();
    auto orderManager = std::make_shared<OrderManager>();
    auto executionEngine = std::make_shared<ExecutionEngine>(orderManager, marketData);
    auto riskManager = std::make_shared<RiskManager>(orderManager);
    
    // Configure risk limits
    RiskLimits limits;
    limits.maxPositionSize = 10000;
    limits.maxLossPerTrade = 500.0;
    limits.maxDailyLoss = 2000.0;
    limits.maxLeverage = 2.0;
    limits.maxOrderSize = 1000;
    limits.maxExposure = 50000.0;
    riskManager->setRiskLimits(limits);
    
    std::cout << "[OK] Core components initialized" << std::endl;
    
    // Start execution engine
    executionEngine->start();
    std::cout << "[OK] Execution engine started" << std::endl;
    
    // Create demo strategy listener
    auto strategyListener = std::make_shared<DemoStrategyListener>(orderManager, executionEngine);
    marketData->subscribe("AAPL", strategyListener);
    std::cout << "[OK] Strategy listener registered" << std::endl;
    
    // Start market simulator
    MarketSimulator simulator(marketData);
    simulator.start();
    std::cout << "[OK] Market simulator started" << std::endl;
    
    // Let the system run for a while
    std::this_thread::sleep_for(std::chrono::seconds(15));
    
    // Print statistics
    std::cout << "\n=== Trading Statistics ===" << std::endl;
    std::cout << "Total Orders Placed: " << orderManager->getTotalOrdersPlaced() << std::endl;
    std::cout << "Total Trades Executed: " << orderManager->getTotalTradesExecuted() << std::endl;
    std::cout << "Average Execution Latency: " << std::fixed << std::setprecision(2) 
              << executionEngine->getAverageLatency() << " μs" << std::endl;
    
    auto trades = orderManager->getTradeHistory();
    if (!trades.empty()) {
        std::cout << "Recent Trades: " << std::endl;
        for (size_t i = 0; i < std::min(size_t(5), trades.size()); i++) {
            const auto& trade = trades[i];
            std::cout << "  - " << (trade.side == OrderSide::BUY ? "BUY" : "SELL")
                      << " " << trade.quantity << " @ $" << std::fixed << std::setprecision(4)
                      << trade.price << std::endl;
        }
    }
    
    // Portfolio snapshot
    auto portfolio = riskManager->getPortfolioSnapshot();
    std::cout << "\n=== Portfolio Snapshot ===" << std::endl;
    std::cout << "Total Balance: $" << std::fixed << std::setprecision(2) 
              << portfolio.totalBalance << std::endl;
    std::cout << "Total PnL: $" << portfolio.totalPnL << std::endl;
    std::cout << "Realized PnL: $" << portfolio.realizedPnL << std::endl;
    std::cout << "Unrealized PnL: $" << portfolio.unrealizedPnL << std::endl;
    
    // Cleanup
    simulator.stop();
    executionEngine->stop();
    
    std::cout << "\n[OK] Trading bot shutdown complete" << std::endl;
    
    return 0;
}