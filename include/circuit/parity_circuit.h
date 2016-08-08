#ifndef PARITY_CIRCUIT_H
#define PARITY_CIRCUIT_H

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* Creates a circuit that finds the parity between the first half and second half of input (each of size n), and puts the output on the outputs wire.
 */

void parityCircuit(garble_circuit *circuit, garble_context *garblingContext, 
                   int n, int* inputs, int* outputs);

#endif
