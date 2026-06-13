#include <atomic>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <vector>

#ifdef __cpp_lib_hardware_interface_size
constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interface_size;
#else
constexpr size_t CACHE_LINE_SIZE = 64;
#endif

// TODO: use mmap for multi-process instead of multi-thread
template <typename T, size_t Size>
struct SharedMemoryQueue {
  alignas(64) std::atomic<size_t> write_index{0};
  alignas(64) std::atomic<size_t> read_index{0};

  T buffer[Size];
};

template <typename T, size_t Size>
class SPSCRingBuffer {
  static_assert((Size != 0) && ((Size & (Size - 1)) == 0),
                "Size must be a power of 2 for fast modulo arithmetic.");

 private:
  T buffer_[Size];

  // force read and write to be in a separate cache line to prevent
  // a cache miss when any of them use their action
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> write_index_{0};
  alignas(CACHE_LINE_SIZE) std::atomic<size_t> read_index_{0};

 public:
  SPSCRingBuffer() = default;

  // push is only called by the producer
  bool push(const T& item) {
    // only the producer modified write_index, so we can use memory order
    // relaxed
    const size_t current_write = write_index_.load(std::memory_order_relaxed);

    // acquire the read index to see where consumer is
    const size_t current_read = read_index_.load(std::memory_order_acquire);

    const size_t next_write = (current_write + 1) & (Size - 1);

    // if the queue is full
    if (next_write == current_read) [[unlikely]] {
      return false;
    }

    buffer_[current_write] = item;

    write_index_.store(next_write, std::memory_order_release);
    return true;
  }

  // only called by consumer
  bool pop(T& item_out) {
    const size_t current_read = read_index_.load(std::memory_order_relaxed);
    const size_t current_write = write_index_.load(std::memory_order_acquire);

    if (current_read == current_write) [[unlikely]] {
      return false;
    }

    item_out = buffer_[current_read];

    const size_t next_read = (current_read + 1) & (Size - 1);
    read_index_.store(next_read, std::memory_order_release);
    return true;
  }
};