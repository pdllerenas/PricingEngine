#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "../src/common/types.hpp"

int main() {
  std::cout << "[Mock Exchange] Booting UDP feed...\n";

  // Create a raw UDP socket for sending
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    std::cerr << "[Mock Exchange] Failed to create socket.\n";
    return 1;
  }

  // Configure the destination
  sockaddr_in dest_addr{};
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(50000);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

  // Setup a realistic price generator (Random Walk)
  std::mt19937 rng(std::random_device{}());
  std::normal_distribution<double> price_drift(
      0.0, 0.05);  // Mean 0, 5-cent variance

  double current_price = 150.00;

  std::cout << "[Mock Exchange] Blasting ticks to 127.0.0.1:50000...\n";

  while (true) {
    // Walk the price
    current_price += price_drift(rng);

    OrderUpdate tick;
    tick.ticker_id = 1;
    tick.price =
        static_cast<uint32_t>(current_price * 10000);  // Convert to fixed-point
    tick.quantity = 100;
    tick.side = 'B';

    // Blast it over the socket
    sendto(sock, &tick, sizeof(OrderUpdate), 0, (struct sockaddr*)&dest_addr,
           sizeof(dest_addr));

    // Throttle to mimic a realistic ~1,000 ticks/sec feed
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }

  close(sock);
  return 0;
}