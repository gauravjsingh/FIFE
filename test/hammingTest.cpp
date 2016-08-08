#include <vector>
#include <iostream>
#include <cstdlib>

#include "circuit/hamming_circuit.h"
#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

#include "gtest/gtest.h"

class CircuitTest : public testing::Test {
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

TEST_F(CircuitTest, hamming1) {
  int n = 8, m = 3;

  start(n, m);
  hammingCircuit(gc, ctxt, n/2, inp, outputs);
  
  finishGarbleAndEval(n, m, {1,1,0,1, 0,1,1,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(2, z);
}

TEST_F(CircuitTest, hamming2) {
  int n = 12, m = 3;

  start(n, m);
  hammingCircuit(gc, ctxt, n/2, inp, outputs);

  std::vector<int> input(n);
  int expected = 0;
  for (int i = 0; i < n/2; i++) {
    input[i] = rand() % 2;
    input[i + n/2] = rand() % 2;
    expected += (input[i] != input[i + n/2]);
  }

  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(expected, z);
}

TEST_F(CircuitTest, hamming3) {
  int m = 9, n = (1 << m);

  start(n, m);
  hammingCircuit(gc, ctxt, n/2, inp, outputs);

  std::vector<int> input(n);
  int expected = 0;
  for (int i = 0; i < n/2; i++) {
    input[i] = 0;
    input[i + n/2] = 1;
    expected += (input[i] != input[i + n/2]);
  }

  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(expected, z);
}

TEST_F(CircuitTest, hamming4) {
  int m = 16, n = (1 << m);

  start(n, m);
  hammingCircuit(gc, ctxt, n/2, inp, outputs);

  std::vector<int> input(n);
  int expected = 0;
  for (int i = 0; i < n/2; i++) {
    input[i] = rand() % 2;
    input[i + n/2] = rand() % 2;
    expected += (input[i] != input[i + n/2]);
  }

  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(expected, z);
}
