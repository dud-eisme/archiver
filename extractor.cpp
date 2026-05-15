#include "headerfiles.h"

void extractor(std::string archive_name) {
  if (std::filesystem::exists(archive_name)) {
    std::ifstream archive(archive_name);
    std::string line;
    int i = 1;
    while (line.find("[file]") == 0 || std::getline(archive, line)) {
      if (line.find("[file]") == 0) {
        std::ofstream file(line.substr(7, line.length() - 7 + 1));
        while (std::getline(archive, line) && line.find("[file]") != 0) file << line << std::endl;
      }
    }
  }
}
