#ifndef CIRCUIT_UTILS_H
#define CIRCUIT_UTILS_H

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

/* These methods add specific functions to a circuit.
 */

// if decider wire has a 1, first input, else second input, for length len inputs
void mux(garble_circuit *circuit, garble_context *context,
           int *in1, int *in2, int deciderWire, int *out, int len);

// outputs if in1 != in2
void neq(garble_circuit *circuit, garble_context *context,
         int *in1, int *in2, int *out, int len);

// outputs if in1>=in2
void gteq(garble_circuit *circuit, garble_context *context,
              int* in1, int* in2, int* out, int len);

// output the minimum of two numbers, and sets minimal[0] = 0 if the first was smaller
void min(garble_circuit *circuit, garble_context *context,
         int *in1, int *in2, int *out, int* minimal, int len);

// Out (length len + 1) is set to sum of length len numbers on in1 and in2
void add(garble_circuit *circuit, garble_context *context,
         int* in1, int* in2, int* out, int len);

// Puts the value in in1 - the value in in2 in out (length len), and sign = 1 iff in1-in2>=0
void subtract(garble_circuit *circuit, garble_context *context,
              int* in1, int* in2, int* out, int *sign, int len);

// Takes a number 0<=n<2p, and gives a number 0<=out<p
void reduceModP(garble_circuit *circuit, garble_context *context,
                int* in, int* out, int len, int *p);

// Adds two inputs modulo p, and assumes inputs are already reduced mod p.
void addModP(garble_circuit *circuit, garble_context *context,
               int *in1, int *in2, int *out, int len, int *p);

// Multiplies a number by 2 mod p.
void multiplyBy2ModP(garble_circuit *circuit, garble_context *context,
           int* in, int* out, int len, int *p);

// Multiplies numbers modulo p, assumes inputs are reduced mod p.
void multiplyModP(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int len, int* p);

// Add/subtract in GF(2^n).
void addGF2N(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int n);

// reduces elements in GF2N, given the irreducible polynomial for a representation
void reduceGF2NByIrredPoly(garble_circuit *circuit, garble_context *context,
               int* in, int *out, int n, int *irredPoly, int highCoeff);

// Multiply in GF(2^n), given the irreducible polynomial for the representation.
void multiplyGF2N(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int n, int *irredPoly);

// Add mod 2^32
void add32(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out);

// Multiply mod 2^32
void multiply32(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out);

// Outputs hamming distance between input1 and input2, of length len
void hamming(garble_circuit *circuit, garble_context *context,
             int *in1, int *in2, int *out, int len);

// As defined by Huang et al. in "Faster Secure Two-Party Computation Using Garbled Circuits", to help compute Levenshtein distance.
void levenshteinCore(garble_circuit *circuit, garble_context *context,
                     std::vector<int> &xCand, std::vector<int> &yCand, std::vector<int> &diagCand,
                     int *in1, int *in2, std::vector<int> &out, int alphabetBits);

#endif
