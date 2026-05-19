#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <functional>

#pragma once
#pragma pack(push, 1)
struct Header {
  struct DataBlock {
    uint8_t file_name_len;
    uint64_t file_size;
    uint64_t data_offset;
  } block;
  std::string file_name;
};
#pragma pack(pop)
