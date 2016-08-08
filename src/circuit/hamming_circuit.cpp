#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/hamming_circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

void hammingCircuit(garble_circuit *circuit, garble_context *context, 
                  int n, int* inputs, int* outputs) {
  hamming(circuit, context, inputs, inputs + n, outputs, n);
}
