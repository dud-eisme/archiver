#include "headerfiles.h"


void zip_file(std::string file_name, struct Header &header,
              std::vector<char> &output_buffer, char &is_done) {
  std::ifstream file(file_name, std::ios::binary);
  if (!file.is_open()) {
    is_done = true;
    return;
  }

  uint64_t raw_size = std::filesystem::file_size(file_name);
  std::vector<char> raw_data(raw_size);
  file.read(raw_data.data(), raw_size);
  file.close();

  uLongf max_comp_size = compressBound(raw_size);
  output_buffer.resize(max_comp_size);

  uLongf actual_comp_size = max_comp_size;
  int result = compress(
      reinterpret_cast<Bytef *>(output_buffer.data()), &actual_comp_size,
      reinterpret_cast<const Bytef *>(raw_data.data()), raw_size);

  if (result == Z_OK) {
    output_buffer.resize(actual_comp_size);
    header.block.file_size = raw_size;
    header.block.compressed_size = actual_comp_size;
  } else
    std::cerr << "Compression failed for: " << file_name << '\n';

  is_done = true;
}

void multithreading_zipping(int n_files, char *files[],
                            std::vector<struct Header> &header_list, std::vector<std::vector<char>> &compressed_buffers) {
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
    active_threads.push_back(std::thread(
        zip_file, path.string(), std::ref(header_list[i]),
        std::ref(compressed_buffers[i]), std::ref(finished_flags[i])));
  }
  for (auto &t : active_threads)
    if (t.joinable())
      t.join();
}

void zipper(int n_files, char *files[], std::string archive_name) {
  if (archive_name.find(".eisme") == std::string::npos)
    archive_name += ".eisme";
  std::vector<struct Header> header_list(n_files);
  std::vector<std::vector<char>> compressed_buffers(n_files);

  multithreading_zipping(n_files, files, header_list, compressed_buffers);

  uint64_t current_offset = 0;
  for (int i = 0; i < n_files; i++) {
    std::filesystem::path path = files[i + 3];
    header_list[i].file_name = path.filename();
    header_list[i].block.file_name_len = header_list[i].file_name.length();

    header_list[i].block.signature[0] = 'J';
    header_list[i].block.signature[1] = 'O';
    header_list[i].block.signature[2] = 'E';
    header_list[i].block.signature[3] = 'Z';

    current_offset += sizeof(struct DataBlock) + header_list[i].block.file_name_len;
  }

  for (int i = 0; i < n_files; i++) {
    header_list[i].block.data_offset = current_offset;
    current_offset += header_list[i].block.compressed_size;
  }

  std::ofstream archive(archive_name, std::ios::binary);

  for (int i = 0; i < n_files; i++) {
    archive.write(reinterpret_cast<const char *>(&header_list[i].block), sizeof(header_list[i].block));
    archive.write(header_list[i].file_name.data(), header_list[i].block.file_name_len);
  }

  for (int i = 0; i < n_files; i++) {
    archive.write(compressed_buffers[i].data(), header_list[i].block.compressed_size);
  }

  archive.close();
}
