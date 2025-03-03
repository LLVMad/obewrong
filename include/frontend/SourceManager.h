#ifndef OBW_SOURCEMANAGER_H
#define OBW_SOURCEMANAGER_H

#include <string>
#include <filesystem>
#include <utility>
#include <vector>

#include "SourceLocation.h"

struct FileData {
  const std::filesystem::path directory;
  const std::string name;
  std::string content;
  const std::filesystem::path fullPath;

  FileData(std::filesystem::path directory, std::string name, std::string data, std::filesystem::path fullPath) :
    directory(std::move(directory)),
    name(std::move(name)),
    content(std::move(data)),
    fullPath(std::move(fullPath)) {}
};

struct FileInfo {
  FileData* data;
  SourceLocation includedLocation;
};

class SourceManager {
public:
  SourceManager();

  void addSystemDirectorys();

  void addUserDirectorys();

  size_t getLineNumber();

  size_t getColumnNumber();

  SourceBuffer readSource(const std::filesystem::path& fullPath);

  std::string getSourceText(const BufferID& buffer);
private:
  // include dirs
  std::vector<std::filesystem::path> systemDirectories;
  std::vector<std::filesystem::path> userDirectories;

  std::vector<FileData> files;
};

#endif
