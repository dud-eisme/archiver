#include "headerfiles.h"

#define BUFFER_SIZE 4 * 1024 * 1024
#define MAX_OCCUPIED_THREADS 4

std::vector<struct Header> header_creation(int n_files, char *files[]) {
  std::vector<struct Header> header_list(n_files);

  uint64_t header_offset = 0;
  for (int i = 0; i < n_files; i++) {
    std::filesystem::path path = files[i + 3];
    header_list[i].file_name = path.filename();
    header_list[i].block.file_name_len = header_list[i].file_name.length();
    header_list[i].block.file_size = std::filesystem::file_size(path);
    header_offset += 17 + header_list[i].block.file_name_len;
  }

  uint64_t initial_offset = header_offset;
  for (int i = 0; i < n_files; i++) {
    header_list[i].block.data_offset = initial_offset;
    initial_offset += header_list[i].block.file_size;
  }

  return header_list;
}

void header_write(std::string archive_name, int n_files,
                  std::vector<struct Header> header_list) {
  std::ofstream archive(archive_name, std::ios::binary);

  for (int i = 0; i < n_files; i++) {
    archive.write(reinterpret_cast<const char *>(&header_list[i].block),
                  sizeof(header_list[i].block));
    archive.write(header_list[i].file_name.data(),
                  header_list[i].block.file_name_len);
  }

  archive.close();
}

void zip_files(std::string archive_name, std::string file_name,
               struct Header header, char &is_done) {
  std::fstream archive(archive_name,
                       std::ios::binary | std::ios::in | std::ios::out);
  std::ifstream file(file_name, std::ios::binary);
  if (!archive.is_open() || !file.is_open())
    return;

  std::vector<char> buffer(BUFFER_SIZE);
  archive.seekp(header.block.data_offset, std::ios::beg);
  while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
    archive.write(buffer.data(), file.gcount());
  }

  archive.close();
  file.close();

  is_done = true;
}

void multithreading_zipping(int n_files, char *files[],
                            std::string archive_name,
                            std::vector<struct Header> header_list) {
  size_t max_threads = std::thread::hardware_concurrency();
  if (!max_threads)
    max_threads = MAX_OCCUPIED_THREADS;

  std::vector<std::thread> active_threads;
  std::vector<size_t> active_indices;
  std::vector<char> finished_flags(n_files, false);

  for (int i = 0; i < n_files; i++) {
    std::filesystem::path path = files[i + 3];

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
    active_threads.push_back(std::thread(zip_files, archive_name, path.string(),
                                         header_list[i],
                                         std::ref(finished_flags[i])));
  }
  for (auto &t : active_threads)
    if (t.joinable())
      t.join();
}

void zipper(int n_files, char *files[], std::string archive_name) {
  std::vector<struct Header> header_list = header_creation(n_files, files);
  header_write(archive_name, n_files, header_list);
  multithreading_zipping(n_files, files, archive_name, header_list);
}
