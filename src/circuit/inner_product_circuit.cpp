#include <cstring>

#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/inner_product_circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"
#include "libgarble/circuits.h"

void innerProductCircuit(garble_circuit *circuit, garble_context *context, 
                  int n, int offset, int* inputs, int* outputs, int len, int p) {
  int oldSumWires[len], newSumWires[len], productWires[len];
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < len; i++) {
    oldSumWires[i] = zeroWire;
  }

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

  for (int i = 0; i < n/len; i++) {
    multiplyModP(circuit, context, &inputs[i * len], &inputs[i * len + offset], productWires, len, parr);
    addModP(circuit, context, oldSumWires, productWires, newSumWires, len, parr);

    std::memcpy(oldSumWires, newSumWires, len * sizeof(int));
  }

  std::memcpy(outputs, oldSumWires, len * sizeof(int));
}

void innerProductGF2NCircuit(garble_circuit *circuit, garble_context *context, 
                  int n, int* inputs, int* outputs, int len, int *irredPoly) {
  int oldSumWires[len], newSumWires[len], productWires[len];
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < len; i++) {
    oldSumWires[i] = zeroWire;
  }

  for (int i = 0; i < n/len; i++) {
    multiplyGF2N(circuit, context, &inputs[i * len], &inputs[i * len + n], 
                 productWires, len, irredPoly);
    addGF2N(circuit, context, oldSumWires, productWires, newSumWires, len);

    std::memcpy(oldSumWires, newSumWires, len * sizeof(int));
  }

  std::memcpy(outputs, oldSumWires, len * sizeof(int));
}

void innerProductCircuit32(garble_circuit *circuit, garble_context *context, 
                  int n, int* inputs, int* outputs) {
  int len = 32;
  int oldSumWires[len], newSumWires[len], productWires[len];
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < len; i++) {
    oldSumWires[i] = zeroWire;
  }

  for (int i = 0; i < n/len; i++) {
    multiply32(circuit, context, &inputs[i * len], &inputs[i * len + n], productWires);
    add32(circuit, context, oldSumWires, productWires, newSumWires);

    std::memcpy(oldSumWires, newSumWires, len * sizeof(int));
  }

  std::memcpy(outputs, oldSumWires, len * sizeof(int));
}
