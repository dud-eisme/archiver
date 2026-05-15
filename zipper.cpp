#include "headerfiles.h"

void zipper(int n_files, char *files[], std::string archive_name) {
  std::ofstream archive(archive_name);
  
  for (int i = 1; i < n_files; i++) {
    std::ifstream file(files[i + 2]);
    std::string line;
    archive << "[file] " << files[i + 2] << std::endl;
    while (std::getline(file, line))
      archive << line << std::endl;
    file.close();
  }
}
