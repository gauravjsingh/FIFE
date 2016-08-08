#ifndef HAMMING_CIRCUIT_H
#define HAMMING_CIRCUIT_H

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* Creates a circuit that finds the hamming distance between the first and second half of the input wires (each of size n), and put the output on the output wires (also of size n).
 */

void hammingCircuit(garble_circuit *circuit, garble_context *garblingContext, 
                   int n, int* inputs, int* outputs);

#endif
