#include "headerfiles.h"

#define BUFFER_SIZE 4 * 1024 * 1024

std::vector<struct Header> header_creation(int n_files, char *files[]) {
  std::vector<struct Header> header_list(n_files);
  uint64_t header_offset = 0;
  for (int i = 0; i < n_files; i++) {
    std::filesystem::path path = files[i + 2];
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

void header_write(std::string archive_name, int n_files, std::vector<struct Header> header_list) {
  std::ofstream archive(archive_name);
  for (int i = 0; i < n_files; i++) {
    archive.write(reinterpret_cast<const char*>(&header_list[i].block), sizeof(header_list[i].block));
    archive.write(header_list[i].file_name.data(), header_list[i].block.file_name_len);
  }
  archive.close();
}

void zip_files(std::string archive_name, std::string file_name, struct Header header) {
  std::ofstream archive(archive_name, std::ios::binary | std::ios::in | std::ios::out);
  std::ifstream file(file_name, std::ios::binary);
  std::vector<char> buffer(BUFFER_SIZE);
  archive.seekp(header.block.data_offset, std::ios::beg);
  while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
    archive.write(buffer.data(), file.gcount());
  }
  archive.close();
  file.close();
}

void zipper(int n_files, char *files[], std::string archive_name) {
  std::vector<struct Header> header_list = header_creation(n_files, files);
  header_write(archive_name, n_files, header_list);
}
