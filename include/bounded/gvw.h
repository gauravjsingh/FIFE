#ifndef GVW_H
#define GVW_H

#include <vector>
#include <string>
#include <iostream>
#include <msgpack.hpp>

#include "circuit/circuit.h"
#include "oneqfe/ss.h"

template<class OneQS>
class GVW {
private:
  int key_limit, depth, secret_shares, total_shares, delta_size, delta_pool_size;
  OneQS *oneqfe;
  bool useDelta;

public:
  struct MasterSecretKey {
    std::vector<typename OneQS::MasterSecretKey> sks; //size total_shares

    MasterSecretKey() {};
    MasterSecretKey(int size): sks(size) {};

    MSGPACK_DEFINE(sks);
  };

  struct MasterPublicKey {
    std::vector<typename OneQS::MasterPublicKey> pks; //size total_shares

    MasterPublicKey() {};
    MasterPublicKey(int size): pks(size) {};

    MSGPACK_DEFINE(pks);
  };

  struct KeyPair {
    MasterSecretKey sk;
    MasterPublicKey pk;
  };

  struct SecretKey {
    std::vector<long> Gamma; // size secret_shares * depth + 1
    std::vector<int> Delta; // size v
    std::vector<typename OneQS::SecretKey> sks; // size secret_shares * depth+ 1

    MSGPACK_DEFINE(Gamma, Delta, sks);
  };

  struct CipherText {
    std::vector<typename OneQS::CipherText> cts; // size total_shares

    CipherText() {};
    CipherText(int size): cts(size) {};

    MSGPACK_DEFINE(cts);
  };

  // Initialize parameters based as suggested by GVW.
  GVW(int keys, int depth, int kappa, int modulus, bool useDelta, CircuitDescription *description);

  // Initialize concrete values of secret_shares, total_shares, delta_size,
  // delta_pool_size
  GVW(int keys, int depth, int secret_shares, int total_shares, int delta_size, int delta_pool_size, int modulus, bool useDelta, CircuitDescription *description);

  KeyPair Setup(int length);

  SecretKey KeyGen(MasterSecretKey msk, Circuit *circuit);

  CipherText Encrypt(MasterPublicKey mpk, std::vector<int> msg);

  std::vector<int> Decrypt(SecretKey sk, CipherText ct);
};

typedef GVW<SS_AES> GVW_SS_AES;
typedef GVW<SS_RSA> GVW_SS_RSA;
typedef GVW<SS_SingletonAES> GVW_SS_SingletonAES;
typedef GVW<SS_SingletonRSA> GVW_SS_SingletonRSA;

#endif
