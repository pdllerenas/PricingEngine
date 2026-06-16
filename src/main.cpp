#include <xmmintrin.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "common/spsc_queue.hpp"
#include "common/types.hpp"
#include "pricing_engine/ms_lbfgs.hpp"

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

  const size_t N = 3;
  const size_t M = 5;
  MsLbfgsSolver solver(N, M);

  std::vector<double> current_params = {1.0, 1.0, 1.0};
  std::vector<double> current_gradient(N, 0.0);
  std::vector<double> next_gradient(N, 0.0);
  std::vector<double> search_direction(N, 0.0);

  std::vector<double> step_s(N, 0.0);
  std::vector<double> grad_diff_y(N, 0.0);

  const double learning_rate = 0.01;

  OrderUpdate current_tick{};
  while (!stoken.stop_requested()) {
    if (queue.pop(current_tick)) {
      double market_price = current_tick.price / 10000.0;

      for (size_t i = 0; i < N; ++i) {
        current_gradient[i] = current_params[i] - (market_price / N);
      }

      solver.compute_search_direction(current_gradient, search_direction);

      for (size_t i = 0; i < N; ++i) {
        step_s[i] = learning_rate * search_direction[i];
        current_params[i] += step_s[i];
      }

      for (size_t i = 0; i < N; ++i) {
        next_gradient[i] = current_params[i] - (market_price / N);
        grad_diff_y[i] = next_gradient[i] - current_gradient[i];
      }

      solver.update_history(step_s, grad_diff_y);

      std::cout << "[Consumer] Tick " << current_tick.ticker_id << " @ $"
                << market_price << " | Params: [" << current_params[0] << ", "
                << current_params[1] << ", " << current_params[2] << "]\n";
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
