#ifndef OBW_SOURCELOCATION_H
#define OBW_SOURCELOCATION_H

#include <cstdint>
#include <string>

/*
 * To distinguish files
 * by number, also to make it able to
 * track where do we include what later
 */
struct BufferID {
  std::string name;
  uint32_t id;

  BufferID(const std::string &name, uint32_t id) : name(name), id(id) {}
};

/*
 * To track current location during
 * compilation process
 *
 * @deprecated
 */
struct SourceLocation {
  // id of a buffer
  uint64_t buffer;
  // byte offset representing the location within that buffer
  uint64_t charOffset;
};

/*
 * Stores source file text
 * and a given id to it
 */
struct SourceBuffer {
  BufferID id;
  std::string data;

  SourceBuffer(BufferID id_, const std::string &data_) : id(id_), data(data_) {}
};

#endif
