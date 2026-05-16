#include "headerfiles.h"

void zipper(int n_files, char *files[], std::string archive_name) {
  std::ofstream archive(archive_name);
  
  std::vector<char> buffer(1024);
  for (int i = 1; i < n_files; i++) {
    std::ifstream file(files[i + 2], std::ios::binary);
    while (!file.eof()) {
      file.read(buffer, 1024);
      archive.write(buffer, 1024);
    }
    file.close();
  }
}
