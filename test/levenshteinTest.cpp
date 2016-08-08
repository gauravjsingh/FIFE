#include <vector>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "circuit/levenshtein_circuit.h"
#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

#include "gtest/gtest.h"

class LevenshteinTest : public testing::Test {
 protected:
  void SetUp() {
    gc = new garble_circuit;
    ctxt = new garble_context;
  }

  void TearDown() {
    delete gc;
    delete ctxt;
    delete inp;
    delete outputs;
    delete vals;
  }

  void start(int n, int m) {
    outputs = new int[m];
    vals = new bool[m];
    inp = new int[n];
    garble_new(gc, n, m, GARBLE_TYPE_HALFGATES);
    builder_init_wires(inp, n);
    builder_start_building(gc, ctxt);
  }

  void finishGarbleAndEval(int n, int m, std::vector<int> labels) {
    std::vector<block> inputLabels(2 * n);
    std::vector<block> outputMap(2 * m);

    builder_finish_building(gc, ctxt, outputs);
    garble_garble(gc, NULL, outputMap.data());

    std::vector<block> extractedLabels(n);
    for (int i = 0; i < n; i++) {
      extractedLabels[i] = gc->wires[2*i+labels[i]];
    }

    garble_eval(gc, extractedLabels.data(), NULL, vals);

  }

  garble_circuit *gc;
  garble_context *ctxt;
  int *inp;
  int *outputs;
  bool *vals;
};

TEST_F(LevenshteinTest, 1bit_equal) {
  int n = 8, m = 3;
  int len1 = 4, len2 = 4;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, 1);
  
  finishGarbleAndEval(n, m, {1,0,1,1, 1,0,1,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(0, z);
}

TEST_F(LevenshteinTest, 1bit_unequal) {
  int n = 8, m = 3;
  int len1 = 4, len2 = 4;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, 1);
  
  finishGarbleAndEval(n, m, {1,1,1,1, 0,0,1,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(2, z);
}

TEST_F(LevenshteinTest, 1bit_different_lengths) {
  int n = 6, m = 3;
  int len1 = 4, len2 = 2;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, 1);
  
  finishGarbleAndEval(n, m, {1,0,1,1, 0,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(2, z);
}

TEST_F(LevenshteinTest, 2bitsSimple1) {
  int n = 4, m = 1;
  int len1 = 1, len2 = 1;
  int alphabetBits = 2;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, alphabetBits);
  
  finishGarbleAndEval(n, m, {1,0, 0,0});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(1, z);
}


TEST_F(LevenshteinTest, 2bitsSimple2) {
  int n = 8, m = 2;
  int len1 = 2, len2 = 2;
  int alphabetBits = 2;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, alphabetBits);
  
  finishGarbleAndEval(n, m, {1,0,0,0, 0,0,0,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(2, z);
}

TEST_F(LevenshteinTest, 2bits) {
  int n = 12, m = 3;
  int len1 = 4, len2 = 2;
  int alphabetBits = 2;

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, alphabetBits);
  
  finishGarbleAndEval(n, m, {1,0,0,0,1,1,1,0, 0,0,0,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(3, z);
}

TEST_F(LevenshteinTest, 2bitsLarge) {
  int len1 = 100, len2 = 100;
  int alphabetBits = 2;
  int n = (len1 + len2) * alphabetBits, m = (int) ceil(log2(std::max(len1, len2) + 1));

  start(n, m);
  levenshteinCircuit(gc, ctxt, inp, outputs, len1, len2, alphabetBits);

  std::vector<int> inputs(n);

  for (size_t i = 0; i < inputs.size(); i++) {
    inputs[i] = rand() % 2;
  }

  finishGarbleAndEval(n, m, inputs);

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }
}
