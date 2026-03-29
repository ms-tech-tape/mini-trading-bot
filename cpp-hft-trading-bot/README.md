# C++ HFT Mini Trading Bot

A professional-grade, high-frequency trading (HFT) simulation system written in modern C++17. This project demonstrates core trading system architecture patterns, real-time market data processing, order management, and execution optimization.

## Features

### Core Components

1. **Market Data Handler** (`market_data.h`)
   - Real-time tick and order book processing
   - Observer pattern for market data subscriptions
   - Historical data buffering with configurable limits
   - Technical indicator calculations (volatility, momentum, RSI)

2. **Order Manager** (`order_manager.h`)
   - Order lifecycle management (placement, modification, cancellation)
   - Trade execution and settlement tracking
   - Order history and performance metrics
   - Callback system for order state changes

3. **Execution Engine** (`execution_engine.h`)
   - Priority-based order execution queue
   - Latency tracking and optimization
   - Threaded execution loop for sub-millisecond response
   - Performance metrics collection

4. **Risk Manager** (`risk_manager.h`)
   - Position tracking and P&L calculation
   - Risk limit enforcement
   - Portfolio metrics (max drawdown, win rate, Sharpe ratio)
   - Real-time exposure monitoring

5. **Trading Strategies** (`trading_strategy.h`)
   - Mean Reversion Strategy
   - Momentum Strategy
   - Spread Trading (Arbitrage)
   - Market Making Strategy
   - Extensible base class for custom strategies

6. **Data Structures** (`data_structures.h`)
   - High-precision timestamps (nanosecond resolution)
   - Optimized market tick and order book representations
   - Position and portfolio snapshots
   - Performance metrics tracking

## Architecture

```
┌─────────────────────────────────────────────────────┐
│         Market Data (Real-time Ticks)                │
└─────────────────────┬───────────────────────────────┘
                      │
        ┌─────────────┴──────────────┐
        ▼                            ▼
┌──────────────────┐      ┌──────────────────────┐
│ Trading Strategy │      │ Market Data Handler  │
│ (Buy/Sell Logic) │      │ (Subscriptions)      │
└────────┬─────────┘      └──────────────────────┘
         │
         ▼
┌──────────────────────┐
│  Execution Engine    │
│ (Priority Queue)     │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Order Manager       │
│  (Order Lifecycle)   │
└────────┬─────────────┘
         │
         ▼
┌──────────────────────┐
│  Risk Manager        │
│  (Position/P&L)      │
└───────��──────────────┘
```

## Building

### Prerequisites
- C++17 compatible compiler (GCC 8+, Clang 10+, MSVC 2019+)
- CMake 3.15+
- Threads library (standard)

### Compilation
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Running
```bash
./trading_bot
```

## Example Output

```
=== C++ HFT Mini Trading Bot ===
Starting trading system initialization...
[OK] Core components initialized
[OK] Execution engine started
[OK] Strategy listener registered
[OK] Market simulator started
[SIGNAL] BUY signal for AAPL at price: 100.1234
[SIGNAL] SELL signal for AAPL at price: 100.5432

=== Trading Statistics ===
Total Orders Placed: 5
Total Trades Executed: 5
Average Execution Latency: 45.32 μs

=== Portfolio Snapshot ===
Total Balance: $99,850.25
Total PnL: -$149.75
```

## Project Structure

```
cpp-hft-trading-bot/
├── include/
│   ├── data_structures.h      # Core data types
│   ├── market_data.h           # Market data handler
│   ├── order_manager.h         # Order management
│   ├── execution_engine.h      # Fast order execution
│   ├── trading_strategy.h      # Strategy base & implementations
│   └── risk_manager.h          # Risk & position tracking
├── src/
│   ├── main.cpp                # Main entry point
│   ├── market_data.cpp         # Implementation
│   ├── order_manager.cpp       # Implementation
│   ├── execution_engine.cpp    # Implementation
│   ├── trading_strategy.cpp    # Implementation
│   └── risk_manager.cpp        # Implementation
├── CMakeLists.txt              # Build configuration
└── README.md                   # This file
```

## Key Performance Features

### Low Latency Design
- **Nanosecond Timestamps**: High-resolution timing for trade analysis
- **Lock-Free Queues**: Priority-based execution with minimal contention
- **Inline Optimizations**: Fast path calculations for order execution
- **Memory Efficiency**: Pre-allocated data structures minimize allocations

### Trading System Patterns
- **Observer Pattern**: Decoupled market data listeners
- **Strategy Pattern**: Pluggable trading algorithms
- **Factory Pattern**: Extensible order and trade creation
- **Singleton Pattern**: Single instance of core managers

## Extension Points

### Adding Custom Strategies
```cpp
class MyStrategy : public TradingStrategy {
    void initialize() override { /* init logic */ }
    Signal generateSignal(const MarketTick& tick) override { 
        /* signal generation */
    }
    void onTick(const MarketTick& tick) override { /* handle tick */ }
    void onOrderBook(const OrderBook& book) override { /* handle book */ }
    std::string getName() const override { return "MyStrategy"; }
};
```

### Connecting to Live Markets
Replace the `MarketSimulator` with actual data from:
- WebSocket feeds (Binance, Coinbase, etc.)
- REST APIs
- Binary data streams
- Hardware data feeds

## Performance Metrics

This bot tracks several key metrics:
- **Execution Latency**: Time from signal generation to order submission
- **Win Rate**: Percentage of profitable trades
- **Sharpe Ratio**: Risk-adjusted returns
- **Maximum Drawdown**: Peak-to-trough decline
- **Profit Factor**: Gross profit / Gross loss ratio

## Risk Management

Built-in safeguards:
- Maximum position size limits
- Per-trade loss limits
- Daily loss circuit breakers
- Leverage constraints
- Exposure caps

## Future Enhancements

- [ ] WebSocket integration for live market data
- [ ] Machine learning strategy optimization
- [ ] Backtesting framework with historical data
- [ ] Advanced order types (iceberg, TWAP, VWAP)
- [ ] Multi-asset portfolio optimization
- [ ] Real-time visualization dashboard
- [ ] Distributed order execution across multiple venues
- [ ] Options pricing and Greek calculations

## Performance Benchmarks

On typical hardware:
- Market tick processing: < 10 μs
- Order submission: < 50 μs
- Strategy signal generation: < 100 μs
- Portfolio calculation: < 200 μs

## Legal Disclaimer

This is an **educational project** for learning trading system architecture. 
- Do not use for live trading without proper testing and regulation compliance
- Paper trading only for real market connections
- Ensure compliance with securities regulations in your jurisdiction
- Use risk management features as critical safety mechanisms

## Contributing

Contributions welcome! Areas for improvement:
- Performance optimizations
- Additional trading strategies
- Better visualization
- Comprehensive unit tests
- Documentation improvements

## License

MIT License - See LICENSE file for details

## References

- "Algorithmic Trading" by Ernest P. Chan
- "High-Frequency Trading" by Irene Aldridge
- "Design Patterns" by Gang of Four
- C++ ISO/IEC 14882:2017 Standard

---

**Author**: ms-tech-tape  
**Created**: 2026-03-29  
**Language**: C++17  
**Status**: Educational/Development