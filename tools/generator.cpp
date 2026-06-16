#include <array>
#include <cstdint>

#include "../epoll/epoll.hpp"
#include "../socket/socket.hpp"
#include "../../src/common/types.hpp"

void run_market_data_engine() {
  try {
    Socket market_data_socket(50000);

    Epoll epoll_instance;
    epoll_instance.add_socket(market_data_socket.get_fd(), EPOLLIN);

    constexpr int MAX_EVENTS = 32;
    std::array<struct epoll_event, MAX_EVENTS> events{};
    OrderUpdate current_tick{};

    std::cout << "Engine Starting. Polling for market data...\n";

    while (true) {
      int num_ready = epoll_instance.wait(events.data(), MAX_EVENTS, -1);
      if (num_ready < 0) [[unlikely]] {
        if (errno == EINTR) continue;
        break;
      }
      for (int i = 0; i < num_ready; ++i) {
        if (events[i].data.fd == market_data_socket.get_fd()) {
          ssize_t bytes_read = read(market_data_socket.get_fd(), &current_tick,
                                    sizeof(OrderUpdate));
          if (bytes_read == sizeof(OrderUpdate)) [[likely]] {
            std::cout << "Tick: " << current_tick.ticker_id << " @ "
                      << current_tick.price << "\n";
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Engine crashed: " << e.what() << "\n";
  }
}