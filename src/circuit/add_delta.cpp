#include <cstring>

#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/inner_product_circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"
#include "libgarble/circuits.h"

void addDelta(garble_circuit *circuit, garble_context *context, 
              int delta_pool_size, int* existing, int* zetas, int* delta, int* outputs, int len, int p) {
  std::vector<int> sumWires(len), tempWires(len);
  std::memcpy(sumWires.data(), existing, len * sizeof(int));

  int parr[len];
  int mod = p;

  for (int i = 0; i < len; i++) {
    if (mod % 2 == 0) {
      parr[i] = 0;
    } else {
      parr[i] = 1;
    }
    mod /= 2;
  }

  for (int i = 0; i < delta_pool_size; i++) {
    // Select either the ith value of the delta pool, or 0, based
    // on whether delta[i] == 1 and add it to the output
    for (int j = 0; j < len; j++) {
      tempWires[j] = builder_next_wire(context);
      gate_AND(circuit, context, zetas[i * len + j], delta[i], tempWires[j]);
    }
    addModP(circuit, context, sumWires.data(), tempWires.data(), outputs, len, parr);

    std::memcpy(sumWires.data(), outputs, len * sizeof(int));
  }
}
