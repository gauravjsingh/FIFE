#ifndef CIRCUITS_H
#define CIRCUITS_H

#include <vector>
#include <cmath>
#include <algorithm>

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

// Types of supported circuits.
typedef enum {
  CIRCUIT_TYPE_PARITY,
  CIRCUIT_TYPE_INNER_PRODUCT_MOD_P,
  CIRCUIT_TYPE_INNER_PRODUCT_MOD_P_DELTA,
  CIRCUIT_TYPE_HAMMING,
  CIRCUIT_TYPE_LEVENSHTEIN,
} CircuitType;

// Generic structure of a Circuit Description. This describes 
class CircuitDescription {
 public:
  CircuitType type;

  int circuit_size, input_size, output_size;

  // Returns delta_pool_size as used by the GVW bounded-collusion FE scheme.
  virtual int getPossibleDeltaSize() {
    return 0;
  };

  // Gives the modulus for the computation, if it is being performed over a finite field.
  virtual int getMod() {
    return 0;
  };

  // Gives the number of bits in the modulus, if it exists.
  virtual int getModBits() {
    return 0;
  };

  // Turns the raw output of a circuit into more meaningful values for the circuit
  virtual std::vector<int> returnVals(bool *vals) = 0;

  // Turns the message to binary to input into the circuit.
  virtual int msgBit(std::vector<int> &msg, int i) = 0;

  // Creates a universal circuit based on the circuit description.
  virtual void universalCircuit(garble_circuit *circuit) {
    garble_context context;

    int n = input_size + circuit_size;
    int m = output_size;

    // Set up labels
    std::vector<int> inp(n);
    builder_init_wires(inp.data(), n);
    std::vector<int> outputs(m);

    // create the circuit
    garble_new(circuit, n, m, GARBLE_TYPE_HALFGATES);
    builder_start_building(circuit, &context);

    // The core of the universal circuit is dependent on the type of circuit description.
    fillUniversalCircuit(circuit, &context, inp, outputs);

    builder_finish_building(circuit, &context, outputs.data());
  }

  // Circuit description type dependent universal circuit creation.
  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs) = 0;
};

// Parity circuits, ie inner product over F_2.
class ParityCircuitDescription : public CircuitDescription {
 public:
  ParityCircuitDescription(int size) {
    type = CIRCUIT_TYPE_PARITY;
    circuit_size = size;
    input_size = size;
    output_size = 1;
  };

  virtual std::vector<int> returnVals(bool *vals) {
    std::vector<int> returnedVals(output_size);
    for (int i = 0; i < output_size; i++) {
      returnedVals[i] = (int) vals[i];
    }

    return returnedVals;
  }

  virtual int msgBit(std::vector<int> &msg, int i) {
    return msg[i];
  }

  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs);
};

// Circuits for inner product over F_p, for some prime p.
class InnerProductModPCircuitDescription : public CircuitDescription {
 public:
  int mod, modBits;

  InnerProductModPCircuitDescription(int mod, int numbers): mod(mod) {
    modBits = (int) ceil(log2(mod));
    type = CIRCUIT_TYPE_INNER_PRODUCT_MOD_P;
    circuit_size = modBits * numbers;
    input_size = modBits * numbers;
    output_size = modBits;
  };

  virtual int getMod() {
    return mod;
  };

  virtual int getModBits() {
    return modBits;
  };

  virtual std::vector<int> returnVals(bool *vals) {
    std::vector<int> returnedVals(output_size/modBits);
    for (int i = 0; i < (int) returnedVals.size(); i++) {
      returnedVals[i] = 0;
      for (int j = 0; j < modBits; j++) {
        returnedVals[i] += ((int) vals[i * modBits + j]) << j;
      }
    }

    return returnedVals;
  }

  virtual int msgBit(std::vector<int> &msg, int i) {
    return (msg[i/modBits] >> (i % modBits)) % 2;
  }

  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs);
};

// Circuits for inner product over F_p for prime p, with the added Delta as used in the GVW bounded-collusion FE scheme.
class InnerProductModPDeltaCircuitDescription : public InnerProductModPCircuitDescription {
 public:
  int delta_pool_size;

  InnerProductModPDeltaCircuitDescription(int mod, int numbers, int delta_pool_size): InnerProductModPCircuitDescription(mod, numbers + delta_pool_size), delta_pool_size(delta_pool_size) {
    type = CIRCUIT_TYPE_INNER_PRODUCT_MOD_P_DELTA;
    circuit_size = modBits * numbers + delta_pool_size;
  };

  virtual int getPossibleDeltaSize() {
    return delta_pool_size;
  };

  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs);
};

// Circuits for computing hamming distance.
class HammingCircuitDescription : public CircuitDescription {
 public:
  HammingCircuitDescription(int size) {
    type = CIRCUIT_TYPE_HAMMING;
    circuit_size = size;
    input_size = size;
    output_size = (int) floor(log2(size)) + 1;
  };

  virtual std::vector<int> returnVals(bool *vals) {
    std::vector<int> returnedVals(output_size);
    for (int i = 0; i < (int) returnedVals.size(); i++) {
      returnedVals[i] = (int) vals[i];
    }

    return returnedVals;
  }

  virtual int msgBit(std::vector<int> &msg, int i) {
    return msg[i];
  }

  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs);
};

// Circuits for computing levenshtein distance.
class LevenshteinCircuitDescription : public CircuitDescription {
 public:
  int alphabetBits, inputLen, circuitLen;

  LevenshteinCircuitDescription(int inputLen, int circuitLen, int alphabetBits) {
    type = CIRCUIT_TYPE_LEVENSHTEIN;
    circuit_size = circuitLen * alphabetBits;
    input_size = inputLen * alphabetBits;
    output_size = (int) ceil(log2(std::max(inputLen, circuitLen) + 1));
    this->alphabetBits = alphabetBits;
    this->inputLen = inputLen;
    this->circuitLen = circuitLen;
  };

  virtual std::vector<int> returnVals(bool *vals) {
    std::vector<int> returnedVals(1);
    returnedVals[0] = 0;
    for (int i = 0; i < output_size; i++) {
      returnedVals[0] += ((int) vals[i]) << i;
    }

    return returnedVals;
  }

  virtual int msgBit(std::vector<int> &msg, int i) {
    return (msg[i/alphabetBits] >> (i % alphabetBits)) % 2;
  }

  virtual void fillUniversalCircuit(garble_circuit *circuit, garble_context *context, std::vector<int> inputs, std::vector<int> &outputs);
};

// Details of a specific circuit. These correspond to circuit descriptions, but can be seen as an instance of a circuit described by the circuit description.
class Circuit {
 public:
  CircuitType type;

  Circuit(CircuitType type): type(type) {};

  virtual int getBit(int i) = 0;
};

// Computes parity with the encoded message.
class ParityCircuit : public Circuit {
public:
  std::vector<int> circuit; // size is circuit_size, with 0 and 1 bits

  ParityCircuit(std::vector<int> circuit): Circuit(CIRCUIT_TYPE_PARITY), circuit(circuit) {};

  virtual int getBit(int i) {
    return circuit[i];
  }
};

// Computes inner product with the encoded message.
class InnerProductModPCircuit : public Circuit {
public:
  int mod, modBits;

  std::vector<int> circuit; // size is circuit_size, with 0 and 1 bits

  InnerProductModPCircuit(int mod, std::vector<int> circuit): Circuit(CIRCUIT_TYPE_INNER_PRODUCT_MOD_P), mod(mod), circuit(circuit) {
    modBits = (int) ceil(log2(mod));
  };
  InnerProductModPCircuit(const InnerProductModPCircuit &circ): Circuit(CIRCUIT_TYPE_INNER_PRODUCT_MOD_P) {
    mod = circ.mod;
    modBits = circ.modBits;
    circuit = circ.circuit;
  };

  virtual int getBit(int i) {
    return (circuit[i/modBits] >> (i % modBits)) % 2;
  }
};

// Computes inner prodcut with Delta of the encoded message, and with the given Delta.
class InnerProductModPDeltaCircuit : public InnerProductModPCircuit {
public:
  std::vector<int> Delta;

  InnerProductModPDeltaCircuit(InnerProductModPCircuit circ, int delta_pool_size, std::vector<int> Delta): InnerProductModPCircuit(circ) {
    type = CIRCUIT_TYPE_INNER_PRODUCT_MOD_P_DELTA;

    this->Delta.resize(delta_pool_size, 0);
    for (auto i: Delta) {
      this->Delta[i] = 1;
    }
  };

  virtual int getBit(int i) {
    if (i < (int) circuit.size() * modBits) {
      return (circuit[i/modBits] >> (i % modBits)) % 2;
    } else {
      return Delta[i-circuit.size() * modBits];
    }
  }
};

// COmputes Hamming distance with the encoded message.
class HammingCircuit : public Circuit {
public:
  std::vector<int> circuit; // size is circuit_size, with 0 and 1 bits

  HammingCircuit(std::vector<int> circuit): Circuit(CIRCUIT_TYPE_HAMMING), circuit(circuit) {};

  virtual int getBit(int i) {
    return circuit[i];
  }
};

// COmputes Levenshtein distance with the encoded message.
class LevenshteinCircuit : public Circuit {
public:
  std::vector<int> circuit;
  int inputLen, alphabetBits;

  LevenshteinCircuit(std::vector<int> circuit, int inputLen, int alphabetBits): Circuit(CIRCUIT_TYPE_LEVENSHTEIN), circuit(circuit), inputLen(inputLen), alphabetBits(alphabetBits) {};

  virtual int getBit(int i) {
    return (circuit[i/alphabetBits] >> (i % alphabetBits)) % 2;
  }
};

#endif
