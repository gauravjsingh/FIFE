#ifndef ADD_DELTA
#define ADD_DELTA

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

// Given a circuit to compute a function, adds adding Delta to the circuit, as used by the GVW bounded-collusion FE scheme.
void addDelta(garble_circuit *circuit, garble_context *garblingContext, 
              int S, int* existing, int* zetas, int* delta, int* outputs,
              int len, int p);

#endif
