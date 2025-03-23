#include "frontend/SourceManager.h"

#include <fstream>
#include <sstream>

SourceManager::SourceManager() {}

SourceBuffer SourceManager::readSource(const std::filesystem::path &fullPath) {
  auto source_file = std::ifstream(fullPath);
  if (!source_file) {
    // Error
  }

  std::stringstream source_stream;
  source_stream << source_file.rdbuf();
  std::string contents = source_stream.str();
  std::string fileName = fullPath.filename().string();

  FileData fd(fullPath.parent_path(), fileName, contents, fullPath);
  // @TODO check if exists already
  files.push_back(fd);

  return SourceBuffer(
      BufferID(fd.name, static_cast<uint32_t>(files.size() - 1)), contents);
}

std::string SourceManager::getSourceText(const BufferID &buffer) {
  return files[buffer.id].content;
}
