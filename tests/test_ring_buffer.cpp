#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "common/spsc_queue.hpp"

TEST(SPSCRingBufferTest, ConcurrentPushPop) {
  SPSCRingBuffer<size_t, 1024> queue;
  const size_t target_count = 1'000'000;

  std::vector<size_t> consumed_data;
  consumed_data.reserve(target_count);

  std::thread producer([&]() {
    for (size_t i = 0; i < target_count; i++) {
      while (!queue.push(i)) {
      }
    }
  });

  std::thread consumer([&]() {
    size_t item{0};
    for (size_t i = 0; i < target_count; ++i) {
      while (!queue.pop(item)) { }
      consumed_data.push_back(item);
    }
  });

  producer.join();
  consumer.join();

  ASSERT_EQ(consumed_data.size(), target_count);

  for (size_t i = 0; i < target_count; ++i) {
    ASSERT_EQ(consumed_data[i], i);
  }
}