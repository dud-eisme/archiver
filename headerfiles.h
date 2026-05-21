#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <zlib.h>
#include <cstdint>
#include <zconf.h>

#pragma once
#define BUFFER_SIZE 4 * 1024 * 1024
#define MAX_OCCUPIED_THREADS 4

#pragma pack(push, 1)
struct DataBlock {
  char signature[4];
  uint8_t file_name_len;
  uint64_t file_size;
  uint64_t compressed_size;
  uint64_t data_offset;
};
#pragma pack(pop)
struct Header {
  struct DataBlock block;
  std::string file_name;
};
