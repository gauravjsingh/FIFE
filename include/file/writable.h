#ifndef WRITABLE
#define WRITABLE

#include <fstream>
#include <iomanip>

#include <msgpack.hpp>

// Writes an object compatible with msgpack to file.
template <class Writable>
inline void writeToFile(Writable w, std::string fileName) {
  std::ofstream file;
  file.open(fileName);
  msgpack::pack(file, w);
  file.close();
};

// Reads an object compatible with msgpack from file.
template <class Writable>
inline void readFromFile(Writable &w, std::string fileName) {
  std::ifstream file;
  file.open(fileName, std::ios::in | std::ios::binary | std::ios::ate);

  std::streampos size = file.tellg();
  file.seekg(0, std::ios::beg);

  char *memblock = (char *) malloc(size);

  file.read(memblock, size);
   
  msgpack::object_handle oh = msgpack::unpack(memblock, size);
  msgpack::object obj = oh.get();
  file.close();
  free(memblock);

  obj.convert(w);
};

#endif
