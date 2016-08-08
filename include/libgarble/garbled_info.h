#ifndef GARBLED_INFO
#define GARBLED_INFO

#include <vector>
#include <msgpack.hpp>

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

// Gives the number of non-free (non-XOR) gates in a circuit.
inline uint64_t numNonXOR(garble_circuit *gc) {
  int n = 0;
  for (size_t i = 0; i < gc->q; i++) {
    if (gc->gates[i].type != GARBLE_GATE_XOR) {
      n++;
    }
  }
  return n;
}

// A Struct to store the cryptographic information for a garbled circuit, in compact form.
struct GarbledInfo {
  std::vector<bool> output_perms;
  std::vector<block> table;
  block fixed_label;
  block global_key;

  template <typename Packer> inline void msgpack_pack(Packer& pk) const;

  inline void msgpack_unpack(msgpack::object const& o);

  // This removes table entries for free (XOR) gates.
  void packTable(garble_circuit *gc) {
    table.resize(numNonXOR(gc) * garble_table_size(gc) / sizeof(block));

    char *p = (char *) table.data();

    for (size_t i = 0; i < gc->q; i++) {
      if (gc->gates[i].type != GARBLE_GATE_XOR) {
        std::memcpy(p, ((const char *) gc->table) + i * garble_table_size(gc), garble_table_size(gc));
        p += garble_table_size(gc);
      }
    }
  };

  // This fills in a table with the garbled values for non-free (non-XOR) gates.
  void unpackTable(garble_circuit *gc) {
    gc->table = (block *) calloc(gc->q, garble_table_size(gc));

    const char *p = (const char *) table.data();

    for (size_t i = 0; i < gc->q; i++) {
      if (gc->gates[i].type != GARBLE_GATE_XOR) {
        std::memcpy(((char *) gc->table) + i * garble_table_size(gc), p, garble_table_size(gc));
        p += garble_table_size(gc);
      }
    }
  };
};

// Packs the GarbledInfo data structure.
template <typename Packer>
void GarbledInfo::msgpack_pack(Packer& pk) const {
  pk.pack_array(2);

  pk.pack_array(output_perms.size());
  for (size_t i = 0; i < output_perms.size(); i++) {
    if (output_perms[i]) {
      pk.pack_true();
    } else {
      pk.pack_false();
    }
  }

  pk.pack_bin((table.size() + 2) * sizeof(block));
  pk.pack_bin_body((const char *) table.data(), table.size() * sizeof(block));
  pk.pack_bin_body((const char *) &fixed_label, sizeof(block));
  pk.pack_bin_body((const char *) &global_key, sizeof(block));
};

// Unpacks the GarbledInfo data structure.
void GarbledInfo::msgpack_unpack(msgpack::object const& o) {
  if (o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
  assert(o.via.array.size == 2);

  output_perms.resize(o.via.array.ptr[0].via.array.size);
  for (size_t i = 0; i < output_perms.size(); i++) {
    if (o.via.array.ptr[0].via.array.ptr[i].via.boolean) {
      output_perms[i] = true;
    } else {
      output_perms[i] = false;
    }
  }

  table.resize(o.via.array.ptr[1].via.bin.size / sizeof(block) - 2);
  memcpy(table.data(), o.via.array.ptr[1].via.bin.ptr, table.size() * sizeof(block));
  memcpy(&fixed_label, o.via.array.ptr[1].via.bin.ptr + table.size() * sizeof(block), sizeof(block));
  memcpy(&global_key, o.via.array.ptr[1].via.bin.ptr + (table.size() + 1) * sizeof(block), sizeof(block));
};

#endif
