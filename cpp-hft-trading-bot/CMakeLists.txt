cmake_minimum_required(VERSION 3.15)
project(CPPHFTTradingBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/market_data.cpp
    src/order_manager.cpp
    src/execution_engine.cpp
    src/trading_strategy.cpp
    src/risk_manager.cpp
)

add_executable(trading_bot ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(trading_bot PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(trading_bot PRIVATE /O2 /arch:AVX2)
else()
    target_compile_options(trading_bot PRIVATE -O3 -march=native)
endif()