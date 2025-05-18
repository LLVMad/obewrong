#include "frontend/SourceManager.h"

#include <algorithm>
#include <fstream>
#include <llvm/Linker/Linker.h>
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

  auto fd = new FileData(fullPath.parent_path(), fileName, contents, fullPath);
  auto contains = std::ranges::find_if(
  files,
  [&](const auto &file) { return file->name == fileName; });

  if (contains == files.end()) files.push_back(fd);

  return SourceBuffer(
      BufferID(fd->name, static_cast<uint32_t>(files.size() - 1)), contents);
}

std::string SourceManager::getSourceText(const BufferID &buffer) {
  return files[buffer.id]->content;
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

  auto fd = new FileData(relPath.parent_path(), fileName, contents, relPath);

  auto contains = std::ranges::find_if(
    files,
    [&](const auto &file) { return file->name == fileName; });

  if (contains == files.end()) files.push_back(fd);
}

bool SourceManager::isImportProvided(const std::string &importName) {
  return std::ranges::any_of(files, [&](auto fd) {
    return fd->name == (importName + ".obw");
  });
}

void
SourceManager::addIncludedModule(const SourceBuffer &buffto,
  const std::string &moduleName) {
  auto imported = std::ranges::find_if(
      files, [&](auto &fd) { return fd->name == (moduleName + ".obw"); });

  this->files[buffto.id.id - (files.size() - 1)]->includedFiles.push_back(
      *imported);
}

void SourceManager::addCompiledModule(const SourceBuffer &buffto, std::unique_ptr<llvm::Module> module) {
  llvm::outs() << "Storing module in FileData for buffer " << buffto.id.id << "\n";
  this->files[buffto.id.id]->module = std::move(module);
  llvm::outs() << "Module stored successfully\n";
}

bool SourceManager::linkWithIncludedModules(const SourceBuffer &buffto) {
  // get include modules names
  // find these in SourceManager
  // get their llvm modules
  // link

  // auto thisFile = this->files[buffto.id.id - (files.size() - 1)];
  // auto includedModules = this->files[buffto.id.id - (files.size() - 1)].includedFiles;
  //
  // std::ranges::for_each(
  //   includedModules,
  //   [&](FileData &fd) {
  //     if (llvm::Linker::linkModules(*thisFile.module,
  //     std::unique_ptr<llvm::Module>(fd.module))) return false;
  //   });

  return true;
}

std::vector<FileData*>
SourceManager::getIncluded(const SourceBuffer &buff) {
  return this->files[buff.id.id - (files.size() - 1)]->includedFiles;
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

  auto fd = new FileData(importPath.parent_path(), fileName, contents, importPath);
  // @TODO check if exists already
  files.push_back(fd);

  for (auto &file : files) {
    if (file->name == fromName) {
      file->includedFiles.push_back(fd);
      fd->includedFrom = file;
      break;
    }
  }

  files.push_back(fd);

  return contents;
}
std::string SourceManager::getLastFileName() const {
  return this->files.back()->name;
}