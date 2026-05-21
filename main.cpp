#include "necessary.h"

#define MAX_ARGS 100

void zipper(int, char**, std::string);
void extractor(std::string);


int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Usage:\n";
    std::cout << "  Archive:   " << argv[0]
              << " a <archive_name> <file1> <file2> ...\n";
    std::cout << "  Extract:   " << argv[0] << " x <archive_name>\n";
    return 1;
  }

  std::string cmd = argv[1];
  std::string archive_path = argv[2];

  if (cmd == "a") {
    if (argc < 4) {
      std::cout << "Error: No target files specified for archiving.\n";
      std::cout << "Usage: " << argv[0]
                << " a <archive_name> <file1> <file2> ...\n";
      return 1;
    }
    int n_files = argc - 3;
    std::cout << "Initializing parallel compression for " << n_files
              << " files...\n";
    zipper(n_files, argv, archive_path);
    std::cout << "Archive package tracking complete.\n";
  }

  else if (cmd == "x") {
    std::cout << "Initializing parallel decompression pipeline for " << archive_path << "...\n";
    extractor(archive_path);
    std::cout << "Extraction pipeline complete.\n";
  }

  else {
    std::cout << "Error: Unknown command flag '" << cmd << "'. Use 'a' to archive or 'x' to extract.\n";
    return 1;
  }

  return 0;
}
