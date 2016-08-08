#include <vector>

#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/parity_circuit.h"
#include "circuit/inner_product_circuit.h"
#include "circuit/hamming_circuit.h"
#include "circuit/levenshtein_circuit.h"
#include "circuit/add_delta.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* Create a universal circuit for each type of circuit.
 */

void ParityCircuitDescription::fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) {
  parityCircuit(circuit, context, input_size, inputs.data(), outputs.data());
}

void InnerProductModPCircuitDescription::fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) {
  innerProductCircuit(circuit, context, input_size, input_size, inputs.data(), outputs.data(), modBits, mod);
}

void InnerProductModPDeltaCircuitDescription::fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) {
  std::vector<int> tempOuts(output_size);

  int inner_prod_size = input_size - delta_pool_size * modBits;

  innerProductCircuit(circuit, context, inner_prod_size, input_size, inputs.data(), tempOuts.data(), modBits, mod);
  addDelta(circuit, context, delta_pool_size, tempOuts.data(), inputs.data() + inner_prod_size, inputs.data() + input_size + inner_prod_size, outputs.data(), modBits, mod);
}

void HammingCircuitDescription::fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) {
  hammingCircuit(circuit, context, input_size, inputs.data(), outputs.data());
}

void LevenshteinCircuitDescription::fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) {
  levenshteinCircuit(circuit, context, inputs.data(), outputs.data(), inputLen, circuitLen, alphabetBits);
}
