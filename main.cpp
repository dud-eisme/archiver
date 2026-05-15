#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <string>
#define MAX_ARGS 100

void zipper(int, char**, std::string);
void extracter(std::string);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Usage:\n";
    std::cout << "./z.ip a <archive_name>.zip <file(s)_or_folder>\n";
  }

  std::string cmd = argv[1];
  std::string archive_name = argv[2];

  if (cmd == "a") {
    zipper(argc - 2, argv, archive_name);
  }

  else if (cmd == "x") {
    extracter(archive_name);
  }
}

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

void extracter(std::string archive_name) {
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
