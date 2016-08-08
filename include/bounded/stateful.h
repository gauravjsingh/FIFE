#ifndef STATEFUL
#define STATEFUL

#include <vector>
#include <string>
#include <iostream>
#include <msgpack.hpp>

#include "circuit/circuit.h"
#include "oneqfe/ss.h"

/* A template for creating a bounded-collusion functional encryption scheme
 * allowing for keeping state, based on a one-query functional encryption
 * scheme.
 */

template<class OneQS>
class StatefulFE {
private:
  int key_limit;
  OneQS *oneqfe;
  int state;

public:
  struct MasterSecretKey {
    std::vector<typename OneQS::MasterSecretKey> sks;

    MasterSecretKey() {};
    MasterSecretKey(int size): sks(size) {};

    MSGPACK_DEFINE(sks);
  };

  struct MasterPublicKey {
    std::vector<typename OneQS::MasterPublicKey> pks;

    MasterPublicKey() {};
    MasterPublicKey(int size): pks(size) {};

    MSGPACK_DEFINE(pks);
  };

  struct KeyPair {
    MasterSecretKey sk;
    MasterPublicKey pk;
  };

  struct SecretKey {
    int index;
    typename OneQS::SecretKey sk;

    MSGPACK_DEFINE(index, sk);
  };

  struct CipherText {
    std::vector<typename OneQS::CipherText> cts; // size N

    CipherText() {};
    CipherText(int size): cts(size) {};

    MSGPACK_DEFINE(cts);
  };

  StatefulFE(int keys, CircuitDescription *description);

  KeyPair Setup(int length);

  SecretKey KeyGen(MasterSecretKey msk, Circuit *circuit);

  CipherText Encrypt(MasterPublicKey mpk, std::vector<int> msg);

  std::vector<int> Decrypt(SecretKey sk, CipherText ct);
};

typedef StatefulFE<SS_AES> StatefulFE_AES;
typedef StatefulFE<SS_RSA> StatefulFE_RSA;
typedef StatefulFE<SS_SingletonAES> StatefulFE_SingletonAES;
typedef StatefulFE<SS_SingletonRSA> StatefulFE_SingletonRSA;

#endif
