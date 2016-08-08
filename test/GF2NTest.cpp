#include <vector>

#include "circuit/circuit_utils.h"
#include "circuit/inner_product_circuit.h"
#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

#include "gtest/gtest.h"

class GF2NTest : public testing::Test {
 protected:
  void SetUp() {
    gc = new garble_circuit;
    ctxt = new garble_context;
    irredPoly = new int[4];
    irredPoly[0] = 1;
    irredPoly[1] = 1;
    irredPoly[2] = 0;
    irredPoly[3] = 1;
  }

  void TearDown() {
    delete gc;
    delete ctxt;
    delete inp;
    delete outputs;
    delete vals;
    delete irredPoly;
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
  int *irredPoly;
};

TEST_F(GF2NTest, AddGF2N) {
  int n = 8, m = 4;

  start(n, m);
  addGF2N(gc, ctxt, inp, inp + 4, outputs, m);
  finishGarbleAndEval(n, m, {1,1,0,0,1,1,1,0});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
  EXPECT_EQ(0, (int) vals[3]);
}

TEST_F(GF2NTest, ReduceGF2NByIrredPolyFirst) {
  int n = 4, m = 3;

  start(n, m);
  reduceGF2NByIrredPoly(gc, ctxt, inp, outputs, m, irredPoly, 3);
  finishGarbleAndEval(n, m, {1,0,1,1});

  EXPECT_EQ(0, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
}

TEST_F(GF2NTest, ReduceGF2NByIrredPolySecond) {
  int n = 4, m = 3;

  start(n, m);
  reduceGF2NByIrredPoly(gc, ctxt, inp, outputs, m, irredPoly, 3);
  finishGarbleAndEval(n, m, {1,0,1,0});

  EXPECT_EQ(1, (int) vals[0]);
  EXPECT_EQ(0, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
}

TEST_F(GF2NTest, innerProductGF2NCircuit) {
  int n = 6, m = 3;

  start(2 * n, m);
  innerProductGF2NCircuit(gc, ctxt, n, inp, outputs, m, irredPoly);
  finishGarbleAndEval(2 * n, m, {1,1,0,0,1,0, 0,1,0,1,0,1});

  EXPECT_EQ(1, (int) vals[0]);
  EXPECT_EQ(1, (int) vals[1]);
  EXPECT_EQ(1, (int) vals[2]);
}
