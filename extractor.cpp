#include "headerfiles.h"

std::vector<struct Header> read_header(std::string archive_path) {
  std::ifstream archive(archive_path, std::ios::binary);
  if (!archive.is_open()) {
    std::cerr << "Error: Could not open archive: " << archive_path << '\n';
    return {};
  }

  std::vector<struct Header> header_list;
  char check_sig[4];
  while (archive.read(check_sig, 4)) {
    if (check_sig[0] != 'J' || check_sig[1] != 'O' || check_sig[2] != 'E' ||
        check_sig[3] != 'Z')
      break;

    struct Header h;
    std::copy(check_sig, check_sig + 4, h.block.signature);
    archive.read(reinterpret_cast<char *>(&h.block) + 4, sizeof(h.block) - 4);
    h.file_name.resize(h.block.file_name_len);
    archive.read(&h.file_name[0], h.block.file_name_len);

    header_list.push_back(h);
  }

  archive.close();
  return header_list;
}

void extract_file(std::string archive_name, struct Header header,
                  char &is_done) {
  std::ifstream archive(archive_name, std::ios::binary);
  std::ofstream file(header.file_name, std::ios::binary);
  if (!archive.is_open() || !file.is_open()) {
    is_done = true;
    return;
  }

  std::vector<char> compressed_data(header.block.compressed_size);
  archive.seekg(header.block.data_offset, std::ios::beg);
  archive.read(compressed_data.data(), header.block.compressed_size);
  archive.close();

  std::vector<char> uncompressed_data(header.block.file_size);

  uLongf target_size = header.block.file_size;
  int result = uncompress(
      reinterpret_cast<Bytef *>(uncompressed_data.data()), &target_size,
      reinterpret_cast<const Bytef *>(compressed_data.data()),
      header.block.compressed_size);

  if (result == Z_OK) {
    file.write(uncompressed_data.data(), target_size);
  } else {
    std::cerr << "Extraction failed (decompression error) for: "
              << header.file_name << "\n";
  }

  file.close();
  is_done = true;
}

void multithreading_extractor(std::string archive_name,
                              std::vector<struct Header> header_list) {
  size_t max_threads = std::thread::hardware_concurrency();
  if (!max_threads)
    max_threads = MAX_OCCUPIED_THREADS;

  std::vector<std::thread> active_threads;
  std::vector<size_t> active_indices;

  size_t n_files = header_list.size();
  std::vector<char> finished_flags(n_files, false);

  for (size_t i = 0; i < n_files; i++) {
    while (active_threads.size() >= max_threads) {
      bool slot_freed = false;

      for (size_t j = 0; j < active_threads.size(); j++) {
        size_t original_file_index = active_indices[j];

        if (finished_flags[original_file_index]) {
          if (active_threads[j].joinable())
            active_threads[j].join();

          active_threads.erase(active_threads.begin() + j);
          active_indices.erase(active_indices.begin() + j);

          slot_freed = true;
          break;
        }
      }

      if (!slot_freed)
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    active_indices.push_back(i);
    active_threads.push_back(std::thread(extract_file, archive_name,
                                         header_list[i],
                                         std::ref(finished_flags[i])));
  }
  for (auto &t : active_threads)
    if (t.joinable())
      t.join();
}

void extractor(std::string archive_path) {
  std::vector<struct Header> header_list = read_header(archive_path);
  multithreading_extractor(archive_path, header_list);
}
