#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* Creates a circuit that finds the levenshtein distance between the first len1*alphabetBits wires and the last len2*alphabetBits wires of input, and puts the output on the out wires. The inputs are treated as vectors of size len1 and len2 of characters represented by alphabetBits.
 */

void levenshteinCircuit(garble_circuit *circuit, garble_context *garblingContext, 
                        int *inputs, int *out, int len1, int len2, int alphabetBits);

#endif
