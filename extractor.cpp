#include "headerfiles.h"

std::vector<struct Header> read_header(std::string archive_path) {
  std::ifstream archive(archive_path, std::ios::binary);

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
  if (!archive.is_open() || !file.is_open())
    return;

  std::vector<char> buffer(BUFFER_SIZE);
  archive.seekg(header.block.data_offset, std::ios::beg);
  uint64_t bytes_remaining = header.block.file_size;
  while (bytes_remaining > 0) {
    uint64_t to_read = std::min((uint64_t)buffer.size(), bytes_remaining);

    archive.read(buffer.data(), to_read);
    size_t bytes_read = archive.gcount();

    if (bytes_read == 0)
      break;

    file.write(buffer.data(), bytes_read);
    bytes_remaining -= bytes_read;
  }

  archive.close();
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
