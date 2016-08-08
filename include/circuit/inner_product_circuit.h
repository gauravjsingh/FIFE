#ifndef INNER_PRODUCT_CIRCUIT_H
#define INNER_PRODUCT_CIRCUIT_H

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* Creates a circuit that finds the inner product modulo p between the first and second half of the input wires (each vectors of n/len numbers, with len bits each), and put the output on the output wires (also a vector of n/len len-bit numbers). Assumes that the inputs are reduced modulo p.
*/

void innerProductCircuit(garble_circuit *circuit, garble_context *garblingContext, 
                  int n, int offset, int* inputs, int* outputs, int len, int p);

/* Creates a circuit that finds the inner product over GF2N between the first and second half of the input wires (each vectors of n/len numbers, with len bits each), and put the output on the output wires (also a vector of n/len len-bit numbers). This also takes the irreducible polynomial for the representation of GF2N used.
*/

void innerProductGF2NCircuit(garble_circuit *circuit, garble_context *garblingContext, 
                  int n, int* inputs, int* outputs, int len, int *irredPoly);

/* Creates a circuit that finds the inner product modulo 2^32 (ie, for 32-bit numbers) between the first and second half of the input wires (each vectors of n/len numbers, with len bits each), and put the output on the output wires (also a vector of n/len len-bit numbers).
*/

void innerProductCircuit32(garble_circuit *circuit, garble_context *garblingContext, 
                  int n, int* inputs, int* outputs);

#endif
