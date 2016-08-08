#include <vector>
#include <iostream>
#include <cstdlib>

#include "circuit/circuit_utils.h"
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

TEST_F(CircuitTest, ANDGate) {
  int n = 8, m = 4;

  start(n, m);
  for (int i = 0; i < 4; i ++) {
    outputs[i] = builder_next_wire(ctxt);
    gate_AND(gc, ctxt, i, i + 4, outputs[i]);
  }
  finishGarbleAndEval(n, m, {0,0,1,1,0,1,0,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(1, (int) vals[3]);
}

TEST_F(CircuitTest, XORGate) {
  int n = 8, m = 4;

  start(n, m);
  for (int i = 0; i < 4; i ++) {
    outputs[i] = builder_next_wire(ctxt);
    gate_XOR(gc, ctxt, i, i + 4, outputs[i]);
  }
  finishGarbleAndEval(n, m, {0,0,1,1,0,1,0,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(CircuitTest, ORGate) {
  int n = 8, m = 4;

  start(n, m);
  for (int i = 0; i < 4; i ++) {
    outputs[i] = builder_next_wire(ctxt);
    gate_OR(gc, ctxt, i, i + 4, outputs[i]);
  }
  finishGarbleAndEval(n, m, {0,0,1,1,0,1,0,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
  EXPECT_EQ(1, (int) vals[3]);
}

TEST_F(CircuitTest, ZeroWire) {
  int n = 8, m = 1;

  start(n, m);
  outputs[0]=wire_zero(gc);
  finishGarbleAndEval(n, m, {0,0,1,1,0,1,0,1});

  EXPECT_EQ(0, (int) vals[0]);
}

TEST_F(CircuitTest, OneWire) {
  int n = 8, m = 1;

  start(n, m);
  outputs[0]=wire_one(gc);
  finishGarbleAndEval(n, m, {0,0,1,1,0,1,0,1});

  EXPECT_EQ(1, (int) vals[0]);
}

TEST_F(CircuitTest, MuxFirst) {
  int n = 9, m = 4;

  start(n, m);
  mux(gc, ctxt, inp, inp + 4, inp[8], outputs, m);
  finishGarbleAndEval(n, m, {0,1,0,0,1,0,0,1,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(CircuitTest, MuxSecond) {
  int n = 9, m = 4;

  start(n, m);
  mux(gc, ctxt, inp, inp + 4, inp[8], outputs, m);
  finishGarbleAndEval(n, m, {0,1,0,0,1,0,0,1,0});

  EXPECT_EQ(1, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(1, (int) vals[3]);
}

TEST_F(CircuitTest, Add) {
  int n = 8, m = 5;

  start(n, m);
  add(gc, ctxt, inp, inp + 4, outputs, m - 1);
  finishGarbleAndEval(n, m, {1,1,0,0,1,1,1,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(1, (int) vals[3]);
  EXPECT_EQ(0, (int) vals[4]);
}

TEST_F(CircuitTest, Subtract1) {
  int n = 2, m = 2;

  start(n, m);
  subtract(gc, ctxt, inp, inp + 1, outputs, outputs + 1, m - 1);
  finishGarbleAndEval(n, m, {1,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
}

TEST_F(CircuitTest, Subtract2) {
  int n = 2, m = 2;

  start(n, m);
  subtract(gc, ctxt, inp, inp + 1, outputs, outputs + 1, m - 1);
  finishGarbleAndEval(n, m, {0,1});

  EXPECT_EQ(1, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
}

TEST_F(CircuitTest, Subtract3) {
  int n = 8, m = 5;

  start(n, m);
  subtract(gc, ctxt, inp, inp + 4, outputs, outputs + 4, m - 1);
  finishGarbleAndEval(n, m, {1,0,0,1,1,1,0,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
  EXPECT_EQ(1, (int) vals[4]);
}

TEST_F(CircuitTest, AddModP1) {
  int n = 8, m = 4;
  int p[4] = {1,1,0,1};

  start(n, m);
  addModP(gc, ctxt, inp, inp + 4, outputs, m, p);
  finishGarbleAndEval(n, m, {1,0,0,1,1,1,0,0});

  EXPECT_EQ(1, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(CircuitTest, AddModP2) {
  int n = 8, m = 4;
  int p[4] = {1,1,0,1};

  start(n, m);
  addModP(gc, ctxt, inp, inp + 4, outputs, m, p);
  finishGarbleAndEval(n, m, {1,0,1,0,1,1,0,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(0, (int) vals[2]);
  EXPECT_EQ(1, (int) vals[3]);
}

TEST_F(CircuitTest, multiplyModP) {
  int n = 8, m = 4;
  int p[4] = {1,1,0,1};

  start(n, m);
  multiplyModP(gc, ctxt, inp, inp + 4, outputs, m, p);
  finishGarbleAndEval(n, m, {0,1,0,1,1,1,1,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(CircuitTest, add32) {
  int n = 64, m = 32;

  start(n, m);
  add32(gc, ctxt, inp, inp + 32, outputs);
  std::vector<int> input(64);
  int x = 1282048, y = 974027482;
  int expected = x + y;
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

TEST_F(CircuitTest, multiply32) {
  int n = 64, m = 32;

  start(n, m);
  multiply32(gc, ctxt, inp, inp + 32, outputs);
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

TEST_F(CircuitTest, hamming) {
  int n = 8, m = 3;

  start(n, m);
  hamming(gc, ctxt, inp, inp + n/2, outputs, n/2);
  
  finishGarbleAndEval(n, m, {1,1,0,1, 0,1,1,1});

  int z = 0;

  for (int i = m - 1; i >= 0; i--) {
    z *= 2;
    z += vals[i];
  }

  EXPECT_EQ(2, z);
}

