#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/parity_circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"
#include "libgarble/circuits.h"

void parityCircuit(garble_circuit *circuit, garble_context *context, 
                  int n, int* inputs, int* outputs) {
  int oldInternalWire = wire_zero(circuit);
  int newInternalWire, productWire;

  for (int i = 0; i < n; i++) {
    productWire = builder_next_wire(context);
    newInternalWire = builder_next_wire(context);
    gate_AND(circuit, context, inputs[i], inputs[i + n], productWire);
    gate_XOR(circuit, context, oldInternalWire, productWire, newInternalWire);
    oldInternalWire=newInternalWire;
  }

  outputs[0] = oldInternalWire;
}
