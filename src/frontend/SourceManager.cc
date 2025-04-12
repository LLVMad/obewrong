#include "frontend/SourceManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

SourceManager::SourceManager() {}

SourceBuffer SourceManager::readSource(const std::filesystem::path &fullPath) {
  auto source_file = std::ifstream(fullPath);
  if (!source_file) {
    throw std::runtime_error("Could not open file " + fullPath.string());
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

void SourceManager::addFile(std::filesystem::path relPath) {
  auto source_file = std::ifstream(relPath);
  if (!source_file) {
    throw std::runtime_error("Could not open file " + relPath.string());
  }

  std::stringstream source_stream;
  source_stream << source_file.rdbuf();
  std::string contents = source_stream.str();
  std::string fileName = relPath.filename().string();

  FileData fd(relPath.parent_path(), fileName, contents, relPath);
  // @TODO check if exists already
  files.push_back(fd);
}

bool SourceManager::isImportProvided(const std::string &importName) {
  return std::ranges::any_of(files, [&](const FileData &fd) {
    return fd.name == (importName + ".obw");
  });
}

std::string SourceManager::resolveImport(std::string importName,
                                         const std::string &fromName) {
  // import {dir.}moduleName
  std::replace(importName.begin(), importName.end(), '.', '/');
  auto importPath = std::filesystem::path(importName + ".obw");
  if (importPath.empty())
    throw std::runtime_error("Could not resolve import: " + importName);
  // readSource(importPath);

  auto source_file = std::ifstream(importPath);
  if (!source_file) {
    throw std::runtime_error("Could not open file " + importPath.string());
  }

  std::stringstream source_stream;
  source_stream << source_file.rdbuf();
  std::string contents = source_stream.str();
  std::string fileName = importPath.filename().string();

  FileData fd(importPath.parent_path(), fileName, contents, importPath);
  // @TODO check if exists already
  // files.push_back(fd);

  for (auto &file : files) {
    if (file.name == fromName) {
      file.includedFiles.push_back(&fd);
      fd.includedFrom = &file;
      break;
    }
  }

  files.push_back(fd);

  return contents;
}
std::string SourceManager::getLastFileName() const {
  return this->files.back().name;
}