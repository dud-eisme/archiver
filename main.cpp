#include "necessary.h"

#define MAX_ARGS 100

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
    extractor(archive_name);
  }
}
