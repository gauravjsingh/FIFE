#include <cstring>
#include <iostream>

#include "circuit/circuit.h"
#include "circuit/circuit_utils.h"
#include "circuit/levenshtein_circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

void levenshteinCircuit(garble_circuit *circuit, garble_context *context,
                        int *inputs, int *out, int len1, int len2, int alphabetBits) {
  // Table of partial levenshtein distances.
  std::vector<std::vector<std::vector<int> > > vals(len1 + 1);

  int *in1 = inputs;
  int *in2 = inputs + len1 * alphabetBits;

  int zeroWire = wire_zero(circuit);
  int oneWire = wire_one(circuit);

  // Initializing table entries to correct sizes and values
  for (auto &i: vals) {
    i.resize(len2 + 1);
  }

  for (int i = 0; i < len1 + 1; i++) {
    for (int j = 0; j < len2 + 1; j++) {
      int bits = (int) ceil(log2(std::max(i,j) + 1));
      vals[i][j].resize(bits);
    }
  }

  for (int i = 0; i < len1; i++) {
    for (size_t j = 0; j < vals[i][0].size(); j++) {
      if ((i >> j) % 2 == 0) {
        vals[i][0][j] = zeroWire;
      } else {
        vals[i][0][j] = oneWire;
      }
    }
  }

  for (int i = 0; i < len2; i++) {
    for (size_t j = 0; j < vals[0][i].size(); j++) {
      if ((i >> j) % 2 == 0) {
        vals[0][i][j] = zeroWire;
      } else {
        vals[0][i][j] = oneWire;
      }
    }
  }

  std::vector<int> xCand, yCand, diagCand;

  // Populating table of partial values.
  for (int i = 1; i < len1 + 1; i++) {
    for (int j = 1; j < len2 + 1; j++) {
      // Getting the three candidates for the levenshtein core.
      xCand.assign(vals[i-1][j].begin(), vals[i-1][j].end());
      if (xCand.size() < vals[i][j].size()) {
        xCand.push_back(zeroWire);
      }

      yCand.assign(vals[i][j-1].begin(), vals[i][j-1].end());
      if (yCand.size() < vals[i][j].size()) {
        yCand.push_back(zeroWire);
      }

      diagCand.assign(vals[i-1][j-1].begin(), vals[i-1][j-1].end());
      if (diagCand.size() < vals[i][j].size()) {
        diagCand.push_back(zeroWire);
      }

      // Uses levenshtein core to get the partial levenshtein value.
      levenshteinCore(circuit, context, xCand, yCand, diagCand,
                      in1 + (i - 1) * alphabetBits, in2 + (j - 1) * alphabetBits, vals[i][j], alphabetBits);
    }
  }

  // Output the final levenshtein value.
  std::memcpy(out, vals[len1][len2].data(), vals[len1][len2].size() * sizeof(int));
}
