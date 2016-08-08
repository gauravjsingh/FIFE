#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "circuit/circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"
#include "libgarble/circuits.h"

void mux(garble_circuit *circuit, garble_context *context,
             int *in1, int *in2, int deciderWire, int *out, int len) {
  int internalWire1, internalWire2;

  for (int i = 0; i < len; i++) {
    internalWire1 = builder_next_wire(context);
    internalWire2 = builder_next_wire(context);
    out[i] = builder_next_wire(context);

    gate_XOR(circuit, context, in1[i], in2[i], internalWire1);
    gate_AND(circuit, context, internalWire1, deciderWire, internalWire2);
    gate_XOR(circuit, context, internalWire2, in2[i], out[i]);
  }
}

void neq(garble_circuit *circuit, garble_context *context,
         int* in1, int* in2, int* out, int len) {
  int tmpWire1, tmpWire2;

  out[0] = builder_next_wire(context);
  gate_XOR(circuit, context, in1[0], in2[0], out[0]);

  for (int i = 1; i < len; i++) {
    tmpWire1 = builder_next_wire(context);
    tmpWire2 = builder_next_wire(context);
    gate_XOR(circuit, context, in1[i], in2[i], tmpWire1);
    gate_OR(circuit, context, out[0], tmpWire1, tmpWire2);
    out[0] = tmpWire2;
  }
}

void gteq(garble_circuit *circuit, garble_context *context,
           int *in1, int *in2, int *out, int len) {
  int carryWire = wire_one(circuit);
  int internalWire1, internalWire2, preCarryWire;

  for (int i = 0; i < len; i++) {
    internalWire1 = builder_next_wire(context);
    internalWire2 = builder_next_wire(context);
    preCarryWire = builder_next_wire(context);

    gate_XOR(circuit, context, in1[i], carryWire, internalWire1);
    gate_XOR(circuit, context, in2[i], carryWire, internalWire2);
    gate_AND(circuit, context, internalWire1, internalWire2, preCarryWire);

    carryWire = builder_next_wire(context);
    gate_XOR(circuit, context, in1[i], preCarryWire, carryWire);
  }

  out[0] = carryWire;
}


void min(garble_circuit *circuit, garble_context *context,
         int *in1, int *in2, int *out, int *minimal, int len) {
  gteq(circuit, context, in1, in2, minimal, len);
  mux(circuit, context, in2, in1, minimal[0], out, len);
}

void add(garble_circuit *circuit, garble_context *context,
           int* in1, int* in2, int* out, int len) {
  int inputs[3], outputs[2];

  inputs[0] = in1[0];
  inputs[1] = in2[0];

  circuit_add22(circuit, context, inputs, outputs);
  out[0] = outputs[0];
  inputs[2] = outputs[1];

  for (int i = 1; i < len; i++) {
    inputs[0] = in1[i];
    inputs[1] = in2[i];
    circuit_add32(circuit, context, inputs, outputs);
    out[i] = outputs[0];
    inputs[2] = outputs[1];
  }

  out[len] = outputs[1];
}

void subtract(garble_circuit *circuit, garble_context *context,
           int* in1, int* in2, int* out, int *sign, int len) {
  int carryWire = wire_one(circuit);
  int internalWire1, internalWire2, preCarryWire, preOutWire;

  for (int i = 0; i < len; i++) {
    internalWire1 = builder_next_wire(context);
    internalWire2 = builder_next_wire(context);
    preCarryWire = builder_next_wire(context);
    preOutWire = builder_next_wire(context);
    out[i] = builder_next_wire(context);

    gate_XOR(circuit, context, in1[i], carryWire, internalWire1);
    gate_XOR(circuit, context, in2[i], carryWire, internalWire2);
    gate_XOR(circuit, context, in1[i], internalWire2, preOutWire);
    gate_NOT(circuit, context, preOutWire, out[i]);
    gate_AND(circuit, context, internalWire1, internalWire2, preCarryWire);

    carryWire = builder_next_wire(context);
    gate_XOR(circuit, context, in1[i], preCarryWire, carryWire);
  }

  sign[0] = carryWire;
}

void reduceModP(garble_circuit *circuit, garble_context *context,
                  int *in, int *out, int len, int *p) {
  int zeroWire = wire_zero(circuit);
  int oneWire = wire_one(circuit);

  int pWires[len+1];
  for (int i = 0; i<len; i++) {
    if (p[i] == 1) {
      pWires[i] = oneWire;
    } else {
      pWires[i] = zeroWire;
    }
  }
  pWires[len] = zeroWire;

  int subtracted[len + 1], sign;
  subtract(circuit, context, in, pWires, subtracted, &sign, len + 1);

  mux(circuit, context, subtracted, in, sign, out, len);
}

void addModP(garble_circuit *circuit, garble_context *context,
               int *in1, int *in2, int *out, int len, int *p) {
  int sum[len+1];
  add(circuit, context, in1, in2, sum, len);
  reduceModP(circuit, context, sum, out, len, p);
}

void multiplyBy2ModP(garble_circuit *circuit, garble_context *context,
                       int *in, int *out, int len, int *p) {
  int internalWires[len + 1];
  
  internalWires[0] = wire_zero(circuit);
  std::memcpy(&(internalWires[1]), in, len * sizeof(int));
  
  reduceModP(circuit, context, internalWires, out, len, p);
}

void multiplyModP(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int len, int* p) {
  int internalWires1[len], internalWires2[len];
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < len; i++) {
    out[i] = zeroWire;
  }

  for (int i = len - 1; i >= 0; i--) {
    multiplyBy2ModP(circuit, context, out, internalWires1, len, p);
    addModP(circuit, context, internalWires1, in2, internalWires2, len, p);
    mux(circuit, context, internalWires2, internalWires1, in1[i], out, len);
  }
}

void addGF2N(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int n) {
  for (int i = 0; i < n; i++) {
    out[i] = builder_next_wire(context);
    gate_XOR(circuit, context, in1[i], in2[i], out[i]);
  }
}

void reduceGF2NByIrredPoly(garble_circuit *circuit, garble_context *context,
               int* in, int *out, int n, int *irredPoly, int highCoeff) {
  for (int i = 0; i < n; i ++) {
    if (irredPoly[i] == 1) {
      out[i] = builder_next_wire(context);
      gate_XOR(circuit, context, in[i], highCoeff, out[i]);
    } else if (irredPoly[i] == 0) {
      out[i] = in[i];
    } else {
      throw std::runtime_error("Invalid irreducible polynomial coefficient for GF(2^n).");
    }
  }
}

void multiplyGF2N(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int n, int *irredPoly) {
  int internalWires1[n], internalWires2[n], highCoeff;
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < n; i++) {
    out[i] = zeroWire;
  }

  for (int i = n - 1; i >= 0; i--) {
    highCoeff = out[n - 1];
    memmove(&out[1], out, (n - 1) * sizeof(int));
    out[0] = zeroWire;
    reduceGF2NByIrredPoly(circuit, context, out, internalWires1, n, irredPoly, highCoeff);
    addGF2N(circuit, context, internalWires1, in2, internalWires2, n);
    mux(circuit, context, internalWires2, internalWires1, in1[i], out, n);
  }
}

void add32(garble_circuit *circuit, garble_context *context,
           int* in1, int* in2, int* out) {
  int tmpOut[33];
  
  add(circuit, context, in1, in2, tmpOut, 32);

  memcpy(out, tmpOut, 32 * sizeof(int));
}

void multiply32(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out) {
  int len = 32;
  int internalWires1[len], internalWires2[len];
  int zeroWire = wire_zero(circuit);

  for (int i = 0; i < len; i++) {
    out[i] = zeroWire;
  }

  for (int i = len - 1; i >= 0; i--) {
    internalWires1[0] = zeroWire;
    std::memcpy(&(internalWires1[1]), out, (len - 1) * sizeof(int));

    add32(circuit, context, internalWires1, in2, internalWires2);
    mux(circuit, context, internalWires2, internalWires1, in1[i], out, len);
  }
}

void hamming(garble_circuit *circuit, garble_context *context,
               int* in1, int* in2, int *out, int len) {
  std::vector<int> oldDiffs(len + 1), newDiffs(len + 1);
  int width = 1;
  int sums = len;

  for (int i = 0; i < len; i++) {
    oldDiffs[i] = builder_next_wire(context);
    gate_XOR(circuit, context, in1[i], in2[i], oldDiffs[i]);
  }

  while (sums > 1) {
    for (int i = 0; i < sums/2; i ++) {
      add(circuit, context, &oldDiffs[2 * i * width], &oldDiffs[(2 * i + 1) * width], &newDiffs[i * (width + 1)], width);
    }

    if (sums % 2 == 1) {
      std::memcpy(&newDiffs[(sums / 2) * (width + 1)], &oldDiffs[(sums-1) * width], width * sizeof(int));
      newDiffs[(sums / 2 + 1) * (width + 1) - 1] = wire_zero(circuit);
    }


    std::memcpy(oldDiffs.data(), newDiffs.data(), (len + 1) * sizeof(int));

    sums = (sums + 1)/2;
    width += 1;
  }

  std::memcpy(out, oldDiffs.data(), width * sizeof(int));
}

void levenshteinCore(garble_circuit *circuit, garble_context *context,
                     std::vector<int> &xCand, std::vector<int> &yCand, std::vector<int> &diagCand, 
                     int *in1, int *in2, std::vector<int> &out, int alphabetBits) {
  int neqWire, isDiag, oneWire = wire_one(circuit);
  std::vector<int> min1Wires(out.size()), min2Wires(out.size()), increment(out.size(), wire_zero(circuit));

  min(circuit, context, xCand.data(), yCand.data(), min1Wires.data(), &isDiag, out.size());
  min(circuit, context, min1Wires.data(), diagCand.data(), min2Wires.data(), &isDiag, out.size());
  neq(circuit, context, in1, in2, &neqWire, alphabetBits);
  mux(circuit, context, &neqWire, &oneWire, isDiag, increment.data(), 1);

  out.resize(out.size() + 1);

  add(circuit, context, min2Wires.data(), increment.data(), out.data(), out.size() - 1);

  out.resize(out.size() - 1);
}
