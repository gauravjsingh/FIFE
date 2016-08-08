#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>

#include <emmintrin.h>

#include "oneqfe/ss.h"
#include "oneqfe/singleton.h"
#include "oneqfe/esWrapper.h"
#include "circuit/circuit.h"

#include "libgarble/garble.h"
#include "libgarble/circuit_builder.h"

template<class ES>
SS<ES>::SS(CircuitDescription *description) {
  circuitDescription = description;
}

template<class ES>
typename SS<ES>::KeyPair SS<ES>::Setup(int length) {
  typename SS<ES>::KeyPair p;
  p.sk = typename SS<ES>::MasterSecretKey(circuitDescription->circuit_size);
  p.pk = typename SS<ES>::MasterPublicKey(circuitDescription->circuit_size);

  for (int i=0; i<circuitDescription->circuit_size; i++) {
    typename ES::KeyPair p1 = ES::Setup(length);
    typename ES::KeyPair p2 = ES::Setup(length);
    p.sk.sks[i].first = p1.sk;
    p.sk.sks[i].second = p2.sk;
    p.pk.pks[i].first = p1.pk;
    p.pk.pks[i].second = p2.pk;
  }

  return p;
}

template<class ES>
typename SS<ES>::SecretKey SS<ES>::KeyGen(typename SS<ES>::MasterSecretKey msk, Circuit *circuit) {
  assert(circuit->type == circuitDescription->type);

  typename SS<ES>::SecretKey sk;
  sk.bits = std::vector<int>(circuitDescription->circuit_size);
  sk.sks = std::vector<typename ES::SecretKey>(circuitDescription->circuit_size);

  for (int i=0; i<circuitDescription->circuit_size; i++) {
    sk.bits[i] = circuit->getBit(i);

    if (sk.bits[i] == 0) {
      sk.sks[i] = ES::KeyGen(msk.sks[i].first);
    } else {
      sk.sks[i] = ES::KeyGen(msk.sks[i].second);
    }
  }
  return sk;
}

template<class ES>
typename SS<ES>::CipherText SS<ES>::Encrypt(typename SS<ES>::MasterPublicKey mpk, std::vector<int> msg) {

  garble_circuit circuit;

  // get the universal circuit, and garble it
  circuitDescription->universalCircuit(&circuit);
  garble_garble(&circuit, NULL, NULL);

  typename SS<ES>::CipherText ct;

  ct.garbled_info.output_perms.resize(circuit.m);
  
  for (size_t i = 0; i < circuit.m; i++) {
    ct.garbled_info.output_perms[i] = circuit.output_perms[i];
  }

  ct.garbled_info.table.resize(circuit.q);
  ct.garbled_info.packTable(&circuit);
  ct.garbled_info.fixed_label = circuit.fixed_label;
  ct.garbled_info.global_key = circuit.global_key;

  ct.labels.resize(circuitDescription->input_size);
  ct.inputs.resize(circuitDescription->circuit_size);

  //use only the encoded labels for the message
  for (int i = 0; i < circuitDescription->input_size; i++) {
    ct.labels[i] = circuit.wires[2 * i + circuitDescription->msgBit(msg, i)];
  }

  //if input bit i is b, encrypt it with msk[i][b]
  for (int i = 0; i < circuitDescription->circuit_size; i++) {
    const unsigned char *bytes1 = (const unsigned char*) &circuit.wires[2 * i + 2 * circuitDescription->input_size];
    typename ES::PlainText pt1(bytes1, bytes1 + 16);
    const unsigned char *bytes2 = (const unsigned char*) &circuit.wires[2 * i + 1 + 2 * circuitDescription->input_size];
    typename ES::PlainText pt2(bytes2, bytes2 + 16);
    ct.inputs[i].first = ES::Encrypt(mpk.pks[i].first, pt1);
    ct.inputs[i].second = ES::Encrypt(mpk.pks[i].second, pt2);
  }

  return ct;
}

template<class ES>
std::vector<int> SS<ES>::Decrypt(typename SS<ES>::SecretKey sk, typename SS<ES>::CipherText ct) {
  garble_circuit circuit;

  // generate the universal circuit
  circuitDescription->universalCircuit(&circuit);

  std::vector<block> extractedLabels(circuit.n);

  // copy the labels for the message
  memcpy(extractedLabels.data(), ct.labels.data(), ct.labels.size() * sizeof(block));

  //decrypt the labels given by the secret key
  std::vector<unsigned char> bytes;
  for (size_t i = 0; i < ct.inputs.size(); i++) {
    if (sk.bits[i] == 0) {
      bytes = ES::Decrypt(sk.sks[i], ct.inputs[i].first);
    } else {
      bytes = ES::Decrypt(sk.sks[i], ct.inputs[i].second);
    }
    extractedLabels[i + ct.labels.size()] = _mm_loadu_si128((__m128i *) bytes.data());
  }

  // Unpack garbled_info into circuit to evaulate it
  circuit.output_perms = (bool *) calloc(circuit.m, sizeof(bool));
  
  for (size_t i = 0; i < circuit.m; i ++) {
    circuit.output_perms[i] = ct.garbled_info.output_perms[i];
  }

  ct.garbled_info.unpackTable(&circuit);

  circuit.fixed_label=ct.garbled_info.fixed_label;
  circuit.global_key=ct.garbled_info.global_key;

  // Evaluate the garbled circuit.
  bool vals[circuit.m];
  garble_eval(&circuit, extractedLabels.data(), NULL, vals);

  return circuitDescription->returnVals(vals);
}

template class SS<AESWrapper>;
template class SS<RSAWrapper>;
template class SS<SingletonAES>;
template class SS<SingletonRSA>;

