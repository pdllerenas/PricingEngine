#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct OrderUpdate {
  uint64_t timestamp;
  uint32_t ticker_id;
  uint32_t price;
  uint32_t quantity;
  uint8_t side;
};
#pragma pack(pop)