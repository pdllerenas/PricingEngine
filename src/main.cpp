#include <xmmintrin.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "common/spsc_queue.hpp"
#include "common/types.hpp"

using MarketDataQueue = SPSCRingBuffer<OrderUpdate, 1024>;

void feed_handler_loop(std::stop_token stoken, MarketDataQueue& queue) {
  std::cout << "[Producer] Feed Handler thread started.\n";

  uint32_t simulated_price = 1500000;  // $150.00
  while (!stoken.stop_requested()) {
    OrderUpdate tick{.timestamp = 1680000000000,
                     .ticker_id = 1,
                     .price = simulated_price++,
                     .quantity = 100,
                     .side = 'B'};
    while (!queue.push(tick) && !stoken.stop_requested()) {
      _mm_pause();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "[Producer] Shutting down.\n";
}

void pricing_engine_loop(std::stop_token stoken, MarketDataQueue& queue) {
  std::cout << "[Consumer] Pricing Engine thread started.\n";

  OrderUpdate current_tick{};
  while (!stoken.stop_requested()) {
    if (queue.pop(current_tick)) {
      std::cout << "[Consumer] Processed Ticker: " << current_tick.ticker_id
                << " | Price: " << (current_tick.price / 10000.0) << '\n';
    } else {
      _mm_pause();
    }
  }
  std::cout << "[Consumer] Shutting down.\n";
}

int main() {
  std::cout << "Booting Quant Engine...\n";
  auto queue = std::make_unique<MarketDataQueue>();

  std::jthread producer_thread(feed_handler_loop, std::ref(*queue));
  std::jthread consumer_thread(pricing_engine_loop, std::ref(*queue));

  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::cout << "Initializing Engine Shutdown...\n";

  producer_thread.request_stop();
  consumer_thread.request_stop();

  return 0;
}
