#ifndef OBW_SOURCEMANAGER_H
#define OBW_SOURCEMANAGER_H

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "SourceLocation.h"

#include <llvm/IR/Module.h>

struct FileData {
  const std::filesystem::path directory;
  const std::string name;
  std::string content;
  FileData *includedFrom;
  std::vector<FileData*> includedFiles;
  mutable std::unique_ptr<llvm::Module> module;
  const std::filesystem::path fullPath;

  FileData(std::filesystem::path directory, std::string name, std::string data,
           std::filesystem::path fullPath)
      : directory(std::move(directory)), name(std::move(name)),
        content(std::move(data)), includedFrom(nullptr),
        includedFiles(), module(nullptr),
        fullPath(std::move(fullPath)) {}
};

// struct FileInfo {
//   FileData *data;
//   SourceLocation includedLocation;
// };

class SourceManager {
public:
  SourceManager();

  // void addSystemDirectorys();
  //
  // void addUserDirectorys();
  //
  // size_t getLineNumber();
  //
  // size_t getColumnNumber();

  SourceBuffer readSource(const std::filesystem::path &fullPath);

  // static

  std::string getSourceText(const BufferID &buffer);
  void addFile(std::filesystem::path relPath);
  bool isImportProvided(const std::string &importName);

  void addIncludedModule(const SourceBuffer &buffto,
                         const std::string &moduleName);

  bool linkWithIncludedModules(const SourceBuffer &buffto);
  std::vector<FileData*> getIncluded(const SourceBuffer &buff);

  void addCompiledModule(const SourceBuffer &buffto, std::unique_ptr<llvm::Module> module);

  std::string resolveImport(std::string importName,
                            const std::string &fromName);

  std::string getLastFileName() const;

  std::string getLastFilePath() const { return files.back()->fullPath; }

private:
  // include dirs
  std::vector<std::filesystem::path> systemDirectories;
  std::vector<std::filesystem::path> userDirectories;

  std::vector<FileData*> files;
};

#endif
