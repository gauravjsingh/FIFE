#ifndef ES_WRAPPER
#define ES_WRAPPER

#include <vector>
#include <iostream>

#include "circuit/circuit.h"

// THis creates a simple (and insecure) FE scheme, where the encrypted message is just encrypted with a PKE scheme.
template <class ES>
class ESWrapper {
public:
  typedef typename ES::SecretKey MasterSecretKey;

  typedef typename ES::PublicKey MasterPublicKey;

  struct KeyPair {
    MasterSecretKey sk;
    MasterPublicKey pk;

    KeyPair() {};
    KeyPair(typename ES::KeyPair p): sk(p.sk), pk(p.pk) {};
  };

  typedef typename ES::SecretKey SecretKey;

  typedef typename ES::CipherText CipherText;

  typedef typename ES::PlainText PlainText;

  static KeyPair Setup(int length) {
    return KeyPair(ES::Setup(length));
  };

  static SecretKey KeyGen(MasterSecretKey msk) {
    return msk;
  };

  static CipherText Encrypt(MasterPublicKey mpk, PlainText msg) {
    return ES::Encrypt(mpk, msg);
  };

  static PlainText Decrypt(SecretKey sk, CipherText ct) {
    return ES::Decrypt(sk, ct);
  };
};

typedef ESWrapper<AES> AESWrapper;
typedef ESWrapper<RSA> RSAWrapper;

#endif
