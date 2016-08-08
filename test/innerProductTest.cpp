#include <vector>
#include <iostream>
#include <cstdlib>

#include "circuit/inner_product_circuit.h"
#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

#include "gtest/gtest.h"

class InnerProductCircuitTest : public testing::Test {
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

    block extractedLabels[n];
    for (int i = 0; i < n; i++) {
      extractedLabels[i] = gc->wires[2*i+labels[i]];
    }
    garble_eval(gc, extractedLabels, NULL, vals);
  }

  garble_circuit *gc;
  garble_context *ctxt;
  int *inp;
  int *outputs;
  bool *vals;
};

TEST_F(InnerProductCircuitTest, innerProductCircuit) {
  int n = 8, m = 4;

  start(2 * n, m);
  innerProductCircuit(gc, ctxt, n, n, inp, outputs, m, 11);
  finishGarbleAndEval(2 * n, m, {1,1,1,0,0,0,1,0, 0,1,0,1,1,0,1,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(InnerProductCircuitTest, innerProductCircuit32) {
  int n = 64, m = 32;

  start(n, m);
  innerProductCircuit32(gc, ctxt, n/2, inp, outputs);
  std::vector<int> input(64);
  int x = 1282048, y = 974027482;
  int expected = x * y;
  for (int i = 0; i < 32; i++) {
    input[i] = x % 2;
    input[i + 32] = y % 2;
    x /= 2;
    y /= 2;
  }
  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = 31; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(expected, z);
}

TEST_F(InnerProductCircuitTest, innerProductCircuit32_2) {
  int n = 64, m = 32;

  start(n, m);
  innerProductCircuit32(gc, ctxt, n/2, inp, outputs);
  std::vector<int> input(64);
  int x = 1, y = 1;
  int expected = x * y;
  for (int i = 0; i < 32; i++) {
    input[i] = x % 2;
    input[i + 32] = y % 2;
    x /= 2;
    y /= 2;
  }
  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = 31; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(z, expected);
}

TEST_F(InnerProductCircuitTest, innerProductCircuit32_3) {
  int n = 64*100, m = 32;

  start(n, m);
  innerProductCircuit32(gc, ctxt, n/2, inp, outputs);
  std::vector<int> x(n/64);
  std::vector<int> y(n/64);
  std::vector<int> input(n);

  for (size_t i = 0; i < x.size(); i++) {
    x[i] = std::rand();
    y[i] = std::rand();
  }

  int expected = 0;
  for (size_t i = 0; i < x.size(); i++) {
    expected += x[i] * y[i];
  }

  for (size_t i = 0; i < x.size(); i++) {
    for (int j = 0; j < 32; j++) {
      input[i*32 + j] = x[i] % 2;
      input[i*32 + j + 32*x.size()] = y[i] % 2;
      x[i] /= 2;
      y[i] /= 2;
    }
  }
  finishGarbleAndEval(n, m, input);

  int z = 0;

  for (int i = 31; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(expected, z);
}
