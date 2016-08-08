#include <vector>

#include "circuit/parity_circuit.h"
#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

#include "gtest/gtest.h"

class ParityCircuitTest : public testing::Test {
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

TEST_F(ParityCircuitTest, Parity) {
  int n = 4, m = 1;

  start(2 * n, m);
  parityCircuit(gc, ctxt, n, inp, outputs);
  finishGarbleAndEval(2 * n, m, {1,1,0,1, 0,1,0,0});

  EXPECT_EQ(1, (int) vals[0]);
}
